/*
  Copyright (c) 2018 Equinor ASA
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

#include <opm/output/eclipse/RestartIO.hpp>

#include <opm/output/eclipse/AggregateGroupData.hpp>
#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/AggregateConnectionData.hpp>
#include <opm/output/eclipse/AggregateMSWData.hpp>
#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/AggregateActionxData.hpp>

#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/io/eclipse/OutputStream.hpp>
#include <opm/io/eclipse/PaddedOutputString.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Tuning.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/Eqldims.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Opm { namespace RestartIO {

namespace {
    /*
      The RestartValue structure has an 'extra' container which can be used to
      add extra fields to the restart file. The extra field is used both to add
      OPM specific fields like 'OPMEXTRA', and eclipse standard fields like
      THRESHPR. In the case of e.g. THRESHPR this should - if present - be added
      in the SOLUTION section of the restart file. The extra_solution object
      identifies the keys which should be output in the solution section.
    */

    bool extraInSolution(const std::string& vector)
    {
        static const auto extra_solution =
            std::unordered_set<std::string>
        {
            "THRESHPR",
        };

        return extra_solution.count(vector) > 0;
    }

    double nextStepSize(const Opm::RestartValue& rst_value)
    {
        return rst_value.hasExtra("OPMEXTRA")
            ? rst_value.getExtra("OPMEXTRA")[0]
            : 0.0;
    }

    std::vector<int>
    serialize_OPM_IWEL(const data::Wells&              wells,
                       const std::vector<std::string>& well_names)
    {
      const auto getctrl = [&]( const std::string& wname ) {
            const auto itr = wells.find( wname );
            return itr == wells.end() ? 0 : itr->second.control;
        };

        std::vector<int> iwel(well_names.size(), 0.0);
        std::transform(well_names.begin(), well_names.end(), iwel.begin(), getctrl);

        return iwel;
    }

    std::vector<double>
    serialize_OPM_XWEL(const data::Wells&              wells,
                       const Schedule&                 schedule,
                       const std::vector<std::string>& well_names,
                       const int                       sim_step,
                       const Phases&                   phase_spec,
                       const EclipseGrid&              grid)
    {
        using rt = data::Rates::opt;

        std::vector<rt> phases;
        if (phase_spec.active(Phase::WATER)) phases.push_back(rt::wat);
        if (phase_spec.active(Phase::OIL))   phases.push_back(rt::oil);
        if (phase_spec.active(Phase::GAS))   phases.push_back(rt::gas);

        std::vector< double > xwel;
        for (const auto& wellname : well_names) {
            const auto& sched_well = schedule.getWell(wellname, sim_step);
            if (wells.count(wellname) == 0 ||
                sched_well.getStatus() == Opm::Well::Status::SHUT)
            {
                const auto elems = (sched_well.getConnections().size()
                                    * (phases.size() + data::Connection::restart_size))
                    + 3 /* bhp, thp, temperature */
                    + phases.size();

                // write zeros if no well data is provided
                xwel.insert( xwel.end(), elems, 0.0 );
                continue;
            }

            const auto& well = wells.at( wellname );

            xwel.push_back( well.bhp );
            xwel.push_back( well.thp );
            xwel.push_back( well.temperature );

            for (auto phase : phases)
                xwel.push_back(well.rates.get(phase));

            for (const auto& sc : sched_well.getConnections()) {
                const auto i = sc.getI(), j = sc.getJ(), k = sc.getK();

                const auto rs_size = phases.size() + data::Connection::restart_size;
                if (!grid.cellActive(i, j, k) || sc.state() == Connection::State::SHUT) {
                    xwel.insert(xwel.end(), rs_size, 0.0);
                    continue;
                }

                const auto global_index = grid.getGlobalIndex(i, j, k);

                const auto& connection =
                    std::find_if(well.connections.begin(),
                                 well.connections.end(),
                        [global_index](const data::Connection& c)
                    {
                        return c.index == global_index;
                    });

                if (connection == well.connections.end()) {
                    xwel.insert( xwel.end(), rs_size, 0.0 );
                    continue;
                }

                xwel.push_back(connection->pressure);
                xwel.push_back(connection->reservoir_rate);
                xwel.push_back(connection->cell_pressure);
                xwel.push_back(connection->cell_saturation_water);
                xwel.push_back(connection->cell_saturation_gas);
                xwel.push_back(connection->effective_Kh);

                for (auto phase : phases)
                    xwel.push_back(connection->rates.get(phase));
            }
        }

        return xwel;
    }

    void checkSaveArguments(const EclipseState& es,
                            const RestartValue& restart_value,
                            const EclipseGrid&  grid)
    {
        for (const auto& elm: restart_value.solution)
            if (elm.second.data.size() != grid.getNumActive())
                throw std::runtime_error("Wrong size on solution vector: " + elm.first);

        if (es.getSimulationConfig().getThresholdPressure().size() > 0) {
            // If the the THPRES option is active the restart_value should have a
            // THPRES field. This is not enforced here because not all the opm
            // simulators have been updated to include the THPRES values.
            if (!restart_value.hasExtra("THRESHPR")) {
                OpmLog::warning("This model has THPRES active - should have THPRES as part of restart data.");
                return;
            }

            const auto num_regions = es.getTableManager().getEqldims().getNumEquilRegions();
            const auto& thpres = restart_value.getExtra("THRESHPR");

            if (thpres.size() != num_regions * num_regions)
                throw std::runtime_error("THPRES vector has invalid size - should have num_region * num_regions.");
        }
    }

    std::vector<int>
    writeHeader(const int                     report_step,
                const int                     sim_step,
                const double                  next_step_size,
                const double                  simTime,
                const Schedule&               schedule,
                const EclipseGrid&            grid,
                const EclipseState&           es,
                EclIO::OutputStream::Restart& rstFile)
    {
        // write INTEHEAD to restart file
        auto ih = Helpers::
            createInteHead(es, grid, schedule, simTime,
                           report_step, // Should really be number of timesteps
                           report_step, sim_step);

        rstFile.write("INTEHEAD", ih);

        // write LOGIHEAD to restart file
        rstFile.write("LOGIHEAD", Helpers::createLogiHead(es));

        // write DOUBHEAD to restart file
        const auto dh = Helpers::createDoubHead(es, schedule, sim_step,
                                                simTime, next_step_size);
        rstFile.write("DOUBHEAD", dh);

        // return the inteHead vector
        return ih;
    }

    void writeGroup(int                           sim_step,
                    const UnitSystem&             units,
                    const Schedule&               schedule,
                    const Opm::SummaryState&      sumState,
                    const std::vector<int>&       ih,
                    EclIO::OutputStream::Restart& rstFile)
    {
        // write IGRP to restart file
        const size_t simStep = static_cast<size_t> (sim_step);

        auto  groupData = Helpers::AggregateGroupData(ih);

        groupData.captureDeclaredGroupData(schedule, units, simStep, sumState, ih);

        rstFile.write("IGRP", groupData.getIGroup());
        rstFile.write("SGRP", groupData.getSGroup());
        rstFile.write("XGRP", groupData.getXGroup());
        rstFile.write("ZGRP", groupData.getZGroup());
    }

    void writeMSWData(int                           sim_step,
                      const UnitSystem&             units,
                      const Schedule&               schedule,
                      const EclipseGrid&            grid,
                      const Opm::SummaryState&      sumState,
                      const Opm::data::Wells&       wells,
                      const std::vector<int>&       ih,
                      EclIO::OutputStream::Restart& rstFile)
    {
        // write ISEG, RSEG, ILBS and ILBR to restart file
        const auto simStep = static_cast<std::size_t> (sim_step);

        auto  MSWData = Helpers::AggregateMSWData(ih);
        MSWData.captureDeclaredMSWData(schedule, simStep, units,
                                       ih, grid, sumState, wells);

        rstFile.write("ISEG", MSWData.getISeg());
        rstFile.write("ILBS", MSWData.getILBs());
        rstFile.write("ILBR", MSWData.getILBr());
        rstFile.write("RSEG", MSWData.getRSeg());
    }

    void writeUDQ(const int                     report_step,
                  const int                     sim_step,
                  const Schedule&               schedule,
                  const SummaryState&           sum_state,
                  const std::vector<int>&       ih,
                  EclIO::OutputStream::Restart& rstFile)
    {
        if (report_step == 0) {
            // Initial condition.  No UDQs yet.
            return;
        }

        // write UDQ - data to restart file
        const std::size_t simStep = static_cast<size_t> (sim_step);

        const auto udqDims = Helpers::createUdqDims(schedule, simStep, ih);
        auto  udqData = Helpers::AggregateUDQData(udqDims);
        udqData.captureDeclaredUDQData(schedule, simStep, sum_state, ih);
        
        if (udqDims[0] >= 1) {
            rstFile.write("ZUDN", udqData.getZUDN());
            rstFile.write("ZUDL", udqData.getZUDL());
            rstFile.write("IUDQ", udqData.getIUDQ());
            if (udqDims[12] >= 1) rstFile.write("DUDF", udqData.getDUDF());
            if (udqDims[11] >= 1) rstFile.write("DUDG", udqData.getDUDG());
            if (udqDims[ 9] >= 1) rstFile.write("DUDW", udqData.getDUDW());
            if (udqDims[ 2] >= 1) rstFile.write("IUAD", udqData.getIUAD());
            if (udqDims[ 7] >= 1) rstFile.write("IUAP", udqData.getIUAP());
            if (udqDims[ 6] >= 1) rstFile.write("IGPH", udqData.getIGPH());
        }
    }

    void writeActionx(const int                     report_step,
                      const int                     sim_step,
                      const EclipseState&           es,
                      const Schedule&               schedule,
                      const Action::State&          action_state,
                      const SummaryState&           sum_state,
                      EclIO::OutputStream::Restart& rstFile)
    {
        if (report_step == 0) {
            // Initial condition.  No ACTION* data yet.
            return;
        }

        // write ACTIONX - data to restart file
        const std::size_t simStep = static_cast<size_t> (sim_step);
        
        const auto actDims = Opm::RestartIO::Helpers::createActionxDims(es.runspec(), schedule, simStep);
        auto  actionxData = Opm::RestartIO::Helpers::AggregateActionxData(actDims);
        actionxData.captureDeclaredActionxData(schedule, action_state, sum_state, actDims, simStep);
        
        if (actDims[0] >= 1) {
            rstFile.write("IACT", actionxData.getIACT());
            rstFile.write("SACT", actionxData.getSACT());
            rstFile.write("ZACT", actionxData.getZACT());
            rstFile.write("ZLACT", actionxData.getZLACT());
            rstFile.write("ZACN", actionxData.getZACN());
            rstFile.write("IACN", actionxData.getIACN());
            rstFile.write("SACN", actionxData.getSACN());
        }
    }

    void writeWell(int                             sim_step,
                   const bool                      ecl_compatible_rst,
                   const Phases&                   phases,
                   const UnitSystem&               units,
                   const EclipseGrid&              grid,
                   const Schedule&                 schedule,
                   const std::vector<std::string>& well_names,
                   const data::Wells&              wells,
                   const Opm::Action::State&       action_state,
                   const Opm::SummaryState&        sumState,
                   const std::vector<int>&         ih,
                   EclIO::OutputStream::Restart&   rstFile)
    {
        auto wellData = Helpers::AggregateWellData(ih);
        wellData.captureDeclaredWellData(schedule, units, sim_step, action_state, sumState, ih);
        wellData.captureDynamicWellData(schedule, sim_step,
                                        wells, sumState);

        rstFile.write("IWEL", wellData.getIWell());
        rstFile.write("SWEL", wellData.getSWell());
        rstFile.write("XWEL", wellData.getXWell());
        rstFile.write("ZWEL", wellData.getZWell());

        // Extended set of OPM well vectors
        if (!ecl_compatible_rst)
        {
            const auto opm_xwel =
                serialize_OPM_XWEL(wells, schedule, well_names,
                                   sim_step, phases, grid);

            const auto opm_iwel = serialize_OPM_IWEL(wells, well_names);

            rstFile.write("OPM_IWEL", opm_iwel);
            rstFile.write("OPM_XWEL", opm_xwel);
        }

        auto connectionData = Helpers::AggregateConnectionData(ih);
        connectionData.captureDeclaredConnData(schedule, grid, units,
                                               wells, sim_step);

        rstFile.write("ICON", connectionData.getIConn());
        rstFile.write("SCON", connectionData.getSConn());
        rstFile.write("XCON", connectionData.getXConn());
    }

    void writeDynamicData(const int                     sim_step,
                          const bool                    ecl_compatible_rst,
                          const Phases&                 phases,
                          const UnitSystem&             units,
                          const EclipseGrid&            grid,
                          const Schedule&               schedule,
                          const data::WellRates&        wellSol,
                          const Opm::Action::State&     action_state,
                          const Opm::SummaryState&      sumState,
                          const std::vector<int>&       inteHD,
                          EclIO::OutputStream::Restart& rstFile)
    {
        writeGroup(sim_step, units, schedule, sumState, inteHD, rstFile);

        // Write well and MSW data only when applicable (i.e., when present)
        const auto& wells = schedule.wellNames(sim_step);

        if (! wells.empty()) {
            const auto haveMSW =
                std::any_of(std::begin(wells), std::end(wells),
                    [&schedule, sim_step](const std::string& well)
                {
                    return schedule.getWell(well, sim_step).isMultiSegment();
                });

            if (haveMSW) {
                writeMSWData(sim_step, units, schedule, grid, sumState,
                             wellSol, inteHD, rstFile);
            }

            writeWell(sim_step, ecl_compatible_rst,
                      phases, units, grid, schedule, wells,
                      wellSol, action_state, sumState, inteHD, rstFile);
        }
    }

    bool haveHysteresis(const RestartValue& value)
    {
        for (const auto* key : { "KRNSW_OW", "PCSWM_OW",
                                 "KRNSW_GO", "PCSWM_GO", })
        {
            if (value.solution.has(key)) { return true; }
        }

        return false;
    }

    std::vector<double>
    convertedHysteresisSat(const RestartValue& value,
                           const std::string&  primary,
                           const std::string&  fallback)
    {
        auto smax = std::vector<double>{};

        if (value.solution.has(primary)) {
            smax = value.solution.data(primary);
        }
        else if (value.solution.has(fallback)) {
            smax = value.solution.data(fallback);
        }

        if (! smax.empty()) {
            std::transform(std::begin(smax), std::end(smax), std::begin(smax),
                           [](const double s) { return 1.0 - s; });
        }

        return smax;
    }

    std::vector<std::string>
    solutionVectorNames(const RestartValue& value)
    {
        auto vectors = std::vector<std::string>{};
        vectors.reserve(value.solution.size());

        for (const auto& [name, vector] : value.solution) {
            if (vector.target == data::TargetType::RESTART_SOLUTION) {
                vectors.push_back(name);
            }
        }

        return vectors;
    }

    std::vector<std::string>
    extendedSolutionVectorNames(const RestartValue& value)
    {
        auto vectors = std::vector<std::string>{};
        vectors.reserve(value.solution.size());

        for (const auto& [name, vector] : value.solution) {
            if ((vector.target == data::TargetType::RESTART_AUXILIARY) ||
                (vector.target == data::TargetType::RESTART_OPM_EXTENDED))
            {
                vectors.push_back(name);
            }
        }

        return vectors;
    }

    template <class OutputVector>
    void writeSolutionVectors(const RestartValue&             value,
                              const std::vector<std::string>& vectors,
                              const bool                      write_double,
                              OutputVector&&                  writeVector)
    {
        for (const auto& vector : vectors) {
            writeVector(vector, value.solution.data(vector), write_double);
        }
    }

    template <class OutputVector>
    void writeRegularSolutionVectors(const RestartValue& value,
                                     const bool          write_double,
                                     OutputVector&&      writeVector)
    {
        writeSolutionVectors(value, solutionVectorNames(value), write_double,
                             std::forward<OutputVector>(writeVector));
    }

    template <class OutputVector>
    void writeExtendedSolutionVectors(const RestartValue& value,
                                      const bool          write_double,
                                      OutputVector&&      writeVector)
    {
        writeSolutionVectors(value, extendedSolutionVectorNames(value), write_double,
                             std::forward<OutputVector>(writeVector));
    }

    template <class OutputVector>
    void writeExtraVectors(const RestartValue& value,
                           OutputVector&&      writeVector)
    {
        for (const auto& elm : value.extra) {
            const std::string& key = elm.first.key;
            if (extraInSolution(key)) {
                // Observe that the extra data is unconditionally
                // output as double precision.
                writeVector(key, elm.second, true);
            }
        }
    }

    template <class OutputVector>
    void writeEclipseCompatHysteresis(const RestartValue& value,
                                      const bool          write_double,
                                      OutputVector&&      writeVector)
    {
        // Convert Flow-specific vectors {KRNSW,PCSWM}_OW to ECLIPSE's
        // requisite SOMAX vector.  Only partially characterised.
        // Sufficient for Norne.
        {
            const auto somax =
                convertedHysteresisSat(value, "KRNSW_OW", "PCSWM_OW");

            if (! somax.empty()) {
                writeVector("SOMAX", somax, write_double);
            }
        }

        // Convert Flow-specific vectors {KRNSW,PCSWM}_GO to ECLIPSE's
        // requisite SGMAX vector.  Only partially characterised.
        // Sufficient for Norne.
        {
            const auto sgmax =
                convertedHysteresisSat(value, "KRNSW_GO", "PCSWM_GO");

            if (! sgmax.empty()) {
                writeVector("SGMAX", sgmax, write_double);
            }
        }
    }

    void writeSolution(const RestartValue&           value,
                       const Schedule&               schedule,
                       const SummaryState&           sum_state,
                       int                           report_step,
                       int                           sim_step,
                       const bool                    ecl_compatible_rst,
                       const bool                    write_double_arg,
                       const std::vector<int>&       inteHD,
                       EclIO::OutputStream::Restart& rstFile)
    {
        auto write = [&rstFile]
            (const std::string&         key,
             const std::vector<double>& data,
             const bool                 write_double) -> void
        {
            if (write_double) {
                rstFile.write(key, data);
            }
            else {
                rstFile.write(key, std::vector<float> {
                    data.begin(), data.end()
                });
            }
        };

        rstFile.message("STARTSOL");

        writeRegularSolutionVectors(value, write_double_arg, write);

        writeUDQ(report_step, sim_step, schedule, sum_state, inteHD, rstFile);

        writeExtraVectors(value, write);

        if (ecl_compatible_rst && haveHysteresis(value)) {
            writeEclipseCompatHysteresis(value, write_double_arg, write);
        }

        if (! ecl_compatible_rst) {
            writeExtendedSolutionVectors(value, write_double_arg, write);
        }

        rstFile.message("ENDSOL");
    }

    void writeExtraData(const RestartValue::ExtraVector& extra_data,
                        EclIO::OutputStream::Restart&    rstFile)
    {
        for (const auto& extra_value : extra_data) {
            const std::string& key = extra_value.first.key;

            if (! extraInSolution(key)) {
                rstFile.write(key, extra_value.second);
            }
        }
    }

    int numChar(const std::size_t num_reports)
    {
        return static_cast<int>(
            1 + std::floor(std::log10(static_cast<double>(num_reports))));
    }

    void logRestartOutput(const int               report_step,
                          const std::size_t       num_reports,
                          const std::vector<int>& inteHD)
    {
        using Ix = ::Opm::RestartIO::Helpers::VectorItems::intehead;

        std::ostringstream logmsg;

        logmsg << "Restart file written for report step: "
               << std::setw(numChar(num_reports)) << report_step << '/'
               << std::setw(0) << num_reports << ".  Date: "
               << std::setw(4) <<                      inteHD[Ix::YEAR]  << '/'
               << std::setw(2) << std::setfill('0') << inteHD[Ix::MONTH] << '/'
               << std::setw(2) << std::setfill('0') << inteHD[Ix::DAY];

        ::Opm::OpmLog::info(logmsg.str());
    }

} // Anonymous namespace

