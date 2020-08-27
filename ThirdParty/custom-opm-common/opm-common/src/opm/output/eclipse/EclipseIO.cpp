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

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/RFTConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/RPTConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include <opm/parser/eclipse/Units/Dimension.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

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
#include <memory>     // unique_ptr
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <utility>    // move

#include <opm/common/utility/FileSystem.hpp>

namespace {

inline std::string uppercase( std::string x ) {
    std::transform( x.begin(), x.end(), x.begin(),
        []( char c ) { return std::toupper( c ); } );

    return x;
}

void ensure_directory_exists( const Opm::filesystem::path& odir )
{
    namespace fs = Opm::filesystem;

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
    Impl( const EclipseState&, EclipseGrid, const Schedule&, const SummaryConfig& );
        void writeINITFile( const data::Solution& simProps, std::map<std::string, std::vector<int> > int_data, const NNC& nnc) const;
        void writeEGRIDFile( const NNC& nnc );
        bool wantRFTOutput( const int report_step, const bool isSubstep ) const;

        const EclipseState& es;
        EclipseGrid grid;
        const Schedule& schedule;
        std::string outputDir;
        std::string baseName;
        SummaryConfig summaryConfig;
        out::Summary summary;
        bool output_enabled;
};

EclipseIO::Impl::Impl( const EclipseState& eclipseState,
                       EclipseGrid grid_,
                       const Schedule& schedule_,
                       const SummaryConfig& summary_config)
    : es( eclipseState )
    , grid( std::move( grid_ ) )
    , schedule( schedule_ )
    , outputDir( eclipseState.getIOConfig().getOutputDir() )
    , baseName( uppercase( eclipseState.getIOConfig().getBaseName() ) )
    , summaryConfig( summary_config )
    , summary( eclipseState, summaryConfig, grid , schedule )
    , output_enabled( eclipseState.getIOConfig().getOutputEnabled() )
{}


void EclipseIO::Impl::writeINITFile(const data::Solution&                   simProps,
                                    std::map<std::string, std::vector<int>> int_data,
                                    const NNC&                              nnc) const
{
    EclIO::OutputStream::Init initFile {
        EclIO::OutputStream::ResultSet { this->outputDir, this->baseName },
        EclIO::OutputStream::Formatted { this->es.cfg().io().getFMTOUT() }
    };

    InitIO::write(this->es, this->grid, this->schedule,
                  simProps, std::move(int_data), nnc, initFile);
}


void EclipseIO::Impl::writeEGRIDFile( const NNC& nnc ) {
    const auto formatted = this->es.cfg().io().getFMTOUT();

    const auto ext = '.'
        + (formatted ? std::string{"F"} : std::string{})
        + "EGRID";

    const auto egridFile = (Opm::filesystem::path{ this->outputDir }
        / (this->baseName + ext)).generic_string();

    this->grid.save( egridFile, formatted, nnc, this->es.getDeckUnitSystem());
}

bool EclipseIO::Impl::wantRFTOutput( const int  report_step,
                                     const bool isSubstep ) const
{
    return !isSubstep
        && (static_cast<std::size_t>(report_step)
            >= this->schedule.rftConfig().firstRFTOutput());
}

/*
int_data: Writes key(string) and integers vector to INIT file as eclipse keywords
- Key: Max 8 chars.
- Wrong input: invalid_argument exception.
*/
void EclipseIO::writeInitial( data::Solution simProps, std::map<std::string, std::vector<int> > int_data, const NNC& nnc) {
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
                              const SummaryState& st,
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

    /*
      Summary data is written unconditionally for every timestep except for the
      very intial report_step==0 call, which is only garbage.
    */
    if (report_step > 0) {
        this->impl->summary.add_timestep( st,
                                          report_step);
        this->impl->summary.write();
    }

    bool final_step { report_step == static_cast<int>(this->impl->schedule.size()) - 1 };

    if (final_step && this->impl->summaryConfig.createRunSummary()) {
        Opm::filesystem::path outputDir { this->impl->outputDir } ;
        Opm::filesystem::path outputFile { outputDir / this->impl->baseName } ;
        EclIO::ESmry(outputFile).write_rsm_file();
    }

    /*
      Current implementation will not write restart files for substep,
      but there is an unsupported option to the RPTSCHED keyword which
      will request restart output from every timestep.
    */
    if(!isSubstep && schedule.restart().getWriteRestartFile(report_step))
    {
        EclIO::OutputStream::Restart rstFile {
            EclIO::OutputStream::ResultSet { this->impl->outputDir,
                                             this->impl->baseName },
            report_step,
            EclIO::OutputStream::Formatted { ioConfig.getFMTOUT() },
            EclIO::OutputStream::Unified   { ioConfig.getUNIFOUT() }
        };

        RestartIO::save(rstFile, report_step, secs_elapsed, value,
                        es, grid, schedule, action_state, st, write_double);
    }

    // RFT file written only if requested and never for substeps.
    if (this->impl->wantRFTOutput(report_step, isSubstep)) {
        // Open existing RFT file if report step is after first RFT event.
        const auto openExisting = EclIO::OutputStream::RFT::OpenExisting {
            static_cast<std::size_t>(report_step)
            > schedule.rftConfig().firstRFTOutput()
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
        for (const auto& report : schedule.report_config(report_step)) {
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
                      const SummaryConfig& summary_config)
    : impl( new Impl( es, std::move( grid ), schedule , summary_config) )
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
