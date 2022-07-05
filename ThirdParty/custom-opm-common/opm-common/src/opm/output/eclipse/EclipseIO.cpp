/*
  Copyright (c) 2016 Statoil ASA
  Copyright (c) 2013-2015 Andreas Lauser
  Copyright (c) 2013 SINTEF ICT, Applied Mathematics.
  Copyright (c) 2013 Uni Research AS
  Copyright (c) 2015 IRIS AS

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "config.h"

#include <opm/output/eclipse/EclipseIO.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/RPTConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include <opm/input/eclipse/Units/Dimension.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/output/eclipse/AggregateAquiferData.hpp>
#include <opm/output/eclipse/RestartIO.hpp>
#include <opm/output/eclipse/Summary.hpp>
#include <opm/output/eclipse/WriteInit.hpp>
#include <opm/output/eclipse/WriteRFT.hpp>
#include <opm/output/eclipse/WriteRPT.hpp>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/OutputStream.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <limits>
#include <memory>     // unique_ptr
#include <optional>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <utility>    // move


namespace {

inline std::string uppercase( std::string x ) {
    std::transform( x.begin(), x.end(), x.begin(),
        []( char c ) { return std::toupper( c ); } );

    return x;
}

void ensure_directory_exists( const std::filesystem::path& odir )
{
    namespace fs = std::filesystem;

    if (fs::exists( odir ) && !fs::is_directory( odir ))
        throw std::runtime_error {
            "Filesystem element '" + odir.generic_string()
            + "' already exists but is not a directory"
        };

    std::error_code ec{};
    if (! fs::exists( odir ))
        fs::create_directories( odir, ec );

    if (ec) {
        std::ostringstream msg;

        msg << "Failed to create output directory '"
            << odir.generic_string()
            << "\nSystem reports: " << ec << '\n';

        throw std::runtime_error { msg.str() };
    }
}

}

namespace Opm {
class EclipseIO::Impl {
    public:
    Impl( const EclipseState&, EclipseGrid, const Schedule&, const SummaryConfig& , const std::string& baseName, const bool& writeEsmry);
        void writeINITFile( const data::Solution& simProps, std::map<std::string, std::vector<int> > int_data, const std::vector<NNCdata>& nnc) const;
        void writeEGRIDFile( const std::vector<NNCdata>& nnc );
        std::pair<bool, bool> wantRFTOutput( const int report_step, const bool isSubstep ) const;

        bool wantSummaryOutput(const int    report_step,
                               const bool   isSubstep,
                               const double secs_elapsed) const;

        void recordSummaryOutput(const double secs_elapsed);

        const EclipseState& es;
        EclipseGrid grid;
        const Schedule& schedule;
        std::string outputDir;
        std::string baseName;
        SummaryConfig summaryConfig;
        out::Summary summary;
        bool output_enabled;
        std::optional<RestartIO::Helpers::AggregateAquiferData> aquiferData{std::nullopt};

private:
    mutable bool sumthin_active_{false};
    mutable bool sumthin_triggered_{false};
    double last_sumthin_output_{std::numeric_limits<double>::lowest()};

    bool checkAndRecordIfSumthinTriggered(const int report_step,
                                          const double secs_elapsed) const;
    bool summaryAtRptOnly(const int report_step) const;
};

EclipseIO::Impl::Impl( const EclipseState& eclipseState,
                       EclipseGrid grid_,
                       const Schedule& schedule_,
                       const SummaryConfig& summary_config,
                       const std::string& base_name,
                       const bool& writeEsmry)
    : es( eclipseState )
    , grid( std::move( grid_ ) )
    , schedule( schedule_ )
    , outputDir( eclipseState.getIOConfig().getOutputDir() )
    , baseName( uppercase( eclipseState.getIOConfig().getBaseName() ) )
    , summaryConfig( summary_config )
    , summary( eclipseState, summaryConfig, grid , schedule, base_name, writeEsmry )
    , output_enabled( eclipseState.getIOConfig().getOutputEnabled() )
{
    const auto& aqConfig = this->es.aquifer();

    if (aqConfig.hasAnalyticalAquifer() || aqConfig.hasNumericalAquifer()) {
        this->aquiferData = RestartIO::Helpers::AggregateAquiferData {
            RestartIO::inferAquiferDimensions(this->es),
            aqConfig,
            this->grid
        };
    }
}


void EclipseIO::Impl::writeINITFile(const data::Solution&                   simProps,
                                    std::map<std::string, std::vector<int>> int_data,
                                    const std::vector<NNCdata>&             nnc) const
{
    EclIO::OutputStream::Init initFile {
        EclIO::OutputStream::ResultSet { this->outputDir, this->baseName },
        EclIO::OutputStream::Formatted { this->es.cfg().io().getFMTOUT() }
    };

    InitIO::write(this->es, this->grid, this->schedule,
                  simProps, std::move(int_data), nnc, initFile);
}


void EclipseIO::Impl::writeEGRIDFile( const std::vector<NNCdata>& nnc ) {
    const auto formatted = this->es.cfg().io().getFMTOUT();

    const auto ext = '.'
        + (formatted ? std::string{"F"} : std::string{})
        + "EGRID";

    const auto egridFile = (std::filesystem::path{ this->outputDir }
        / (this->baseName + ext)).generic_string();

    this->grid.save( egridFile, formatted, nnc, this->es.getDeckUnitSystem());
}

std::pair<bool, bool>
EclipseIO::Impl::wantRFTOutput( const int  report_step,
                                const bool isSubstep ) const
{
    if (isSubstep)
        return std::make_pair(false, false);

    const auto first_rft = this->schedule.first_RFT();
    if (!first_rft.has_value())
        return std::make_pair(false, false);

    const auto first_rft_out = first_rft.value();
    const auto step = static_cast<std::size_t>(report_step);

    return std::make_pair(step >= first_rft_out, step > first_rft_out);
}

bool EclipseIO::Impl::wantSummaryOutput(const int    report_step,
                                        const bool   isSubstep,
                                        const double secs_elapsed) const
{
    // Check this condition first because the end of a SUMTHIN interval
    // might coincide with a report step.  In that case we also need to
    // reset the interval starting point even if the primary reason for
    // generating summary output is the report step.
    this->checkAndRecordIfSumthinTriggered(report_step, secs_elapsed);

    return !isSubstep
        || (!this->summaryAtRptOnly(report_step)
            && (!this->sumthin_active_ || this->sumthin_triggered_));
}

void EclipseIO::Impl::recordSummaryOutput(const double secs_elapsed)
{
    if (this->sumthin_triggered_)
        this->last_sumthin_output_ = secs_elapsed;
}

bool EclipseIO::Impl::checkAndRecordIfSumthinTriggered(const int    report_step,
                                                       const double secs_elapsed) const
{
    const auto& sumthin = this->schedule[report_step - 1].sumthin();

    this->sumthin_active_ = sumthin.has_value(); // True if SUMTHIN value strictly positive.

    return this->sumthin_triggered_ = this->sumthin_active_
        && ! (secs_elapsed < this->last_sumthin_output_ + sumthin.value());
}

bool EclipseIO::Impl::summaryAtRptOnly(const int report_step) const
{
    return this->schedule[report_step - 1].rptonly();
}

/*
int_data: Writes key(string) and integers vector to INIT file as eclipse keywords
- Key: Max 8 chars.
- Wrong input: invalid_argument exception.
*/
void EclipseIO::writeInitial( data::Solution simProps, std::map<std::string, std::vector<int> > int_data, const std::vector<NNCdata>& nnc) {
    if( !this->impl->output_enabled )
        return;

    {
        const auto& es = this->impl->es;
        const IOConfig& ioConfig = es.cfg().io();

        simProps.convertFromSI( es.getUnits() );
        if( ioConfig.getWriteINITFile() )
            this->impl->writeINITFile( simProps , std::move(int_data), nnc );

        if( ioConfig.getWriteEGRIDFile( ) )
            this->impl->writeEGRIDFile( nnc );
    }

}

