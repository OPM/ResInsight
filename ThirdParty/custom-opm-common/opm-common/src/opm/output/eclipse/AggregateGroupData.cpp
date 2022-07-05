/*
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

#include <opm/output/eclipse/AggregateGroupData.hpp>

#include <opm/output/eclipse/WriteRestartHelpers.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <map>
#include <optional>
#include <string>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/format.h>

#define ENABLE_GCNTL_DEBUG_OUTPUT 0

#if ENABLE_GCNTL_DEBUG_OUTPUT
#include <iostream>
#endif // ENABLE_GCNTL_DEBUG_OUTPUT

// #####################################################################
// Class Opm::RestartIO::Helpers::AggregateGroupData
// ---------------------------------------------------------------------


namespace {

// maximum number of groups
std::size_t ngmaxz(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NGMAXZ];
}

// maximum number of wells in any group
int nwgmax(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NWGMAX];
}
 namespace value = ::Opm::RestartIO::Helpers::VectorItems::IGroup::Value;
 value::GuideRateMode GuideRateModeFromGuideRateProdTarget(Opm::Group::GuideRateProdTarget grpt) {
    switch (grpt) {
        case Opm::Group::GuideRateProdTarget::OIL:
            return value::GuideRateMode::Oil;
        case Opm::Group::GuideRateProdTarget::WAT:
            return value::GuideRateMode::Water;
        case Opm::Group::GuideRateProdTarget::GAS:
            return value::GuideRateMode::Gas;
        case Opm::Group::GuideRateProdTarget::LIQ:
            return value::GuideRateMode::Liquid;
        case Opm::Group::GuideRateProdTarget::RES:
            return value::GuideRateMode::Resv;
        case Opm::Group::GuideRateProdTarget::COMB:
            return value::GuideRateMode::Comb;
        case Opm::Group::GuideRateProdTarget::WGA:
            return value::GuideRateMode::None;
        case Opm::Group::GuideRateProdTarget::CVAL:
            return value::GuideRateMode::None;
        case Opm::Group::GuideRateProdTarget::INJV:
            return value::GuideRateMode::None;
        case Opm::Group::GuideRateProdTarget::POTN:
            return value::GuideRateMode::Potn;
        case Opm::Group::GuideRateProdTarget::FORM:
            return value::GuideRateMode::Form;
        case Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE:
            return value::GuideRateMode::None;

        default:
            throw std::logic_error(fmt::format("Not recognized value: {} for GuideRateProdTarget",
                                               static_cast<int>(grpt)));
    }
}


template <typename GroupOp>
void groupLoop(const std::vector<const Opm::Group*>& groups,
               GroupOp&&                             groupOp)
{
    auto groupID = std::size_t {0};
    for (const auto* group : groups) {
        groupID += 1;

        if (group == nullptr) {
            continue;
        }

        groupOp(*group, groupID - 1);
    }
}

template <typename T>
std::optional<int> findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

    return (it != vecOfElements.end()) ? std::optional<int>{std::distance(vecOfElements.begin(), it)} : std::nullopt;
}

int currentGroupLevel(const Opm::Schedule& sched, const Opm::Group& group, const size_t simStep)
{
    auto current = group;
    int level = 0;
    while (current.name() != "FIELD") {
        level += 1;
        current = sched.getGroup(current.parent(), simStep);
    }

    return level;
}

void groupCurrentlyProductionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const size_t simStep, bool& controllable)
{
    using wellCtrlMode   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;
    if (controllable)
        return;

    for (const auto& group_name : group.groups()) {
        const auto& sub_group = sched.getGroup(group_name, simStep);
        auto cur_prod_ctrl = (sub_group.name() == "FIELD") ? static_cast<int>(sumState.get("FMCTP", -1)) :
                             static_cast<int>(sumState.get_group_var(sub_group.name(), "GMCTP", -1));
        if (cur_prod_ctrl <= 0) {
            //come here if group is controlled by higher level
            groupCurrentlyProductionControllable(sched, sumState, sched.getGroup(group_name, simStep), simStep, controllable);
        }
    }

    for (const auto& well_name : group.wells()) {
        const auto& well = sched.getWell(well_name, simStep);
        if (well.isProducer()) {
            int cur_prod_ctrl = 0;
            // Find control mode for well
            const std::string sum_key = "WMCTL";
            if (sumState.has_well_var(well_name, sum_key)) {
                cur_prod_ctrl = static_cast<int>(sumState.get_well_var(well_name, sum_key));
            }
            if (cur_prod_ctrl == wellCtrlMode::Group) {
                controllable = true;
                return;
            }
        }
    }
}


bool groupCurrentlyProductionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const size_t simStep) {
    bool controllable = false;
    groupCurrentlyProductionControllable(sched, sumState, group, simStep, controllable);
    return controllable;
}


void groupCurrentlyInjectionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const Opm::Phase& iPhase, const size_t simStep, bool& controllable)
{
    using wellCtrlMode   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;
    if (controllable)
        return;

    for (const auto& group_name : group.groups()) {
        const auto& sub_group = sched.getGroup(group_name, simStep);
        int cur_inj_ctrl = 0;
        if (iPhase == Opm::Phase::WATER) {
            cur_inj_ctrl = (sub_group.name() == "FIELD") ? static_cast<int>(sumState.get("FMCTW", -1)) :
                           static_cast<int>(sumState.get_group_var(sub_group.name(), "GMCTW", -1));
        } else if (iPhase == Opm::Phase::GAS) {
            cur_inj_ctrl = (sub_group.name() == "FIELD") ? static_cast<int>(sumState.get("FMCTG", -1)) :
                           static_cast<int>(sumState.get_group_var(sub_group.name(), "GMCTG", -1));
        }
        if (cur_inj_ctrl <= 0) {
            //come here if group is controlled by higher level
            groupCurrentlyInjectionControllable(sched, sumState, sched.getGroup(group_name, simStep), iPhase, simStep, controllable);
        }
    }

    for (const auto& well_name : group.wells()) {
        const auto& well = sched.getWell(well_name, simStep);
        if (well.isInjector() && iPhase == well.wellType().injection_phase()) {
            int cur_inj_ctrl = 0;
            // Find control mode for well
            const std::string sum_key = "WMCTL";
            if (sumState.has_well_var(well_name, sum_key)) {
                cur_inj_ctrl = static_cast<int>(sumState.get_well_var(well_name, sum_key));
            }

            if (cur_inj_ctrl == wellCtrlMode::Group) {
                controllable = true;
                return;
            }
        }
    }
}

bool groupCurrentlyInjectionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const Opm::Phase& iPhase, const size_t simStep) {
    bool controllable = false;
    groupCurrentlyInjectionControllable(sched, sumState, group, iPhase, simStep, controllable);
    return controllable;
}


/*
  Searches upwards in the group tree for the first parent group with active
  control different from NONE and FLD. The function will return an empty
  optional if no such group can be found.
*/

