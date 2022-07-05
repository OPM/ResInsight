/*
  Copyright 2019-2020 Equinor ASA
  Copyright 2018 Statoil ASA

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

#include <opm/output/eclipse/AggregateWellData.hpp>

#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/output/data/Wells.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionAST.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionResult.hpp>
#include <opm/input/eclipse/Schedule/Action/Actions.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/ScheduleTypes.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/VFPProdTable.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Well/WellEconProductionLimits.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>
#include <opm/input/eclipse/EclipseState/TracerConfig.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iterator>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

namespace VI = Opm::RestartIO::Helpers::VectorItems;

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateWellData
// ---------------------------------------------------------------------

namespace {
    std::size_t numWells(const std::vector<int>& inteHead)
    {
        return inteHead[VI::intehead::NWELLS];
    }

    int maxNumGroups(const std::vector<int>& inteHead)
    {
        return inteHead[VI::intehead::NWGMAX];
    }

    std::string trim(const std::string& s)
    {
        const auto b = s.find_first_not_of(" \t");
        if (b == std::string::npos) {
            // All blanks.  Return empty.
            return "";
        }

        const auto e = s.find_last_not_of(" \t");
        assert ((e != std::string::npos) && "Logic Error");

        // Remove leading/trailing blanks.
        return s.substr(b, e - b + 1);
    }

    template <typename WellOp>
    void wellLoop(const std::vector<std::string>& wells,
                  const Opm::Schedule&            sched,
                  const std::size_t               simStep,
                  WellOp&&                        wellOp)
    {
        for (const auto& wname : wells) {
            const auto& well = sched.getWell(wname, simStep);
            wellOp(well, well.seqIndex());
        }
    }

    namespace IWell {
        std::size_t entriesPerWell(const std::vector<int>& inteHead)
        {
            return inteHead[VI::intehead::NIWELZ];
        }

        Opm::RestartIO::Helpers::WindowedArray<int>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

            return WV {
                WV::NumWindows{ numWells(inteHead) },
                WV::WindowSize{ entriesPerWell(inteHead) }
            };
        }

        std::map <const std::string, size_t>  currentGroupMapNameIndex(const Opm::Schedule& sched, const size_t simStep, const std::vector<int>& inteHead)
        {
            // make group name to index map for the current time step
            std::map <const std::string, size_t> groupIndexMap;
            for (const auto& group_name : sched.groupNames(simStep)) {
                const auto& group = sched.getGroup(group_name, simStep);
                int ind = (group.name() == "FIELD")
                    ? inteHead[VI::intehead::NGMAXZ]-1 : group.insert_index()-1;
                std::pair<const std::string, size_t> groupPair = std::make_pair(group.name(), ind);
                groupIndexMap.insert(groupPair);
            }
            return groupIndexMap;
        }

        int groupIndex(const std::string&              grpName,
                       const std::map <const std::string, size_t>&  currentGroupMapNameIndex)
        {
            int ind = 0;
            auto searchGTName = currentGroupMapNameIndex.find(grpName);
            if (searchGTName != currentGroupMapNameIndex.end()) {
                ind = searchGTName->second + 1;
            }
            else {
                std::cout << "group Name: " << grpName << std::endl;
                throw std::invalid_argument( "Invalid group name" );
            }
            return ind;
        }


        int wellVFPTab(const Opm::Well& well, const Opm::SummaryState& st)
        {
            if (well.isInjector()) {
                return well.injectionControls(st).vfp_table_number;
            }
            return well.productionControls(st).vfp_table_number;
        }

        bool wellControlDefined(const Opm::data::Well& xw)
        {
            using PMode = ::Opm::Well::ProducerCMode;
            using IMode = ::Opm::Well::InjectorCMode;

            const auto& curr = xw.current_control;

            return (curr.isProducer && (curr.prod != PMode::CMODE_UNDEFINED))
                || (!curr.isProducer && (curr.inj != IMode::CMODE_UNDEFINED));
        }

        int ctrlMode(const Opm::Well& well, const Opm::data::Well& xw)
        {
            const auto& curr = xw.current_control;

            if (curr.isProducer) {
                return Opm::Well::eclipseControlMode(curr.prod);
            }
            else { // injector
                return Opm::Well::eclipseControlMode(curr.inj, well.injectorType());
            }
        }

        int compOrder(const Opm::Well& well)
        {
            using WCO   = ::Opm::Connection::Order;
            using COVal = ::Opm::RestartIO::Helpers::
                VectorItems::IWell::Value::CompOrder;

            switch (well.getConnections().ordering()) {
            case WCO::TRACK: return COVal::Track;
            case WCO::DEPTH: return COVal::Depth;
            case WCO::INPUT: return COVal::Input;
            }

            return 0;
        }

        int PLossMod(const Opm::Well& well)
        {
            using CPD   = ::Opm::WellSegments::CompPressureDrop;
            using PLM = ::Opm::RestartIO::Helpers::
                VectorItems::IWell::Value::PLossMod;
            switch (well.getSegments().compPressureDrop()) {
                case CPD::HFA: return PLM::HFA;
                case CPD::HF_: return PLM::HF_;
                case CPD::H__: return PLM::H__;
            }

            return 0;
        }

        /*int MPhaseMod(const Opm::Well& well)
        {
            using MPM   = ::Opm::WellSegments::MultiPhaseModel;
            using MUM = ::Opm::RestartIO::Helpers::
                VectorItems::IWell::Value::MPMod;
            switch (well.getSegments().multiPhaseModel()) {
                case MPM::HO: return MUM::HO;
                case MPM::DF: return MUM::DF;
            }

            return 0;
        }*/

        int wellStatus(Opm::Well::Status status) {
            using Value = VI::IWell::Value::Status;
            switch (status) {
            case Opm::Well::Status::OPEN:
                return Value::Open;
            case Opm::Well::Status::STOP:
                return Value::Stop;
            case Opm::Well::Status::SHUT:
                return Value::Shut;
            case Opm::Well::Status::AUTO:
                return Value::Auto;
            default:
                throw std::logic_error("Unhandled enum value");
            }
        }

        int preferredPhase(const Opm::Well& well)
        {
            using PhaseVal = VI::IWell::Value::Preferred_Phase;

            switch (well.getPreferredPhase()) {
            case Opm::Phase::OIL:   return PhaseVal::Oil;
            case Opm::Phase::GAS:   return PhaseVal::Gas;
            case Opm::Phase::WATER: return PhaseVal::Water;

            // Should have LIQUID here too...

            default:
                throw std::invalid_argument {
                    "Unsupported Preferred Phase '" +
                    std::to_string(static_cast<int>(well.getPreferredPhase()))
                    + '\''
                };
            }
        }

        template <typename IWellArray>
        void setHistoryControlMode(const Opm::Well& well,
                                   const int        curr,
                                   IWellArray&      iWell)
        {
            iWell[VI::IWell::index::HistReqWCtrl] =
                well.predictionMode() ? 0 : curr;
        }

        template <typename IWellArray>
        void setCurrentControl(const int   curr,
                               IWellArray& iWell)
        {
            iWell[VI::IWell::index::ActWCtrl] = curr;
        }

        template <typename IWellArray>
        void assignGasliftOpt(const std::string&     well,
                              const Opm::GasLiftOpt& glo,
                              IWellArray&            iWell)
        {
            using Ix = VI::IWell::index;

            if (! glo.has_well(well)) {
                return;
            }

            // Integer flag indicating lift gas optimisation to be calculated
            const auto& w_glo = glo.well(well);

            iWell[Ix::LiftOpt] = w_glo.use_glo();
            iWell[Ix::LiftOptAllocExtra] = w_glo.alloc_extra_gas();
        }

        template <typename IWellArray>
        void assignMSWInfo(const Opm::Well&  well,
                           const std::size_t msWellID,
                           IWellArray&       iWell)
        {
            using Ix = VI::IWell::index;

            // Multi-segmented well information
            iWell[Ix::MsWID] = 0;  // MS Well ID (0 or 1..#MS wells)
            iWell[Ix::NWseg] = 0;  // Number of well segments
            iWell[Ix::MSW_PlossMod] = 0;  // Segment pressure loss model
            iWell[Ix::MSW_MulPhaseMod] = 0;  // Segment multi phase flow model

            if (well.isMultiSegment()) {
                iWell[Ix::MsWID] = static_cast<int>(msWellID);
                iWell[Ix::NWseg] = well.getSegments().size();
                iWell[Ix::MSW_PlossMod] = PLossMod(well);
                iWell[Ix::MSW_MulPhaseMod] = 1;  // temporary solution - valid for HO - multiphase model - only implemented now
                //iWell[Ix::MSW_MulPhaseMod] = MPhaseMod(well);
            }
        }

        template <typename IWellArray>
        void assignWellTest(const std::string&         well,
                            const Opm::WellTestConfig& wtest_config,
                            const Opm::WellTestState&  wtest_state,
                            IWellArray&                iWell)
        {
            using Ix = VI::IWell::index;

            const auto wtest_rst = wtest_state.restart_well(wtest_config, well);

            if (! wtest_rst.has_value()) {
                return;
            }

            iWell[Ix::WTestConfigReason] = wtest_rst->config_reasons;
            iWell[Ix::WTestCloseReason] = wtest_rst->close_reason;
            iWell[Ix::WTestRemaining] = wtest_rst->num_test;
        }

        int wgrupConGuideratePhase(const Opm::Well::GuideRateTarget grTarget)
        {
            using GRTarget = Opm::Well::GuideRateTarget;
            using GRPhase = VI::IWell::Value::WGrupCon::GRPhase;

            switch (grTarget) {
            case GRTarget::OIL:       return GRPhase::Oil;
            case GRTarget::WAT:       return GRPhase::Water;
            case GRTarget::GAS:       return GRPhase::Gas;
            case GRTarget::LIQ:       return GRPhase::Liquid;
            case GRTarget::RAT:       return GRPhase::SurfaceInjectionRate;
            case GRTarget::RES:       return GRPhase::ReservoirVolumeRate;
            case GRTarget::UNDEFINED: return GRPhase::Defaulted;

            default:
                throw std::invalid_argument {
                    fmt::format("Unsupported guiderate phase '{}' for restart",
                                Opm::Well::GuideRateTarget2String(grTarget))
                };
            }
        }

        template <typename IWellArray>
        void assignWGrupCon(const Opm::Well& well,
                            IWellArray&      iWell)
        {
            using Ix = VI::IWell::index;
            namespace Value = VI::IWell::Value;

            iWell[Ix::WGrupConControllable] = well.isAvailableForGroupControl()
                ? Value::WGrupCon::Controllable::Yes // Common case
                : Value::WGrupCon::Controllable::No;

            iWell[Ix::WGrupConGRPhase] =
                wgrupConGuideratePhase(well.getRawGuideRatePhase());
        }

        template <typename IWellArray>
        void assignTHPLookupOptions(const Opm::Well& well,
                                    IWellArray&      iWell)
        {
            using Ix = VI::IWell::index;
            namespace Value = VI::IWell::Value;

            const auto& options = well.getWVFPEXP();

            iWell[Ix::THPLookupVFPTable] = options.explicit_lookup()
                ? Value::WVfpExp::Lookup::Explicit
                : Value::WVfpExp::Lookup::Implicit;

            iWell[Ix::CloseWellIfTHPStabilised] = options.shut()
                ? static_cast<int>(Value::WVfpExp::CloseStabilised::Yes)
                : static_cast<int>(Value::WVfpExp::CloseStabilised::No);

            auto& prevent = iWell[Ix::PreventTHPIfUnstable];
            if (options.report_first()) {
                prevent = static_cast<int>(Value::WVfpExp::PreventTHP::Yes1);
            }
            else if (options.report_every()) {
                prevent = static_cast<int>(Value::WVfpExp::PreventTHP::Yes2);
            }
            else {
                prevent = static_cast<int>(Value::WVfpExp::PreventTHP::No);
            }
        }

        int workoverProcedure(const Opm::WellEconProductionLimits::EconWorkover procedure)
        {
            namespace Value = VI::IWell::Value;
            using WO = Opm::WellEconProductionLimits::EconWorkover;

            switch (procedure) {
            case WO::NONE: return Value::EconLimit::WOProcedure::None;
            case WO::CON:  return Value::EconLimit::WOProcedure::Con;
            case WO::CONP: return Value::EconLimit::WOProcedure::ConAndBelow;
            case WO::WELL: return Value::EconLimit::WOProcedure::StopOrShut;
            case WO::PLUG: return Value::EconLimit::WOProcedure::Plug;

            default:
                throw std::invalid_argument {
                    fmt::format("Unsupported workover procedure '{}' for restart",
                                Opm::WellEconProductionLimits::EconWorkover2String(procedure))
                };
            }
        }

        int econLimitQuantity(const Opm::WellEconProductionLimits::QuantityLimit quantity)
        {
            namespace Value = VI::IWell::Value;
            using Quant = Opm::WellEconProductionLimits::QuantityLimit;

            switch (quantity) {
            case Quant::RATE: return Value::EconLimit::Quantity::Rate;
            case Quant::POTN: return Value::EconLimit::Quantity::Potential;
            }

            throw std::invalid_argument {
                fmt::format("Unsupported economic limit quantity '{}' for restart",
                            Opm::WellEconProductionLimits::QuantityLimit2String(quantity))
            };
        }

        template <typename IWellArray>
        void assignEconomicLimits(const Opm::Well& well,
                                  IWellArray&      iWell)
        {
            using Ix = VI::IWell::index;
            namespace Value = VI::IWell::Value;

            const auto& limits = well.getEconLimits();

            iWell[Ix::EconWorkoverProcedure] = workoverProcedure(limits.workover());
            iWell[Ix::EconWorkoverProcedure_2] =
                workoverProcedure(limits.workoverSecondary());

            iWell[Ix::EconLimitEndRun] = limits.endRun()
                ? Value::EconLimit::EndRun::Yes
                : Value::EconLimit::EndRun::No; // Common case

            iWell[Ix::EconLimitQuantity] = econLimitQuantity(limits.quantityLimit());
        }

        template <class IWellArray>
        void staticContrib(const Opm::Well&                well,
                           const Opm::GasLiftOpt&          glo,
                           const Opm::WellTestConfig&      wtest_config,
                           const Opm::WellTestState&       wtest_state,
                           const Opm::SummaryState&        st,
                           const std::size_t               msWellID,
                           const std::map <const std::string, size_t>&  GroupMapNameInd,
                           IWellArray&                     iWell)
        {
            using Ix = VI::IWell::index;

            iWell[Ix::IHead] = well.getHeadI() + 1;
            iWell[Ix::JHead] = well.getHeadJ() + 1;
            iWell[Ix::Status] = wellStatus(well.getStatus());

            // Connections
            {
                const auto& conn = well.getConnections();

                iWell[Ix::NConn]  = static_cast<int>(conn.size());

                if (well.isMultiSegment()) {
                    // Set top and bottom connections to zero for multi
                    // segment wells
                    iWell[Ix::FirstK] = 0;
                    iWell[Ix::LastK]  = 0;
                }
                else {
                    iWell[Ix::FirstK] = (iWell[Ix::NConn] == 0)
                        ? 0 : conn.get(0).getK() + 1;

                    iWell[Ix::LastK] = (iWell[Ix::NConn] == 0)
                        ? 0 : conn.get(conn.size() - 1).getK() + 1;
                }
            }

            iWell[Ix::Group] =
                groupIndex(trim(well.groupName()), GroupMapNameInd);

            iWell[Ix::WType]  = well.wellType().ecl_wtype();
            iWell[Ix::VFPTab] = wellVFPTab(well, st);
            iWell[Ix::XFlow]  = well.getAllowCrossFlow() ? 1 : 0;

            iWell[Ix::PreferredPhase] = preferredPhase(well);

            // The following items aren't fully characterised yet, but
            // needed for restart of M2.  Will need further refinement.
            iWell[Ix::item18] = -100;
            iWell[Ix::item32] =    7;
            iWell[Ix::item48] = -  1;

            // Deliberate misrepresentation.  Function 'eclipseControlMode'
            // returns the target control mode requested in the simulation
            // deck.  This item is supposed to be the well's actual, active
            // target control mode in the simulator.
            //
            // Observe that setCurrentControl() is called again, for open
            // wells, in the dynamicContrib() function.
            setCurrentControl(Opm::Well::eclipseControlMode(well, st), iWell);
            setHistoryControlMode(well, Opm::Well::eclipseControlMode(well, st), iWell);

            iWell[Ix::CompOrd] = compOrder(well);

            assignGasliftOpt(well.name(), glo, iWell);
            assignMSWInfo(well, msWellID, iWell);
            assignWGrupCon(well, iWell);
            assignTHPLookupOptions(well, iWell);
            assignEconomicLimits(well, iWell);
            assignWellTest(well.name(), wtest_config, wtest_state, iWell);
        }

        template <class IWellArray>
        void dynamicContribShut(IWellArray& iWell)
        {
            using Ix = VI::IWell::index;
            using Value = VI::IWell::Value::Status;

            iWell[Ix::item9 ] = -1000;
            iWell[Ix::Status] = Value::Shut;
        }

        template <class IWellArray>
        void dynamicContribStop(const Opm::data::Well&  xw,
                                IWellArray&             iWell)
        {
            using Ix = VI::IWell::index;
            using Value = VI::IWell::Value::Status;

            const auto any_flowing_conn =
                std::any_of(std::begin(xw.connections),
                            std::end  (xw.connections),
                    [](const Opm::data::Connection& c)
                {
                    return c.rates.flowing();
                });

            iWell[Ix::item9] = any_flowing_conn
                ? 0 : -1;

            iWell[Ix::Status] = any_flowing_conn
                ? Value::Stop  : Value::Shut;

        }

        template <class IWellArray>
        void dynamicContribOpen(const Opm::Well&       well,
                                const Opm::data::Well& xw,
                                IWellArray&            iWell)
        {
            using Ix = VI::IWell::index;
            using Value = VI::IWell::Value::Status;

            if (wellControlDefined(xw)) {
                setCurrentControl(ctrlMode(well, xw), iWell);
            }

            const auto any_flowing_conn =
                std::any_of(std::begin(xw.connections),
                            std::end  (xw.connections),
                    [](const Opm::data::Connection& c)
                {
                    return c.rates.flowing();
                });

            iWell[Ix::item9] = any_flowing_conn
                ? iWell[Ix::ActWCtrl] : -1;

            iWell[Ix::Status] = any_flowing_conn
                ? Value::Open : Value::Shut;
        }
    } // IWell

    namespace SWell {
        std::size_t entriesPerWell(const std::vector<int>& inteHead)
        {
            assert ((inteHead[VI::intehead::NSWELZ] > 121) &&
                    "SWEL must allocate at least 122 elements per well");

            return inteHead[VI::intehead::NSWELZ];
        }

        std::optional<float> datumDepth(const Opm::Well& well)
        {
            if (well.isMultiSegment()) {
                // Datum depth for multi-segment wells is
                // depth of top-most segment.
                return well.getSegments().depthTopSegment();
            }

            // Not a multi-segment well--i.e., this is a regular well.  Use
            // well's reference depth if available.  Otherwise signal a
            // missing value by std::nullopt.  Missing values *might* come
            // from WELSPECS(5) being defaulted in wells with no active
            // reservoir connections.
            return well.hasRefDepth()
                ? std::optional<float>{ well.getRefDepth() }
                : std::nullopt;
        }

        Opm::RestartIO::Helpers::WindowedArray<float>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<float>;

            return WV {
                WV::NumWindows{ numWells(inteHead) },
                WV::WindowSize{ entriesPerWell(inteHead) }
            };
        }

        std::vector<float> defaultSWell()
        {
            const auto dflt  = -1.0e+20f;
            const auto infty =  1.0e+20f;
            const auto zero  =  0.0f;
            const auto one   =  1.0f;
            const auto half  =  0.5f;

            // Initial data by Statoil ASA.
            return { // 122 Items (0..121)
                // 0     1      2      3      4      5
                infty, infty, infty, infty, infty, infty,    //   0..  5  ( 0)
                one  , zero , zero , zero , zero , 1.0e-05f, //   6.. 11  ( 1)
                zero , zero , infty, infty, zero , dflt ,    //  12.. 17  ( 2)
                infty, infty, infty, infty, infty, zero ,    //  18.. 23  ( 3)
                one  , zero , zero , zero , zero , zero ,    //  24.. 29  ( 4)
                zero , one  , zero , infty, zero , zero ,    //  30.. 35  ( 5)
                zero , zero , zero , zero , zero , zero ,    //  36.. 41  ( 6)
                zero , zero , zero , zero , zero , zero ,    //  42.. 47  ( 7)
                zero , zero , zero , zero , zero , zero ,    //  48.. 53  ( 8)
                infty, zero , zero , zero , zero , zero ,    //  54.. 59  ( 9)
                zero , zero , zero , zero , zero , zero ,    //  60.. 65  (10)
                zero , zero , zero , zero , zero , zero ,    //  66.. 71  (11)
                zero , zero , zero , zero , zero , zero ,    //  72.. 77  (12)
                zero , infty, infty, zero , zero , one  ,    //  78.. 83  (13)
                one  , one  , zero , infty, zero , infty,    //  84.. 89  (14)
                one  , dflt , one  , zero , zero , zero ,    //  90.. 95  (15)
                zero , zero , zero , zero , zero , zero ,    //  96..101  (16)
                zero , zero , zero , zero , zero , zero ,    // 102..107  (17)
                zero , zero , half , one  , zero , zero ,    // 108..113  (18)
                zero , zero , zero , zero , zero , infty,    // 114..119  (19)
                zero , one  ,                                // 120..121  (20)
            };
        }

        template <class SWellArray>
        void assignDefaultSWell(SWellArray& sWell)
        {
            const auto& init = defaultSWell();

            const auto sz = static_cast<
                decltype(init.size())>(sWell.size());

            auto b = std::begin(init);
            auto e = b + std::min(init.size(), sz);

            std::copy(b, e, std::begin(sWell));
        }

        float getRateLimit(const Opm::UnitSystem&         units,
                           const Opm::UnitSystem::measure u,
                           const double                   rate)
        {
            float rLimit = 1.0e+20f;

            if (rate > 0.0) {
                rLimit = static_cast<float>(units.from_si(u, rate));
            }
            else if (rate < 0.0) {
                rLimit = 0.0;
            }

            return rLimit;
        }

        template <class SWProp, class SWellArray>
        void assignOWGRateTargetsProd(const Opm::Well::ProductionControls& pc,
                                      const bool                           predMode,
                                      SWProp&&                             swprop,
                                      SWellArray&                          sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            if (predMode) {
                if (pc.oil_rate != 0.0) {
                    sWell[Ix::OilRateTarget] = swprop(M::liquid_surface_rate, pc.oil_rate);
                }

                if (pc.water_rate != 0.0) {
                    sWell[Ix::WatRateTarget] = swprop(M::liquid_surface_rate, pc.water_rate);
                }

                if (pc.gas_rate != 0.0) {
                    sWell[Ix::GasRateTarget] = swprop(M::gas_surface_rate, pc.gas_rate);

                    sWell[Ix::HistGasRateTarget] = sWell[Ix::GasRateTarget];
                }
            }
            else {
                sWell[Ix::OilRateTarget] =
                    swprop(M::liquid_surface_rate, pc.oil_rate);

                sWell[Ix::WatRateTarget] =
                    swprop(M::liquid_surface_rate, pc.water_rate);

                sWell[Ix::GasRateTarget] =
                    swprop(M::gas_surface_rate, pc.gas_rate);

                sWell[Ix::HistGasRateTarget] = sWell[Ix::GasRateTarget];
            }
        }

        template <class SWProp, class SWellArray>
        void assignLiqRateTargetsProd(const Opm::Well::ProductionControls& pc,
                                      const bool                           predMode,
                                      SWProp&&                             swprop,
                                      SWellArray&                          sWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            if (pc.liquid_rate != 0.0) {    // check if this works - may need to be rewritten
                sWell[Ix::LiqRateTarget] = swprop(M::liquid_surface_rate, pc.liquid_rate);

                sWell[Ix::HistLiqRateTarget] = sWell[Ix::LiqRateTarget];
            }
            else if (!predMode)  {
                sWell[Ix::LiqRateTarget] =
                    swprop(M::liquid_surface_rate, pc.oil_rate + pc.water_rate);
            }
        }

        template <class SWProp, class SWellArray>
        void assignResVRateTargetsProd(const std::string&                   well,
                                       const Opm::SummaryState&             smry,
                                       const Opm::Well::ProductionControls& pc,
                                       const bool                           predMode,
                                       SWProp&&                             swprop,
                                       SWellArray&                          sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            if (pc.resv_rate != 0.0)  {
                sWell[Ix::ResVRateTarget] = swprop(M::rate, pc.resv_rate);
            }
            else if (!predMode) {
                // Write out summary voidage production rate if target/limit
                // is not set
                const auto vr = smry.get_well_var("WVPR", well, 0.0);
                if (vr != 0.0) {
                    sWell[Ix::ResVRateTarget] = static_cast<float>(vr);
                }
            }
        }

        template <class SWProp, class SWellArray>
        void assignALQProd(const Opm::VFPProdTable::ALQ_TYPE alqType,
                           const double                      alq_value,
                           SWProp&&                          swprop,
                           SWellArray&                       sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            if (alqType == Opm::VFPProdTable::ALQ_TYPE::ALQ_GRAT) {
                sWell[Ix::Alq_value] = swprop(M::gas_surface_rate, alq_value);
            }
            else if ((alqType == Opm::VFPProdTable::ALQ_TYPE::ALQ_IGLR) ||
                     (alqType == Opm::VFPProdTable::ALQ_TYPE::ALQ_TGLR))
            {
                sWell[Ix::Alq_value] = swprop(M::gas_oil_ratio, alq_value);
            }
            else {
                // Note: Not all ALQ types have associated units of
                // measurement.
                sWell[Ix::Alq_value] = alq_value;
            }
        }

        template <class SWellArray>
        void assignPredictionTargetsProd(const Opm::UnitSystem&               units,
                                         const Opm::Well::ProductionControls& pc,
                                         SWellArray&                          sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            sWell[Ix::OilRateTarget]   = getRateLimit(units, M::liquid_surface_rate, pc.oil_rate);
            sWell[Ix::WatRateTarget]   = getRateLimit(units, M::liquid_surface_rate, pc.water_rate);
            sWell[Ix::GasRateTarget]   = getRateLimit(units, M::gas_surface_rate,    pc.gas_rate);
            sWell[Ix::LiqRateTarget]   = getRateLimit(units, M::liquid_surface_rate, pc.liquid_rate);
            sWell[Ix::ResVRateTarget]  = getRateLimit(units, M::rate,                pc.resv_rate);
        }

        template <class SWProp, class SWellArray>
        void assignProductionTargets(const Opm::Well&         well,
                                     const Opm::SummaryState& smry,
                                     const Opm::Schedule&     sched,
                                     const std::size_t        sim_step,
                                     SWProp&&                 swprop,
                                     SWellArray&              sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            const auto pc = well.productionControls(smry);
            const auto predMode = well.predictionMode();

            assignOWGRateTargetsProd(pc, predMode, std::forward<SWProp>(swprop), sWell);
            assignLiqRateTargetsProd(pc, predMode, std::forward<SWProp>(swprop), sWell);
            assignResVRateTargetsProd(well.name(), smry, pc, predMode,
                                      std::forward<SWProp>(swprop), sWell);

            sWell[Ix::THPTarget] = (pc.thp_limit != 0.0)
                ? swprop(M::pressure, pc.thp_limit)
                : 0.0;

            sWell[Ix::BHPTarget] = (pc.bhp_limit != 0.0)
                ? swprop(M::pressure, pc.bhp_limit)
                : swprop(M::pressure, 1.0*::Opm::unit::atm);

            sWell[Ix::HistBHPTarget] = sWell[Ix::BHPTarget];

            if (pc.alq_value != 0.0) {
                const auto alqType = sched[sim_step].vfpprod(pc.vfp_table_number).getALQType();
                assignALQProd(alqType, pc.alq_value, std::forward<SWProp>(swprop), sWell);
            }

            if (predMode) {
                assignPredictionTargetsProd(sched.getUnits(), pc, sWell);
            }
        }

        template <class SWProp, class SWellArray>
        void assignInjectionTargets(const Opm::Well&         well,
                                    const Opm::SummaryState& smry,
                                    SWProp&&                 swprop,
                                    SWellArray&              sWell)
        {
            using Ix = VI::SWell::index;
            using M  = ::Opm::UnitSystem::measure;
            using IP = ::Opm::Well::InjectorCMode;
            using IT = ::Opm::InjectorType;

            const auto& ic = well.injectionControls(smry);

            if (ic.hasControl(IP::RATE)) {
                switch (ic.injector_type) {
                case IT::OIL:
                    sWell[Ix::OilRateTarget] = swprop(M::liquid_surface_rate, ic.surface_rate);
                    break;

                case IT::WATER:
                    sWell[Ix::WatRateTarget]     = swprop(M::liquid_surface_rate, ic.surface_rate);
                    sWell[Ix::HistLiqRateTarget] = sWell[Ix::WatRateTarget];
                    break;

                case IT::GAS:
                    sWell[Ix::GasRateTarget]     = swprop(M::gas_surface_rate, ic.surface_rate);
                    sWell[Ix::HistGasRateTarget] = sWell[Ix::GasRateTarget];
                    break;

                default:
                    break;
                }
            }

            if (ic.hasControl(IP::RESV)) {
                sWell[Ix::ResVRateTarget] = swprop(M::rate, ic.reservoir_rate);
            }

            if (ic.hasControl(IP::THP)) {
                sWell[Ix::THPTarget] = swprop(M::pressure, ic.thp_limit);
            }

            sWell[Ix::BHPTarget] = ic.hasControl(IP::BHP)
                ? swprop(M::pressure, ic.bhp_limit)
                : swprop(M::pressure, 1.0E05*::Opm::unit::psia);

            sWell[Ix::HistBHPTarget] = sWell[Ix::BHPTarget];
        }

        template <class SWProp, class SWellArray>
        void assignGasLiftOptimisation(const Opm::GasLiftOpt::Well& w_glo,
                                       SWProp&&                     swprop,
                                       SWellArray&                  sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            sWell[Ix::LOmaxRate] = swprop(M::gas_surface_rate, w_glo.max_rate().value_or(0.0));
            sWell[Ix::LOminRate] = swprop(M::gas_surface_rate, w_glo.min_rate());

            sWell[Ix::LOweightFac] = static_cast<float>(w_glo.weight_factor());
            sWell[Ix::LOincFac] = static_cast<float>(w_glo.inc_weight_factor());
        }

        template <class SWProp, class SWellArray>
        void assignReferenceDepth(const Opm::Well& well,
                                  SWProp&&         swprop,
                                  SWellArray&      sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            if (const auto depth = datumDepth(well); depth.has_value()) {
                sWell[Ix::DatumDepth] = swprop(M::length, depth.value());
            }
            else {
                // BHP reference depth missing for this well.  Typically
                // caused by defaulted WELSPECS(5) and no active reservoir
                // connections from which to infer the depth.  Output
                // sentinel value.
                //
                // Note: All unit systems get the *same* sentinel value so
                // we intentionally omit unit conversion here.
                sWell[Ix::DatumDepth] = -1.0e+20f;
            }
        }

        template <class SWellArray>
        void assignWGrupCon(const Opm::Well& well,
                            SWellArray&      sWell)
        {
            using Ix = VI::SWell::index;

            if (const auto gr = well.getGuideRate(); gr > 0.0) {
                sWell[Ix::WGrupConGuideRate] = static_cast<float>(gr);
            }

            sWell[Ix::WGrupConGRScaling] =
                static_cast<float>(well.getGuideRateScalingFactor());
        }

        template <class SWProp, class SWellArray>
        void assignEconomicLimits(const Opm::Well& well,
                                  SWProp&&         swprop,
                                  SWellArray&      sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            const auto& limits = well.getEconLimits();

            if (limits.onMinOilRate()) {
                sWell[Ix::EconLimitMinOil] = swprop(M::liquid_surface_rate, limits.minOilRate());
            }

            if (limits.onMinGasRate()) {
                sWell[Ix::EconLimitMinGas] = swprop(M::gas_surface_rate, limits.minGasRate());
            }

            if (limits.onMaxWaterCut()) {
                sWell[Ix::EconLimitMaxWct] = swprop(M::identity, limits.maxWaterCut());
            }

            if (limits.onMaxGasOilRatio()) {
                sWell[Ix::EconLimitMaxGor] = swprop(M::gas_oil_ratio, limits.maxGasOilRatio());
            }

            if (limits.onMaxWaterGasRatio()) {
                sWell[Ix::EconLimitMaxWgr] = swprop(M::oil_gas_ratio, limits.maxWaterGasRatio());
            }

            if (limits.onSecondaryMaxWaterCut()) {
                sWell[Ix::EconLimitMaxWct_2] = swprop(M::identity, limits.maxSecondaryMaxWaterCut());
            }

            if (limits.onMinLiquidRate()) {
                sWell[Ix::EconLimitMinLiq] = swprop(M::liquid_surface_rate, limits.minLiquidRate());
            }
        }

        template <class SWProp, class SWellArray>
        void assignWellTest(const std::string&        well,
                            const Opm::Schedule&      sched,
                            const Opm::WellTestState& wtest_state,
                            const std::size_t         sim_step,
                            SWProp&&                  swprop,
                            SWellArray&               sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            const auto& wtest_config = sched[sim_step].wtest_config();
            const auto& wtest_param = wtest_state.restart_well(wtest_config, well);

            if (! wtest_param.has_value()) {
                return;
            }

            sWell[Ix::WTestInterval] = swprop(M::time, wtest_param->test_interval);
            sWell[Ix::WTestStartupTime] = swprop(M::time, wtest_param->startup_time);
        }

        template <class SWellArray>
        void assignEfficiencyFactors(const Opm::Well& well,
                                     SWellArray&      sWell)
        {
            using Ix = VI::SWell::index;

            sWell[Ix::EfficiencyFactor1] = well.getEfficiencyFactor();
            sWell[Ix::EfficiencyFactor2] = sWell[Ix::EfficiencyFactor1];
        }

        template <class SWellArray>
        void assignTracerData(const Opm::TracerConfig& tracers,
                              const Opm::SummaryState& smry,
                              const std::string&       wname,
                              SWellArray&              sWell)
        {
            auto output_index = static_cast<std::size_t>(VI::SWell::index::TracerOffset);

            for (const auto& tracer : tracers) {
                sWell[output_index++] =
                    smry.get_well_var(wname, fmt::format("WTIC{}", tracer.name), 0.0);
            }
        }

        template <class SWellArray>
        void staticContrib(const Opm::Well&           well,
                           const Opm::GasLiftOpt&     glo,
                           const std::size_t          sim_step,
                           const Opm::Schedule&       sched,
                           const Opm::TracerConfig&   tracers,
                           const Opm::WellTestState&  wtest_state,
                           const ::Opm::SummaryState& smry,
                           SWellArray&                sWell)
        {
            using Ix = VI::SWell::index;
            using M = ::Opm::UnitSystem::measure;

            assignDefaultSWell(sWell);

            const auto& units = sched.getUnits();
            auto swprop = [&units](const M u, const double x) -> float
            {
                return static_cast<float>(units.from_si(u, x));
            };

            if (well.isProducer()) {
                assignProductionTargets(well, smry, sched, sim_step, swprop, sWell);
            }
            else if (well.isInjector()) {
                assignInjectionTargets(well, smry, swprop, sWell);
            }

            if (glo.has_well(well.name())) {
                assignGasLiftOptimisation(glo.well(well.name()), swprop, sWell);
            }

            assignReferenceDepth(well, swprop, sWell);

            sWell[Ix::DrainageRadius] = swprop(M::length, well.getDrainageRadius());

            assignWGrupCon(well, sWell);
            assignEfficiencyFactors(well, sWell);
            assignEconomicLimits(well, swprop, sWell);
            assignWellTest(well.name(), sched, wtest_state, sim_step, swprop, sWell);
            assignTracerData(tracers, smry, well.name(), sWell);
        }
    } // SWell

    namespace XWell {
        std::size_t entriesPerWell(const std::vector<int>& inteHead)
        {
            assert ((inteHead[VI::intehead::NXWELZ] > 123) &&
                    "XWEL must allocate at least 124 elements per well");

            return inteHead[VI::intehead::NXWELZ];
        }

        Opm::RestartIO::Helpers::WindowedArray<double>
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<double>;

            return WV {
                WV::NumWindows{ numWells(inteHead) },
                WV::WindowSize{ entriesPerWell(inteHead) }
            };
        }

        template <class XWellArray>
        void staticContrib(const ::Opm::Well&    well,
                           const Opm::SummaryState& st,
                           const Opm::UnitSystem& units,
                           XWellArray&            xWell)
        {
            using M  = ::Opm::UnitSystem::measure;
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

            const auto bhpTarget = well.isInjector()
                ? well.injectionControls(st).bhp_limit
                : well.productionControls(st).bhp_limit;

            xWell[Ix::BHPTarget] = units.from_si(M::pressure, bhpTarget);
        }

        template <class XWellArray>
        void assignCumulatives(const std::string&         well,
                               const ::Opm::SummaryState& smry,
                               XWellArray&                xWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

            auto get = [&smry, &well](const std::string& vector)
            {
                return smry.get_well_var(well, vector, 0.0);
            };

            // Since a well can change between producer and injector in the
            // lifetime of the field we output both production and injection
            // cumulatives.

            xWell[Ix::OilPrTotal]  = get("WOPT");
            xWell[Ix::WatPrTotal]  = get("WWPT");
            xWell[Ix::GasPrTotal]  = get("WGPT");
            xWell[Ix::VoidPrTotal] = get("WVPT");

            xWell[Ix::OilPrTotalSolution] = get("WOPTS");
            xWell[Ix::GasPrTotalSolution] = get("WGPTS");

            xWell[Ix::HistOilPrTotal] = get("WOPTH");
            xWell[Ix::HistWatPrTotal] = get("WWPTH");
            xWell[Ix::HistGasPrTotal] = get("WGPTH");

            xWell[Ix::WatInjTotal]     = get("WWIT");
            xWell[Ix::GasInjTotal]     = get("WGIT");
            xWell[Ix::VoidInjTotal]    = get("WVIT");
            xWell[Ix::HistWatInjTotal] = get("WWITH");
            xWell[Ix::HistGasInjTotal] = get("WGITH");
        }

        template <class XWellArray>
        void assignProducer(const std::string&         well,
                            const ::Opm::SummaryState& smry,
                            XWellArray&                xWell)
        {
            using Ix = VI::XWell::index;

            auto get = [&smry, &well](const std::string& vector)
            {
                return smry.get_well_var(well, vector, 0);
            };

            xWell[Ix::OilPrRate] = get("WOPR");
            xWell[Ix::WatPrRate] = get("WWPR");
            xWell[Ix::GasPrRate] = get("WGPR");

            xWell[Ix::LiqPrRate] = xWell[Ix::OilPrRate]
                                 + xWell[Ix::WatPrRate];

            xWell[Ix::VoidPrRate] = get("WVPR");
            xWell[Ix::TubHeadPr] = get("WTHP");
            xWell[Ix::FlowBHP] = get("WBHP");
            xWell[Ix::WatCut]  = get("WWCT");
            xWell[Ix::GORatio] = get("WGOR");

            // Not fully characterised.
            xWell[Ix::item36] = xWell[Ix::OilPrRate];
            xWell[Ix::item37] = xWell[Ix::WatPrRate];
            xWell[Ix::item38] = xWell[Ix::GasPrRate];

            xWell[Ix::PrimGuideRate]   = xWell[Ix::PrimGuideRate_2]   = get("WOPGR");
            xWell[Ix::WatPrGuideRate]  = xWell[Ix::WatPrGuideRate_2]  = get("WWPGR");
            xWell[Ix::GasPrGuideRate]  = xWell[Ix::GasPrGuideRate_2]  = get("WGPGR");
            xWell[Ix::VoidPrGuideRate] = xWell[Ix::VoidPrGuideRate_2] = get("WVPGR");

        }

        template <class GetSummaryVector, class XWellArray>
        void assignCommonInjector(GetSummaryVector& get,
                                  XWellArray&       xWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

            // Injection rates reported as negative.
            xWell[Ix::OilPrRate] = -get("WOIR");
            xWell[Ix::WatPrRate] = -get("WWIR");
            xWell[Ix::GasPrRate] = -get("WGIR");

            // Not fully characterised.
            xWell[Ix::item36] = xWell[Ix::OilPrRate];
            xWell[Ix::item37] = xWell[Ix::WatPrRate];
            xWell[Ix::item38] = xWell[Ix::GasPrRate];

            xWell[Ix::TubHeadPr] = get("WTHP");
            xWell[Ix::FlowBHP] = get("WBHP");
        }

        template <class XWellArray>
        void assignWaterInjector(const std::string&         well,
                                 const ::Opm::SummaryState& smry,
                                 XWellArray&                xWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

            auto get = [&smry, &well](const std::string& vector)
            {
                return smry.get_well_var(well, vector, 0.0);
            };

            assignCommonInjector(get, xWell);

            xWell[Ix::LiqPrRate] = xWell[Ix::WatPrRate];

            xWell[Ix::PrimGuideRate] = xWell[Ix::PrimGuideRate_2] = -get("WWIGR");
            xWell[Ix::WatVoidPrRate] = -get("WWVIR");
        }

        template <class XWellArray>
        void assignGasInjector(const std::string&         well,
                               const ::Opm::SummaryState& smry,
                               XWellArray&                xWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;

            auto get = [&smry, &well](const std::string& vector)
            {
                return smry.get_well_var(well, vector, 0.0);
            };

            assignCommonInjector(get, xWell);

            // Injection rates reported as negative production rates.
            xWell[Ix::VoidPrRate] = -get("WGVIR");

            xWell[Ix::GasFVF] = (std::abs(xWell[Ix::GasPrRate]) > 0.0)
                ? xWell[Ix::VoidPrRate] / xWell[Ix::GasPrRate]
                : 0.0;

            if (std::isnan(xWell[Ix::GasFVF])) {
                xWell[Ix::GasFVF] = 0.0;
            }

            xWell[Ix::PrimGuideRate] = xWell[Ix::PrimGuideRate_2] = -get("WGIGR");
            xWell[Ix::GasVoidPrRate] = xWell[Ix::VoidPrRate];
        }

        template <class XWellArray>
        void assignOilInjector(const std::string&         well,
                               const ::Opm::SummaryState& smry,
                               XWellArray&                xWell)
        {
            using Ix = VI::XWell::index;

            auto get = [&smry, &well](const std::string& vector)
            {
                return smry.get_well_var(well, vector, 0.0);
            };

            assignCommonInjector(get, xWell);

            // Injection rates reported as negative production rates.
            xWell[Ix::VoidPrRate] = -get("WOVIR");
            xWell[Ix::PrimGuideRate] = xWell[Ix::PrimGuideRate_2] = -get("WOIGR");
        }

        template <class XWellArray>
        void assignTracerData(const Opm::TracerConfig& tracers,
                              const Opm::Tracers& tracer_dims,
                              const Opm::SummaryState& smry,
                              const Opm::Well& well,
                              XWellArray& xWell)
        {
            if (tracers.empty())
                return;

            using Ix = ::Opm::RestartIO::Helpers::VectorItems::XWell::index;
            std::fill(xWell.begin() + Ix::TracerOffset, xWell.end(), 0);


            for (std::size_t tracer_index=0; tracer_index < tracers.size(); tracer_index++) {
                const auto& tracer = tracers[tracer_index];
                std::size_t output_index = Ix::TracerOffset + tracer_index;
                if (well.isInjector()) {
                    const auto& wtir = smry.get_well_var(well.name(), fmt::format("WTIR{}", tracer.name), 0);
                    xWell[output_index] = -wtir;
                } else {
                    const auto& wtpr = smry.get_well_var(well.name(), fmt::format("WTPR{}", tracer.name), 0);
                    xWell[output_index] = wtpr;
                }
                output_index++;
            }


            for (std::size_t tracer_index=0; tracer_index < tracers.size(); tracer_index++) {
                const auto& tracer = tracers[tracer_index];
                std::size_t output_index = Ix::TracerOffset + tracer_dims.water_tracers() + tracer_index;
                if (well.isProducer()) {
                    const auto& wtpr = smry.get_well_var(well.name(), fmt::format("WTPT{}", tracer.name), 0);
                    xWell[output_index] = wtpr;
                }
                output_index++;
            }

            for (std::size_t tracer_index=0; tracer_index < tracers.size(); tracer_index++) {
                const auto& tracer = tracers[tracer_index];
                std::size_t output_index = Ix::TracerOffset + 2*tracer_dims.water_tracers() + tracer_index;
                if (well.isInjector()) {
                    const auto& wtir = smry.get_well_var(well.name(), fmt::format("WTIT{}", tracer.name), 0);
                    xWell[output_index] = wtir;
                }
            }

            for (std::size_t n=0; n < 2; n++) {
                for (std::size_t tracer_index=0; tracer_index < tracers.size(); tracer_index++) {
                    const auto& tracer = tracers[tracer_index];
                    std::size_t output_index = Ix::TracerOffset + (3 + n)*tracer_dims.water_tracers() + tracer_index;
                    const auto& wtic = smry.get_well_var(well.name(), fmt::format("WTIC{}", tracer.name), 0);
                    const auto& wtpc = smry.get_well_var(well.name(), fmt::format("WTPC{}", tracer.name), 0);

                    if (std::abs(wtic) > 0)
                        xWell[output_index] = wtic;
                    else
                        xWell[output_index] = wtpc;
                }
            }

            std::size_t output_index = Ix::TracerOffset + 5*tracer_dims.water_tracers();
            xWell[output_index] = 0;
            xWell[output_index + 1] = 0;
        }

        template <class XWellArray>
        void dynamicContrib(const ::Opm::Well&         well,
                            const Opm::TracerConfig&   tracers,
                            const Opm::Tracers&        tracer_dims,
                            const ::Opm::SummaryState& smry,
                            XWellArray&                xWell)
        {
            if (well.isProducer()) {
                assignProducer(well.name(), smry, xWell);
            }
            else if (well.isInjector()) {
                using IType = ::Opm::InjectorType;
                const auto itype = well.injectionControls(smry).injector_type;

                switch (itype) {
                case IType::OIL:
                    assignOilInjector(well.name(), smry, xWell);
                    break;

                case IType::WATER:
                    assignWaterInjector(well.name(), smry, xWell);
                    break;

                case IType::GAS:
                    assignGasInjector(well.name(), smry, xWell);
                    break;

                case IType::MULTI:
                    assignWaterInjector(well.name(), smry, xWell);
                    assignGasInjector  (well.name(), smry, xWell);
                    break;
                }
            }
            assignCumulatives(well.name(), smry, xWell);
            assignTracerData(tracers, tracer_dims, smry, well, xWell);
        }
    } // XWell

    namespace ZWell {
        std::size_t entriesPerWell(const std::vector<int>& inteHead)
        {
            assert ((inteHead[VI::intehead::NZWELZ] > 1) &&
                    "ZWEL must allocate at least 1 element per well");

            return inteHead[VI::intehead::NZWELZ];
        }

        Opm::RestartIO::Helpers::WindowedArray<
            Opm::EclIO::PaddedOutputString<8>
        >
        allocate(const std::vector<int>& inteHead)
        {
            using WV = Opm::RestartIO::Helpers::WindowedArray<
                Opm::EclIO::PaddedOutputString<8>
            >;

            return WV {
                WV::NumWindows{ numWells(inteHead) },
                WV::WindowSize{ entriesPerWell(inteHead) }
            };
        }

        template <class ZWellArray>
        void staticContrib(const Opm::Well& well, const Opm::Action::Actions& actions, const Opm::Action::State& action_state, ZWellArray& zWell)
        {
            using Ix = ::Opm::RestartIO::Helpers::VectorItems::ZWell::index;
            zWell[Ix::WellName] = well.name();
            //loop over actions to assign action name for relevant wells
            for (const auto& action : actions) {
                const auto& result = action_state.result(action.name());
                if (result.has_value()) {
                    if (result->has_well(well.name()))
                        zWell[Ix::ActionX] = action.name();
                }
            }
        }

    } // ZWell
} // Anonymous