// implementation of the writeTimeStep method
void EclipseIO::writeTimeStep(const Action::State& action_state,
                              const WellTestState& wtest_state,
                              const SummaryState& st,
                              const UDQState& udq_state,
                              int report_step,
                              bool  isSubstep,
                              double secs_elapsed,
                              RestartValue value,
                              const bool write_double)
 {
    if (! this->impl->output_enabled) {
        return;
    }

    const auto& es = this->impl->es;
    const auto& grid = this->impl->grid;
    const auto& schedule = this->impl->schedule;
    const auto& ioConfig = es.cfg().io();

    const bool final_step { report_step == static_cast<int>(schedule.size()) - 1 };
    const bool is_final_summary = final_step && !isSubstep;

    if ((report_step > 0) &&
        this->impl->wantSummaryOutput(report_step, isSubstep, secs_elapsed))
    {
        this->impl->summary.add_timestep(st, report_step, isSubstep);
        this->impl->summary.write(is_final_summary);

        this->impl->recordSummaryOutput(secs_elapsed);
    }


    if (final_step && !isSubstep && this->impl->summaryConfig.createRunSummary()) {
        std::filesystem::path outputDir { this->impl->outputDir } ;
        std::filesystem::path outputFile { outputDir / this->impl->baseName } ;
        EclIO::ESmry(outputFile).write_rsm_file();
    }

    /*
      Current implementation will not write restart files for substep,
      but there is an unsupported option to the RPTSCHED keyword which
      will request restart output from every timestep.
    */
    if(!isSubstep && schedule.write_rst_file(report_step))
    {
        EclIO::OutputStream::Restart rstFile {
            EclIO::OutputStream::ResultSet { this->impl->outputDir,
                                             this->impl->baseName },
            report_step,
            EclIO::OutputStream::Formatted { ioConfig.getFMTOUT() },
            EclIO::OutputStream::Unified   { ioConfig.getUNIFOUT() }
        };

        RestartIO::save(rstFile, report_step, secs_elapsed, value,
                        es, grid, schedule, action_state, wtest_state, st,
                        udq_state, this->impl->aquiferData, write_double);
    }

    // RFT file written only if requested and never for substeps.
    if (const auto& [wantRFT, haveExistingRFT] =
        this->impl->wantRFTOutput(report_step, isSubstep);
        wantRFT)
    {
        // Open existing RFT file if report step is after first RFT event.
        const auto openExisting = EclIO::OutputStream::RFT::OpenExisting {
            haveExistingRFT
        };

        EclIO::OutputStream::RFT rftFile {
            EclIO::OutputStream::ResultSet { this->impl->outputDir,
                                             this->impl->baseName },
            EclIO::OutputStream::Formatted { ioConfig.getFMTOUT() },
            openExisting
        };

        RftIO::write(report_step, secs_elapsed, es.getUnits(),
                     grid, schedule, value.wells, rftFile);
    }

    if (!isSubstep) {
        for (const auto& report : schedule[report_step].rpt_config.get()) {
            std::stringstream ss;
            const auto& unit_system = this->impl->es.getUnits();

            RptIO::write_report(ss, report.first, report.second, schedule, grid, unit_system, report_step);

            auto log_string = ss.str();
            if (!log_string.empty())
                OpmLog::note(log_string);
        }
    }
 }