std::optional<Opm::Group> controlGroup(const Opm::Schedule& sched,
                                       const Opm::SummaryState& sumState,
                                       const Opm::Group& group,
                                       const std::size_t simStep) {
    auto current = group;
    bool isField = false;
    double cur_prod_ctrl= 0.;
    while (!isField) {
        if (current.name() != "FIELD") {
            cur_prod_ctrl = sumState.get_group_var(current.name(), "GMCTP", 0);
        } else {
            cur_prod_ctrl = sumState.get("FMCTP", 0);
        }
        if (cur_prod_ctrl > 0) {
            return current;
        }
        if (current.name() != "FIELD") {
            current = sched.getGroup(current.parent(), simStep);
        }
        else {
            isField = true;
        }
    }
    return {};
}


std::optional<Opm::Group>  injectionControlGroup(const Opm::Schedule& sched,
        const Opm::SummaryState& sumState,
        const Opm::Group& group,
        const std::string curGroupInjCtrlKey,
        const std::string curFieldInjCtrlKey,
        const size_t simStep)
//
// returns group of higher (highest) level group with active control different from (NONE or FLD)
//
{
    auto current = group;
    bool isField = false;
    double  cur_inj_ctrl = 0.;
    while (!isField) {
        if (current.name() != "FIELD") {
            cur_inj_ctrl = sumState.get_group_var(current.name(), curGroupInjCtrlKey, 0.);
        } else {
            cur_inj_ctrl = sumState.get(curFieldInjCtrlKey, 0.);
        }
        if (cur_inj_ctrl > 0) {
            return current;
        }
#if ENABLE_GCNTL_DEBUG_OUTPUT
        else {
            std::cout << "Current injection group control: " << curInjCtrlKey
                      << " is not defined for group: " << current.name() << " at timestep: " << simStep << std::endl;
        }
#endif // ENABLE_GCNTL_DEBUG_OUTPUT
        if (current.name() != "FIELD") {
            current = sched.getGroup(current.parent(), simStep);
        } else {
            isField = true;
        }
    }
    return {};
} // namespace


std::vector<std::size_t> groupParentSeqIndex(const Opm::Schedule& sched,
        const Opm::Group& group,
        const size_t simStep)
//
// returns a vector with the group sequence index of all parent groups from current parent group to Field level
//
{
    std::vector<std::size_t> seq_numbers;
    auto current = group;
    while (current.name() != "FIELD") {
        current = sched.getGroup(current.parent(), simStep);
        seq_numbers.push_back(current.insert_index());
    }
    return seq_numbers;
}

namespace IGrp {
std::size_t entriesPerGroup(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NIGRPZ];
}

Opm::RestartIO::Helpers::WindowedArray<int>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<int>;

    return WV {
        WV::NumWindows{ ngmaxz(inteHead) },
        WV::WindowSize{ entriesPerGroup(inteHead) }
    };
}



template <class IGrpArray>
void gconprodCMode(const Opm::Group& group,
                   const int nwgmax,
                   IGrpArray& iGrp) {
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;

    const auto& prod_cmode = group.prod_cmode();
    iGrp[nwgmax + IGroup::GConProdCMode] = Opm::Group::ProductionCMode2Int(prod_cmode);
}