// =====================================================================

Opm::RestartIO::Helpers::AggregateWellData::
AggregateWellData(const std::vector<int>& inteHead)
    : iWell_ (IWell::allocate(inteHead))
    , sWell_ (SWell::allocate(inteHead))
    , xWell_ (XWell::allocate(inteHead))
    , zWell_ (ZWell::allocate(inteHead))
    , nWGMax_(maxNumGroups(inteHead))
{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateWellData::
captureDeclaredWellData(const Schedule&             sched,
                        const TracerConfig&         tracers,
                        const std::size_t           sim_step,
                        const ::Opm::Action::State& action_state,
                        const Opm::WellTestState&   wtest_state,
                        const ::Opm::SummaryState&  smry,
                        const std::vector<int>&     inteHead)
{
    const auto& wells = sched.wellNames(sim_step);
    const auto& step_glo = sched.glo(sim_step);

    // Static contributions to IWEL array.
    {
        //const auto grpNames = groupNames(sched.getGroups());
        const auto groupMapNameIndex = IWell::currentGroupMapNameIndex(sched, sim_step, inteHead);
        auto msWellID = std::size_t{0};

        wellLoop(wells, sched, sim_step, [&groupMapNameIndex, &msWellID, &step_glo, &wtest_state, &smry, &sched, &sim_step, this]
            (const Well& well, const std::size_t wellID) -> void
        {
            msWellID += well.isMultiSegment();  // 1-based index.
            auto iw   = this->iWell_[wellID];
            const auto& wtest_config = sched[sim_step].wtest_config();

            IWell::staticContrib(well, step_glo, wtest_config, wtest_state, smry, msWellID, groupMapNameIndex, iw);
        });
    }

    // Static contributions to SWEL array.
    wellLoop(wells, sched, sim_step, [&step_glo, &sim_step, &sched, &tracers, &wtest_state, &smry, this]
        (const Well& well, const std::size_t wellID) -> void
    {
        auto sw = this->sWell_[wellID];

        SWell::staticContrib(well, step_glo, sim_step, sched, tracers, wtest_state, smry, sw);
    });

    // Static contributions to XWEL array.
    wellLoop(wells, sched, sim_step, [&sched, &smry, this]
        (const Well& well, const std::size_t wellID) -> void
    {
        auto xw = this->xWell_[wellID];

        XWell::staticContrib(well, smry,sched.getUnits(), xw);
    });

    {
        // Static contributions to ZWEL array.
        wellLoop(wells, sched, sim_step, [&sim_step, &action_state, &sched, this]
            (const Well& well, const std::size_t wellID) -> void
        {
            auto zw = this->zWell_[wellID];
            ZWell::staticContrib(well, sched[sim_step].actions(), action_state, zw);
        });
    }
}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateWellData::
captureDynamicWellData(const Opm::Schedule&        sched,
                       const TracerConfig&         tracers,
                       const std::size_t           sim_step,
                       const Opm::data::Wells&     xw,
                       const ::Opm::SummaryState&  smry)
{
    const auto& wells = sched.wellNames(sim_step);

    // Dynamic contributions to IWEL array.
    wellLoop(wells, sched, sim_step, [this, &xw]
        (const Well& well, const std::size_t wellID) -> void
    {
        auto iWell = this->iWell_[wellID];

        auto i = xw.find(well.name());
        if ((i == std::end(xw)) || (i->second.dynamicStatus == Opm::Well::Status::SHUT)) {
            IWell::dynamicContribShut(iWell);
        }
        else if (i->second.dynamicStatus == Opm::Well::Status::STOP) {
            IWell::dynamicContribStop(i->second, iWell);
        }
        else {
            IWell::dynamicContribOpen(well, i->second, iWell);
        }
    });

    // Dynamic contributions to XWEL array.
    wellLoop(wells, sched, sim_step, [this, &sched, &tracers, &smry]
        (const Well& well, const std::size_t wellID) -> void
    {
        auto xwell = this->xWell_[wellID];

        XWell::dynamicContrib(well, tracers, sched.runspec().tracers(), smry, xwell);
    });
}