RestartValue EclipseIO::loadRestart(Action::State& action_state, SummaryState& summary_state, const std::vector<RestartKey>& solution_keys, const std::vector<RestartKey>& extra_keys) const {
    const auto& es                       = this->impl->es;
    const auto& grid                     = this->impl->grid;
    const auto& schedule                 = this->impl->schedule;
    const InitConfig& initConfig         = es.getInitConfig();
    const auto& ioConfig                 = es.getIOConfig();
    const int report_step                = initConfig.getRestartStep();
    const std::string filename           = ioConfig.getRestartFileName( initConfig.getRestartRootName(),
                                                                        report_step,
                                                                        false );

    return RestartIO::load(filename, report_step, action_state, summary_state, solution_keys,
                           es, grid, schedule, extra_keys);
}

EclipseIO::EclipseIO( const EclipseState& es,
                      EclipseGrid grid,
                      const Schedule& schedule,
                      const SummaryConfig& summary_config,
                      const std::string& baseName,
                      const bool writeEsmry
                    )
    : impl( new Impl( es, std::move( grid ), schedule , summary_config, baseName, writeEsmry) )
{
    if( !this->impl->output_enabled )
        return;

    ensure_directory_exists( this->impl->outputDir );
}

const out::Summary& EclipseIO::summary() {
    return this->impl->summary;
}


EclipseIO::~EclipseIO() {}

} // namespace Opm