template <class IGrpArray>
void productionGroup(const Opm::Schedule&     sched,
                     const Opm::Group&        group,
                     const int                nwgmax,
                     const std::size_t        simStep,
                     const Opm::SummaryState& sumState,
                     IGrpArray&               iGrp)
{
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;
    namespace Value = ::Opm::RestartIO::Helpers::VectorItems::IGroup::Value;
    gconprodCMode(group, nwgmax, iGrp);
    const bool is_field = group.name() == "FIELD";
    const auto& production_controls = group.productionControls(sumState);
    const auto& prod_guide_rate_def = production_controls.guide_rate_def;
    Opm::Group::ProductionCMode active_cmode = Opm::Group::ProductionCMode::NONE;
    auto cur_prod_ctrl = (group.name() == "FIELD") ? sumState.get("FMCTP", -1) :
                         sumState.get_group_var(group.name(), "GMCTP", -1);
    if (cur_prod_ctrl >= 0)
        active_cmode = Opm::Group::ProductionCModeFromInt(static_cast<int>(cur_prod_ctrl));

#if ENABLE_GCNTL_DEBUG_OUTPUT
    else {
        // std::stringstream str;
        // str << "Current group production control is not defined for group: " << group.name() << " at timestep: " <<
        // simStep;
        std::cout << "Current group production control is not defined for group: " << group.name()
                  << " at timestep: " << simStep << std::endl;
        // throw std::invalid_argument(str.str());
    }
#endif // ENABLE_GCNTL_DEBUG_OUTPUT

    const auto& cgroup = controlGroup(sched, sumState, group, simStep);
    const auto& deck_cmode = group.prod_cmode();

    if (cgroup && (cgroup->name() != group.name()) && (group.getGroupType() != Opm::Group::GroupType::NONE)) {
        auto cgroup_control = (cgroup->name() == "FIELD") ? static_cast<int>(sumState.get("FMCTP", 0)) : static_cast<int>(sumState.get_group_var(cgroup->name(), "GMCTP", 0));
        iGrp[nwgmax + IGroup::ProdActiveCMode]
            = (prod_guide_rate_def != Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE) ? cgroup_control : 0;
    } else {
        iGrp[nwgmax + IGroup::ProdActiveCMode] = Opm::Group::ProductionCMode2Int(active_cmode);

        // The PRBL and CRAT modes where not handled in a previous explicit if
        // statement; whether that was an oversight or a feature?
        if (active_cmode == Opm::Group::ProductionCMode::PRBL || active_cmode == Opm::Group::ProductionCMode::CRAT)
            iGrp[nwgmax + IGroup::ProdActiveCMode] = 0;
    }
    iGrp[nwgmax + 9] = iGrp[nwgmax + IGroup::ProdActiveCMode];

    iGrp[nwgmax + IGroup::GuideRateDef] = GuideRateModeFromGuideRateProdTarget(prod_guide_rate_def);

    // Set iGrp for [nwgmax + IGroup::ExceedAction]
    /*
    For the reduction option RATE the value is generally = 4

    For the reduction option NONE the values are as shown below, however, this is not a very likely case.

    = 0 for group with  "FLD" or "NONE"
    = 4 for "GRAT" FIELD
    = -40000 for production group with "ORAT"
    = -4000  for production group with "WRAT"
    = -400    for production group with "GRAT"
    = -40     for production group with "LRAT"

    Other reduction options are currently not covered in the code
    */

    const auto& p_exceed_act = production_controls.exceed_action;
    switch (deck_cmode) {
    case Opm::Group::ProductionCMode::NONE:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? 0 : 4;
        break;
    case Opm::Group::ProductionCMode::ORAT:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -40000 : 4;
        break;
    case Opm::Group::ProductionCMode::WRAT:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -4000 : 4;
        break;
    case Opm::Group::ProductionCMode::GRAT:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -400 : 4;
        break;
    case Opm::Group::ProductionCMode::LRAT:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -40 : 4;
        break;
    case Opm::Group::ProductionCMode::RESV:
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -4 : 4; // need to be checked
        break;
    case Opm::Group::ProductionCMode::FLD:
        if (cgroup && (prod_guide_rate_def != Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
            iGrp[nwgmax + IGroup::GuideRateDef] = GuideRateModeFromGuideRateProdTarget(prod_guide_rate_def);
        }
        iGrp[nwgmax + IGroup::ExceedAction] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? 4 : 4;
        break;
    default:
        iGrp[nwgmax + IGroup::ExceedAction] = 0;
    }

    // Start branching for determining iGrp[nwgmax + IGroup::ProdHighLevCtrl]
    // use default value if group is not available for group control

    if (group.getGroupType() == Opm::Group::GroupType::NONE || group.getGroupType() == Opm::Group::GroupType::INJECTION) {
        if (is_field) {
            iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 0;
        } else {
            //set default value for the group's availability for higher level control for injection
            iGrp[nwgmax + IGroup::ProdHighLevCtrl] = (groupCurrentlyProductionControllable(sched, sumState, group, simStep) ) ? 1 : -1;
        }
        return;
    }

    if (group.name() == "FIELD" ) {
        iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 0;
    } else {
        // group is available for higher level control, but is currently constrained by own limits
        iGrp[nwgmax + IGroup::ProdHighLevCtrl] = -1;
        if ((deck_cmode != Opm::Group::ProductionCMode::FLD) && !group.productionGroupControlAvailable()) {
            //group is not free to respond to higher level control)
            iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 0;
        } else if (cgroup && ((active_cmode == Opm::Group::ProductionCMode::FLD) || (active_cmode == Opm::Group::ProductionCMode::NONE))) {
            //a higher level group control is active constraint
            if ((deck_cmode != Opm::Group::ProductionCMode::FLD) && (deck_cmode != Opm::Group::ProductionCMode::NONE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
            } else if ((deck_cmode == Opm::Group::ProductionCMode::FLD) && (prod_guide_rate_def != Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
            } else if ((deck_cmode == Opm::Group::ProductionCMode::NONE) && group.productionGroupControlAvailable() &&
                    (prod_guide_rate_def != Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
                //group is directly under higher level controlGroup
            } else if ((deck_cmode == Opm::Group::ProductionCMode::FLD) && (prod_guide_rate_def == Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 1;
            } else if ((deck_cmode == Opm::Group::ProductionCMode::NONE) && group.productionGroupControlAvailable() &&
                    (prod_guide_rate_def == Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 1;
            }
        } else if (!cgroup && active_cmode == Opm::Group::ProductionCMode::NONE) {
            //group is directly under higher level controlGroup
            if ((deck_cmode == Opm::Group::ProductionCMode::FLD) && (prod_guide_rate_def == Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 1;
            } else if ((deck_cmode == Opm::Group::ProductionCMode::NONE) && group.productionGroupControlAvailable() &&
                    (prod_guide_rate_def == Opm::Group::GuideRateProdTarget::NO_GUIDE_RATE)) {
                iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 1;
            }
        }
    }
}

std::tuple<int, int, int, int> injectionGroup(const Opm::Schedule&     sched,
                                              const Opm::Group&        group,
                                              const int                nwgmax,
                                              const std::size_t        simStep,
                                              const Opm::SummaryState& sumState,
                                              const Opm::Phase         phase)
{
    const bool is_field = group.name() == "FIELD";
    auto group_parent_list = groupParentSeqIndex(sched, group, simStep);
    int high_level_ctrl = 0;
    int current_cmode = 0;
    int gconinje_cmode = 0;
    int guide_rate_def = 0;

    const std::string field_key = (phase == Opm::Phase::WATER) ? "FMCTW" : "FMCTG";
    const std::string group_key = (phase == Opm::Phase::WATER) ? "GMCTW" : "GMCTG";

    // WATER INJECTION GROUP CONTROL
    if (group.hasInjectionControl(phase)) {

        const auto& injection_controls = group.injectionControls(phase, sumState);
        const auto& cur_inj_ctrl = group.name() == "FIELD" ? static_cast<int>(sumState.get(field_key, -1)) : static_cast<int>(sumState.get_group_var(group.name(), group_key, -1));
        Opm::Group::InjectionCMode active_cmode = Opm::Group::InjectionCModeFromInt(cur_inj_ctrl);
        const auto& deck_cmode = (group.hasInjectionControl(phase))
                                    ? injection_controls.cmode : Opm::Group::InjectionCMode::NONE;
        const auto& cgroup = injectionControlGroup(sched, sumState, group, group_key, field_key, simStep);
        const auto& group_control_available = group.injectionGroupControlAvailable(phase);
        const auto& deck_guide_rate_def = injection_controls.guide_rate_def;

        // group is available for higher level control, but is currently constrained by own limits
        high_level_ctrl = -1;
        if ((deck_cmode != Opm::Group::InjectionCMode::FLD) && !group_control_available) {
            //group is not free to respond to higher level control)
            high_level_ctrl = 0;
        }

        if (cgroup) {
            if ((active_cmode == Opm::Group::InjectionCMode::FLD) || (active_cmode == Opm::Group::InjectionCMode::NONE)) {
                //a higher level group control is active constraint
                if ((deck_cmode != Opm::Group::InjectionCMode::FLD) && (deck_cmode != Opm::Group::InjectionCMode::NONE)) {
                    high_level_ctrl = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
                } else {
                    if (deck_guide_rate_def == Opm::Group::GuideRateInjTarget::NO_GUIDE_RATE) {
                        if (deck_cmode == Opm::Group::InjectionCMode::FLD) {
                            high_level_ctrl = 1;
                        } else if ((deck_cmode == Opm::Group::InjectionCMode::NONE) && group_control_available) {
                            high_level_ctrl = 1;
                        }
                    } else {
                        if (deck_cmode == Opm::Group::InjectionCMode::FLD) {
                            high_level_ctrl = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
                        } else if ((deck_cmode == Opm::Group::InjectionCMode::NONE) && group_control_available) {
                            high_level_ctrl = (cgroup->name() == "FIELD") ? nwgmax : static_cast<int>(cgroup->insert_index());
                        }
                    }
                }
            }
        } else {
            if ((active_cmode == Opm::Group::InjectionCMode::NONE) && (deck_guide_rate_def == Opm::Group::GuideRateInjTarget::NO_GUIDE_RATE)) {
                //group is directly under higher level controlGroup
                if (deck_cmode == Opm::Group::InjectionCMode::FLD) {
                    high_level_ctrl = 1;
                } else if ((deck_cmode == Opm::Group::InjectionCMode::NONE) && group_control_available) {
                    high_level_ctrl = 1;
                }
            }
        }

        guide_rate_def = Opm::Group::GuideRateInjTargetToInt(deck_guide_rate_def);
        gconinje_cmode = Opm::Group::InjectionCMode2Int(deck_cmode);
        if (cgroup && (cgroup->name() != group.name()) && (group.getGroupType() != Opm::Group::GroupType::NONE)) {
            auto cgroup_control = (cgroup->name() == "FIELD") ? static_cast<int>(sumState.get(field_key, 0)) : static_cast<int>(sumState.get_group_var(cgroup->name(), group_key, 0));
            current_cmode = (deck_guide_rate_def != Opm::Group::GuideRateInjTarget::NO_GUIDE_RATE) ? cgroup_control : 0;
        } else {
            current_cmode = cur_inj_ctrl;
        }
    }
    else {
        //set default value for the group's availability for higher level control for water injection for groups with no GCONINJE - WATER
        high_level_ctrl = (groupCurrentlyInjectionControllable(sched, sumState, group, phase, simStep) ) ? 1 : -1;
    }

    // special treatment of group "FIELD"
    if (is_field) high_level_ctrl = 0;

    return {high_level_ctrl, current_cmode, gconinje_cmode, guide_rate_def};
}



template <class IGrpArray>
void injectionGroup(const Opm::Schedule&     sched,
                    const Opm::Group&        group,
                    const int                nwgmax,
                    const std::size_t        simStep,
                    const Opm::SummaryState& sumState,
                    IGrpArray&               iGrp)
{
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;
    const bool is_field = group.name() == "FIELD";
    auto group_parent_list = groupParentSeqIndex(sched, group, simStep);
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;


    // set "default value" for production higher level control in case a group is only injection group
    if (group.isInjectionGroup() && !group.isProductionGroup()) {
        iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 1;
    }
    //Special treatment of groups with no GCONINJE data
    if (group.getGroupType() == Opm::Group::GroupType::NONE) {
        if (is_field) {
            iGrp[nwgmax + IGroup::WInjHighLevCtrl] = 0;
            iGrp[nwgmax + IGroup::GInjHighLevCtrl] = 0;
        } else {
            //set default value for the group's availability for higher level control for injection
            iGrp[nwgmax + IGroup::WInjHighLevCtrl] = (groupCurrentlyInjectionControllable(sched, sumState, group, Opm::Phase::WATER, simStep) ) ? 1 : -1;
            iGrp[nwgmax + IGroup::GInjHighLevCtrl] = (groupCurrentlyInjectionControllable(sched, sumState, group, Opm::Phase::GAS, simStep) ) ? 1 : -1;
        }
        return;
    }

    {
        if (group.hasInjectionControl(Opm::Phase::WATER)) {
            auto [high_level_ctrl, active_cmode, gconinje_cmode, guide_rate_def] = injectionGroup(sched, group, nwgmax, simStep, sumState, Opm::Phase::WATER);
            iGrp[nwgmax + IGroup::WInjHighLevCtrl] = high_level_ctrl;
            iGrp[nwgmax + IGroup::WInjActiveCMode] = active_cmode;
            iGrp[nwgmax + IGroup::GConInjeWInjCMode] = gconinje_cmode;
            iGrp[nwgmax + IGroup::GConInjeWaterGuideRateMode] = guide_rate_def;
        }
    }
    {
        auto [high_level_ctrl, active_cmode, gconinje_cmode, guide_rate_def] = injectionGroup(sched, group, nwgmax, simStep, sumState, Opm::Phase::GAS);
        iGrp[nwgmax + IGroup::GInjHighLevCtrl] = high_level_ctrl;
        iGrp[nwgmax + IGroup::GInjActiveCMode] = active_cmode;
        iGrp[nwgmax + IGroup::GConInjeGInjCMode] = gconinje_cmode;
        iGrp[nwgmax + IGroup::GConInjeGasGuideRateMode] = guide_rate_def;
    }
}

template <class IGrpArray>
void storeNodeSequenceNo(const Opm::Schedule& sched,
                    const Opm::Group& group,
                    const int nwgmax,
                    const std::size_t simStep,
                    IGrpArray& iGrp) {

    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;

    const auto& netwrk = sched[simStep].network();
    const auto seq_ind = findInVector(netwrk.node_names(), group.name());

    // The igrp node number is equal to the node sequence number from the BRANPROP keyword
    // for the groups that also are nodes in the external network (BRANPROP, NODEPROP)
    // If not - the node numbr is zero.
    iGrp[nwgmax + IGroup::NodeNumber] = seq_ind ? seq_ind.value()+1 : 0;
}

template <class IGrpArray>
void storeGroupTree(const Opm::Schedule& sched,
                    const Opm::Group& group,
                    const int nwgmax,
                    const int ngmaxz,
                    const std::size_t simStep,
                    IGrpArray& iGrp) {

    namespace Value = ::Opm::RestartIO::Helpers::VectorItems::IGroup::Value;
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;
    const bool is_field = group.name() == "FIELD";

    // Store index of all child wells or child groups.
    if (group.wellgroup()) {
        int igrpCount = 0;
        for (const auto& well_name : group.wells()) {
            const auto& well = sched.getWell(well_name, simStep);
            iGrp[igrpCount] = well.seqIndex() + 1;
            igrpCount += 1;
        }
        iGrp[nwgmax] = group.wells().size();
        iGrp[nwgmax + IGroup::GroupType] = Value::GroupType::WellGroup;
    } else  {
        int igrpCount = 0;
        for (const auto& group_name : group.groups()) {
            const auto& child_group = sched.getGroup(group_name, simStep);
            iGrp[igrpCount] = child_group.insert_index();
            igrpCount += 1;
        }
        iGrp[nwgmax+ IGroup::NoOfChildGroupsWells] = (group.wellgroup()) ? group.wells().size() : group.groups().size();
        iGrp[nwgmax + IGroup::GroupType] = Value::GroupType::TreeGroup;
    }


    // Store index of parent group
    if (is_field)
        iGrp[nwgmax + IGroup::ParentGroup] = 0;
    else {
        const auto& parent_group = sched.getGroup(group.parent(), simStep);
        if (parent_group.name() == "FIELD")
            iGrp[nwgmax + IGroup::ParentGroup] = ngmaxz;
        else
            iGrp[nwgmax + IGroup::ParentGroup] = parent_group.insert_index();
    }

    iGrp[nwgmax + IGroup::GroupLevel] = currentGroupLevel(sched, group, simStep);
}


template <class IGrpArray>
void storeFlowingWells(const Opm::Group&        group,
                       const int                nwgmax,
                       const Opm::SummaryState& sumState,
                       IGrpArray&               iGrp) {
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;
    const bool is_field = group.name() == "FIELD";
    const double g_act_pwells = is_field ? sumState.get("FMWPR", 0) : sumState.get_group_var(group.name(), "GMWPR", 0);
    const double g_act_iwells = is_field ? sumState.get("FMWIN", 0) : sumState.get_group_var(group.name(), "GMWIN", 0);
    iGrp[nwgmax + IGroup::FlowingWells] = static_cast<int>(g_act_pwells) + static_cast<int>(g_act_iwells);
}


template <class IGrpArray>
void staticContrib(const Opm::Schedule&     sched,
                   const Opm::Group&        group,
                   const int                nwgmax,
                   const int                ngmaxz,
                   const std::size_t        simStep,
                   const Opm::SummaryState& sumState,
                   IGrpArray&               iGrp)
{
    using IGroup = ::Opm::RestartIO::Helpers::VectorItems::IGroup::index;
    const bool is_field = group.name() == "FIELD";

    storeGroupTree(sched, group, nwgmax, ngmaxz, simStep, iGrp);

    //node-number for groups in external network (according to sequence in BRANPROP)
    storeNodeSequenceNo(sched, group, nwgmax, simStep, iGrp);

    storeFlowingWells(group, nwgmax, sumState, iGrp);

    // Treat all groups for production controls
    productionGroup(sched, group, nwgmax, simStep, sumState, iGrp);

    // Treat all groups for injection controls
    injectionGroup(sched, group, nwgmax, simStep, sumState, iGrp);

    if (is_field)
    {
        //the maximum number of groups in the model
        iGrp[nwgmax + IGroup::ProdHighLevCtrl] = 0;
        iGrp[nwgmax + IGroup::WInjHighLevCtrl] = 0;
        iGrp[nwgmax + IGroup::GInjHighLevCtrl] = 0;
        iGrp[nwgmax+88] = ngmaxz;
        iGrp[nwgmax+89] = ngmaxz;
        iGrp[nwgmax+95] = ngmaxz;
        iGrp[nwgmax+96] = ngmaxz;
    }
    else
    {
        //parameters connected to oil injection - not implemented in flow yet
        iGrp[nwgmax+11] = 0;
        iGrp[nwgmax+12] = -1;

        // Hack.  Needed by real field cases.
        iGrp[nwgmax + IGroup::WInjHighLevCtrl] = 1;

        //assign values to group number (according to group sequence)
        iGrp[nwgmax+88] = group.insert_index();
        iGrp[nwgmax+89] = group.insert_index();
        iGrp[nwgmax+95] = group.insert_index();
        iGrp[nwgmax+96] = group.insert_index();
    }
}
} // Igrp

namespace SGrp {
std::size_t entriesPerGroup(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NSGRPZ];
}

Opm::RestartIO::Helpers::WindowedArray<float>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<float>;

    return WV {
        WV::NumWindows{ ngmaxz(inteHead) },
        WV::WindowSize{ entriesPerGroup(inteHead) }
    };
}

template <typename SGProp, class SGrpArray>
void assignGroupGasInjectionTargets(const Opm::Group&        group,
                                    const Opm::SummaryState& sumState,
                                    SGProp&&                 sgprop,
                                    SGrpArray&               sGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::SGroup::inj_index;
    using M  = ::Opm::UnitSystem::measure;

    const auto& prop = group.injectionProperties(Opm::Phase::GAS);
    const auto  cntl = group.injectionControls(Opm::Phase::GAS, sumState);

    if (group.has_control(Opm::Phase::GAS, Opm::Group::InjectionCMode::RATE) &&
        (prop.surface_max_rate.is_numeric() || (cntl.surface_max_rate > 0.0)))
    {
        sGrp[Ix::gasSurfRateLimit] = sgprop(M::gas_surface_rate, cntl.surface_max_rate);
        sGrp[Ix::gasSurfRateLimit_2] = sGrp[Ix::gasSurfRateLimit];
    }

    if (group.has_control(Opm::Phase::GAS, Opm::Group::InjectionCMode::RESV) &&
        (prop.resv_max_rate.is_numeric() || (cntl.resv_max_rate > 0.0)))
    {
        sGrp[Ix::gasResRateLimit] = sgprop(M::rate, cntl.resv_max_rate);
        sGrp[Ix::gasResRateLimit_2] = sGrp[Ix::gasResRateLimit];
    }

    if (group.has_control(Opm::Phase::GAS, Opm::Group::InjectionCMode::REIN) &&
        (prop.target_reinj_fraction.is_numeric() || (cntl.target_reinj_fraction > 0.0)))
    {
        sGrp[Ix::gasReinjectionLimit] = cntl.target_reinj_fraction;
        sGrp[Ix::gasReinjectionLimit_2] = sGrp[Ix::gasReinjectionLimit];
    }

    if (group.has_control(Opm::Phase::GAS, Opm::Group::InjectionCMode::VREP) &&
        (prop.target_void_fraction.is_numeric() || (cntl.target_void_fraction > 0.0)))
    {
        sGrp[Ix::gasVoidageLimit] = cntl.target_void_fraction;
        sGrp[Ix::gasVoidageLimit_2] = sGrp[Ix::gasVoidageLimit];
    }

    sGrp[Ix::gasGuideRate] = cntl.guide_rate;
}

template <typename SGProp, class SGrpArray>
void assignGroupWaterInjectionTargets(const Opm::Group&        group,
                                      const Opm::SummaryState& sumState,
                                      SGProp&&                 sgprop,
                                      SGrpArray&               sGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::SGroup::inj_index;
    using M  = ::Opm::UnitSystem::measure;

    const auto& prop = group.injectionProperties(Opm::Phase::WATER);
    const auto  cntl = group.injectionControls(Opm::Phase::WATER, sumState);

    if (group.has_control(Opm::Phase::WATER, Opm::Group::InjectionCMode::RATE) &&
        (prop.surface_max_rate.is_numeric() || (cntl.surface_max_rate > 0.0)))
    {
        sGrp[Ix::waterSurfRateLimit] = sgprop(M::liquid_surface_rate, cntl.surface_max_rate);
        sGrp[Ix::waterSurfRateLimit_2] = sGrp[Ix::waterSurfRateLimit];
    }

    if (group.has_control(Opm::Phase::WATER, Opm::Group::InjectionCMode::RESV) &&
        (prop.resv_max_rate.is_numeric() || (cntl.resv_max_rate > 0.0)))
    {
        sGrp[Ix::waterResRateLimit] = sgprop(M::rate, cntl.resv_max_rate);
        sGrp[Ix::waterResRateLimit_2] = sGrp[Ix::waterResRateLimit];
    }

    if (group.has_control(Opm::Phase::WATER, Opm::Group::InjectionCMode::REIN) &&
        (prop.target_reinj_fraction.is_numeric() || (cntl.target_reinj_fraction > 0.0)))
    {
        sGrp[Ix::waterReinjectionLimit] = cntl.target_reinj_fraction;
        sGrp[Ix::waterReinjectionLimit_2] = sGrp[Ix::waterReinjectionLimit];
    }

    if (group.has_control(Opm::Phase::WATER, Opm::Group::InjectionCMode::VREP) &&
        (prop.target_void_fraction.is_numeric() || (cntl.target_void_fraction > 0.0)))
    {
        sGrp[Ix::waterVoidageLimit] = cntl.target_void_fraction;
        sGrp[Ix::waterVoidageLimit_2] = sGrp[Ix::waterVoidageLimit];
    }

    sGrp[Ix::waterGuideRate] = cntl.guide_rate;
}

template <typename SGProp, class SGrpArray>
void assignGroupOilInjectionTargets(const Opm::Group&        group,
                                    const Opm::SummaryState& sumState,
                                    SGProp&&                 sgprop,
                                    SGrpArray&               sGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::SGroup::inj_index;
    using M  = ::Opm::UnitSystem::measure;

    const auto& prop = group.injectionProperties(Opm::Phase::OIL);
    const auto  cntl = group.injectionControls(Opm::Phase::OIL, sumState);

    if (group.has_control(Opm::Phase::OIL, Opm::Group::InjectionCMode::RATE) &&
        (prop.surface_max_rate.is_numeric() || (cntl.surface_max_rate > 0.0)))
    {
        sGrp[Ix::oilSurfRateLimit] = sgprop(M::liquid_surface_rate, cntl.surface_max_rate);
        sGrp[Ix::oilSurfRateLimit_2] = sGrp[Ix::oilSurfRateLimit];
    }

    if (group.has_control(Opm::Phase::OIL, Opm::Group::InjectionCMode::RESV) &&
        (prop.resv_max_rate.is_numeric() || (cntl.resv_max_rate > 0.0)))
    {
        sGrp[Ix::oilResRateLimit] = sgprop(M::rate, cntl.resv_max_rate);
        sGrp[Ix::oilResRateLimit_2] = sGrp[Ix::oilResRateLimit];
    }

    if (group.has_control(Opm::Phase::OIL, Opm::Group::InjectionCMode::REIN) &&
        (prop.target_reinj_fraction.is_numeric() || (cntl.target_reinj_fraction > 0.0)))
    {
        sGrp[Ix::oilReinjectionLimit] = cntl.target_reinj_fraction;
        sGrp[Ix::oilReinjectionLimit_2] = sGrp[Ix::oilReinjectionLimit];
    }

    if (group.has_control(Opm::Phase::OIL, Opm::Group::InjectionCMode::VREP) &&
        (prop.target_void_fraction.is_numeric() || (cntl.target_void_fraction > 0.0)))
    {
        sGrp[Ix::oilVoidageLimit] = cntl.target_void_fraction;
        sGrp[Ix::oilVoidageLimit_2] = sGrp[Ix::oilVoidageLimit];
    }
}

template <typename SGProp, class SGrpArray>
void assignGroupInjectionTargets(const Opm::Group&        group,
                                 const Opm::SummaryState& sumState,
                                 SGProp&&                 sgprop,
                                 SGrpArray&               sGrp)
{
    if (group.hasInjectionControl(Opm::Phase::GAS)) {
        assignGroupGasInjectionTargets(group, sumState, std::forward<SGProp>(sgprop), sGrp);
    }

    if (group.hasInjectionControl(Opm::Phase::WATER)) {
        assignGroupWaterInjectionTargets(group, sumState, std::forward<SGProp>(sgprop), sGrp);
    }

    if (group.hasInjectionControl(Opm::Phase::OIL)) {
        assignGroupOilInjectionTargets(group, sumState, std::forward<SGProp>(sgprop), sGrp);
    }
}

template <typename SGProp, class SGrpArray>
void assignGroupProductionTargets(const Opm::Group&        group,
                                  const Opm::SummaryState& sumState,
                                  SGProp&&                 sgprop,
                                  SGrpArray&               sGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::SGroup::prod_index;
    using M  = ::Opm::UnitSystem::measure;

    const auto& prop = group.productionProperties();
    const auto  cntl = group.productionControls(sumState);

    if (group.has_control(Opm::Group::ProductionCMode::ORAT) &&
        (prop.oil_target.is_numeric() || (cntl.oil_target > 0.0)))
    {
        sGrp[Ix::OilRateLimit] = sgprop(M::liquid_surface_rate, cntl.oil_target);
        sGrp[Ix::OilRateLimit_2] = sGrp[Ix::OilRateLimit];  // ORAT control
    }

    if (group.has_control(Opm::Group::ProductionCMode::WRAT) &&
        (prop.water_target.is_numeric() || (cntl.water_target > 0.0)))
    {
        sGrp[Ix::WatRateLimit] = sgprop(M::liquid_surface_rate, cntl.water_target);
        sGrp[Ix::WatRateLimit_2] = sGrp[Ix::WatRateLimit];  // WRAT control
    }

    if (group.has_control(Opm::Group::ProductionCMode::GRAT) &&
        (prop.gas_target.is_numeric() || (cntl.gas_target > 0.0)))
    {
        sGrp[Ix::GasRateLimit] = sgprop(M::gas_surface_rate, cntl.gas_target);
        sGrp[Ix::GasRateLimit_2] = sGrp[Ix::GasRateLimit]; // GRAT control
    }

    if (group.has_control(Opm::Group::ProductionCMode::LRAT) &&
        (prop.liquid_target.is_numeric() || (cntl.liquid_target > 0.0)))
    {
        sGrp[Ix::LiqRateLimit] = sgprop(M::liquid_surface_rate, cntl.liquid_target);
        sGrp[Ix::LiqRateLimit_2] = sGrp[Ix::LiqRateLimit];  // LRAT control
    }
}

// Compatibility shim for restart output of gas-lift rates and limits.  The
// values are intentionally discontinuous in small interval close to zero.
template <typename SGProp>
float getGLORate(const SGProp& sgprop, const std::optional<double>& rate)
{
    if (! rate.has_value()) {
        // Defaulted rate limit (e.g., "supply" or "total").
        return ::Opm::RestartIO::Helpers::
            VectorItems::SGroup::Value::NoGLOLimit;
    }

    // Note: These thresholds and values are in output units.
    const auto smallRateThreshold = 1.0e-20f;
    const auto smallRateDefaultValue = 1.0e-6f;

    const auto glo_rate =
        sgprop(Opm::UnitSystem::measure::gas_surface_rate, rate.value());

    if ((glo_rate < 0.0f) || !(glo_rate < smallRateThreshold)) {
        // rate \not\in [0, smallRateThreshold) -> Unchanged
        return glo_rate;
    }

    // rate \in [0, smallRateThreshold) -> smallRateDefaultValue
    return smallRateDefaultValue;
}

template <typename SGProp, class SGrpArray>
void assignGasLiftOptimisation(const Opm::GasLiftOpt::Group& group,
                               const SGProp&                 sgprop,
                               SGrpArray&                    sGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::SGroup::prod_index;

    sGrp[Ix::GLOMaxSupply] = getGLORate(sgprop, group.max_lift_gas());
    sGrp[Ix::GLOMaxRate]   = getGLORate(sgprop, group.max_total_gas());
}

template <class SGrpArray>
void staticContrib(const Opm::Group&        group,
                   const Opm::GasLiftOpt&   glo,
                   const Opm::SummaryState& sumState,
                   const Opm::UnitSystem&   units,
                   SGrpArray&               sGrp)
{
    using Ix  = ::Opm::RestartIO::Helpers::VectorItems::SGroup::index;
    using Isp = ::Opm::RestartIO::Helpers::VectorItems::SGroup::prod_index;
    using M   = ::Opm::UnitSystem::measure;

    const auto dflt   = -1.0e+20f;
    const auto dflt_2 = -2.0e+20f;
    const auto infty  =  1.0e+20f;
    const auto zero   =  0.0f;
    const auto one    =  1.0f;

    const auto init = std::vector<float> { // 112 Items (0..111)
        // 0     1      2      3      4
        infty, infty, dflt , infty , zero ,     //   0..  4  ( 0)
        zero , infty, infty, infty , infty,     //   5..  9  ( 1)
        infty, infty, infty, infty , dflt ,     //  10.. 14  ( 2)
        infty, infty, infty, infty , dflt ,     //  15.. 19  ( 3)
        infty, infty, infty, infty , dflt ,     //  20.. 24  ( 4)
        zero , zero , zero , dflt_2, zero ,     //  24.. 29  ( 5)
        zero , zero , zero , zero  , zero ,     //  30.. 34  ( 6)
        infty, zero , zero , zero  , infty,     //  35.. 39  ( 7)
        zero , zero , zero , zero  , zero ,     //  40.. 44  ( 8)
        zero , zero , zero , zero  , zero ,     //  45.. 49  ( 9)
        zero , infty, infty, infty , infty,     //  50.. 54  (10)
        infty, infty, infty, infty , infty,     //  55.. 59  (11)
        infty, infty, infty, infty , infty,     //  60.. 64  (12)
        infty, infty, infty, infty , zero ,     //  65.. 69  (13)
        zero , zero , zero , zero  , zero ,     //  70.. 74  (14)
        zero , zero , zero , zero  , infty,     //  75.. 79  (15)
        infty, zero , infty, zero  , zero ,     //  80.. 84  (16)
        zero , zero , zero , zero  , zero ,     //  85.. 89  (17)
        zero , zero , one  , zero  , zero ,     //  90.. 94  (18)
        zero , zero , zero , zero  , zero ,     //  95.. 99  (19)
        zero , zero , zero , zero  , zero ,     // 100..104  (20)
        zero , zero , zero , zero  , zero ,     // 105..109  (21)
        zero , zero                             // 110..111  (22)
    };

    const auto sz = static_cast<decltype(init.size())>(sGrp.size());

    auto b = std::begin(init);
    auto e = b + std::min(init.size(), sz);

    std::copy(b, e, std::begin(sGrp));

    auto sgprop = [&units](const M u, const double x) -> float
    {
        return static_cast<float>(units.from_si(u, x));
    };

    sGrp[Ix::EfficiencyFactor] =
        sgprop(M::identity, group.getGroupEfficiencyFactor());

    if (group.isProductionGroup()) {
        assignGroupProductionTargets(group, sumState, sgprop, sGrp);
    }

    if (group.isInjectionGroup()) {
        assignGroupInjectionTargets(group, sumState, sgprop, sGrp);
    }

    if (glo.has_group(group.name())) {
        assignGasLiftOptimisation(glo.group(group.name()), sgprop, sGrp);
    }

    if ((group.name() == "FIELD") && (group.getGroupType() == Opm::Group::GroupType::NONE)) {
        sGrp[Isp::GuideRate] = 0.0;
        sGrp[14] = 0.0;
        sGrp[19] = 0.0;
        sGrp[24] = 0.0;
    }
}
} // SGrp

namespace XGrp {
std::size_t entriesPerGroup(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NXGRPZ];
}

Opm::RestartIO::Helpers::WindowedArray<double>
allocate(const std::vector<int>& inteHead)
{
    using WV = Opm::RestartIO::Helpers::WindowedArray<double>;

    return WV {
        WV::NumWindows{ ngmaxz(inteHead) },
        WV::WindowSize{ entriesPerGroup(inteHead) }
    };
}

// here define the dynamic group quantities to be written to the restart file
template <class XGrpArray>
void dynamicContrib(const std::vector<std::string>&      restart_group_keys,
                    const std::vector<std::string>&      restart_field_keys,
                    const std::map<std::string, size_t>& groupKeyToIndex,
                    const std::map<std::string, size_t>& fieldKeyToIndex,
                    const Opm::Group&                    group,
                    const Opm::SummaryState&             sumState,
                    XGrpArray&                           xGrp)
{
    using Ix = ::Opm::RestartIO::Helpers::VectorItems::XGroup::index;

    std::string groupName = group.name();
    const std::vector<std::string>& keys = (groupName == "FIELD")
                                           ? restart_field_keys : restart_group_keys;
    const std::map<std::string, size_t>& keyToIndex = (groupName == "FIELD")
            ? fieldKeyToIndex : groupKeyToIndex;

    for (const auto& key : keys) {
        std::string compKey = (groupName == "FIELD")
                              ? key : key + ":" + groupName;

        if (sumState.has(compKey)) {
            double keyValue = sumState.get(compKey);
            const auto itr = keyToIndex.find(key);
            xGrp[itr->second] = keyValue;
        }
    }

    xGrp[Ix::OilPrGuideRate_2]  = xGrp[Ix::OilPrGuideRate];
    xGrp[Ix::WatPrGuideRate_2]  = xGrp[Ix::WatPrGuideRate];
    xGrp[Ix::GasPrGuideRate_2]  = xGrp[Ix::GasPrGuideRate];
    xGrp[Ix::VoidPrGuideRate_2] = xGrp[Ix::VoidPrGuideRate];

    xGrp[Ix::WatInjGuideRate_2] = xGrp[Ix::WatInjGuideRate];

    std::fill(xGrp.begin() + Ix::TracerOffset, xGrp.end(), 0);
}
} // XGrp

namespace ZGrp {
std::size_t entriesPerGroup(const std::vector<int>& inteHead)
{
    return inteHead[Opm::RestartIO::Helpers::VectorItems::NZGRPZ];
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
        WV::NumWindows{ ngmaxz(inteHead) },
        WV::WindowSize{ entriesPerGroup(inteHead) }
    };
}

template <class ZGroupArray>
void staticContrib(const Opm::Group& group, ZGroupArray& zGroup)
{
    zGroup[0] = group.name();
}
} // ZGrp
} // Anonymous



// =====================================================================

Opm::RestartIO::Helpers::AggregateGroupData::
AggregateGroupData(const std::vector<int>& inteHead)
    : iGroup_ (IGrp::allocate(inteHead))
    , sGroup_ (SGrp::allocate(inteHead))
    , xGroup_ (XGrp::allocate(inteHead))
    , zGroup_ (ZGrp::allocate(inteHead))
    , nWGMax_ (nwgmax(inteHead))
    , nGMaxz_ (ngmaxz(inteHead))
{}

// ---------------------------------------------------------------------

void
Opm::RestartIO::Helpers::AggregateGroupData::
captureDeclaredGroupData(const Opm::Schedule&                 sched,
                         const Opm::UnitSystem&               units,
                         const std::size_t                    simStep,
                         const Opm::SummaryState&             sumState,
                         const std::vector<int>&              inteHead)
{
    const auto& curGroups = sched.restart_groups(simStep);
    const auto& sched_state = sched[simStep];

    groupLoop(curGroups, [&sched, simStep, sumState, this]
              (const Group& group, const std::size_t groupID) -> void
    {
        auto ig = this->iGroup_[groupID];
        IGrp::staticContrib(sched, group, this->nWGMax_, this->nGMaxz_,
        simStep, sumState, ig);
    });

    // Define Static Contributions to SGrp Array.
    groupLoop(curGroups,
              [&sumState, &units, &sched_state, this](const Group& group , const std::size_t groupID) -> void
    {
        auto sw = this->sGroup_[groupID];
        SGrp::staticContrib(group, sched_state.glo(), sumState, units, sw);
    });

    // Define Dynamic Contributions to XGrp Array.
    groupLoop(curGroups, [&sumState, this]
              (const Group& group, const std::size_t groupID) -> void
    {
        auto xg = this->xGroup_[groupID];

        XGrp::dynamicContrib(this->restart_group_keys, this->restart_field_keys,
        this->groupKeyToIndex, this->fieldKeyToIndex, group,
        sumState, xg);
    });

    // Define Static Contributions to ZGrp Array.
    groupLoop(curGroups, [this, &inteHead]
              (const Group& group, const std::size_t /* groupID */) -> void
    {
        std::size_t group_index = group.insert_index() - 1;
        if (group.name() == "FIELD")
            group_index = ngmaxz(inteHead) - 1;
        auto zg = this->zGroup_[ group_index ];

        ZGrp::staticContrib(group, zg);
    });
}