void save(EclIO::OutputStream::Restart& rstFile,
          int                           report_step,
          double                        seconds_elapsed,
          RestartValue                  value,
          const EclipseState&           es,
          const EclipseGrid&            grid,
          const Schedule&               schedule,
          const Action::State&          action_state,
          const SummaryState&           sumState,
          bool                          write_double)
{
    ::Opm::RestartIO::checkSaveArguments(es, value, grid);

    const auto& ioCfg = es.getIOConfig();
    const auto ecl_compatible_rst = ioCfg.getEclCompatibleRST();

    const auto  sim_step = std::max(report_step - 1, 0);
    const auto& units    = es.getUnits();

    if (ecl_compatible_rst) {
        write_double = false;
    }

    // Convert solution fields and extra values from SI to user units.
    value.convertFromSI(units);

    const auto inteHD =
        writeHeader(report_step, sim_step, nextStepSize(value),
                    seconds_elapsed, schedule, grid, es, rstFile);

    if (report_step > 0) {
        writeDynamicData(sim_step, ecl_compatible_rst, es.runspec().phases(),
                         units, grid, schedule, value.wells, action_state, sumState,
                         inteHD, rstFile);
    }

    writeActionx(report_step, sim_step, es, schedule, action_state, sumState, rstFile);

    writeSolution(value, schedule, sumState, report_step, sim_step,
                  ecl_compatible_rst, write_double, inteHD, rstFile);

    if (! ecl_compatible_rst) {
        writeExtraData(value.extra, rstFile);
    }

    logRestartOutput(report_step, schedule.getTimeMap().numTimesteps(), inteHD);
}

}} // Opm::RestartIO
