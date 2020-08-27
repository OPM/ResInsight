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

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/GTNode.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <string>
#include <stdexcept>

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


template <typename GroupOp>
void groupLoop(const std::vector<const Opm::Group*>& groups,
               GroupOp&&                             groupOp)
{
    auto groupID = std::size_t{0};
    for (const auto* group : groups) {
        groupID += 1;

        if (group == nullptr) { continue; }

        groupOp(*group, groupID - 1);
    }
}

template < typename T>
std::pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    std::pair<bool, int > result;

    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

    if (it != vecOfElements.end())
    {
        result.second = std::distance(vecOfElements.begin(), it);
        result.first = true;
    }
    else
    {
        result.first = false;
        result.second = -1;
    }
    return result;
}

int currentGroupLevel(const Opm::Schedule& sched, const Opm::Group& group, const size_t simStep)
{
    if (group.defined( simStep )) {
        auto current = group;
        int level = 0;
        while (current.name() != "FIELD") {
            level += 1;
            current = sched.getGroup(current.parent(), simStep);
        }

        return level;
    } else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}

bool groupProductionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const size_t simStep)
{
    using wellCtrlMode   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;
    bool controllable = false;
    if (group.defined( simStep )) {
        if (!group.wellgroup()) {
            if(!group.groups().empty()) {
                for (const auto& group_name : group.groups()) {
                    if (groupProductionControllable(sched, sumState, sched.getGroup(group_name, simStep), simStep)) {
                        controllable = true;
                        continue;
                    }
                }
            }
        }
        else {
            for (const auto& well_name : group.wells()) {
                const auto& well = sched.getWell(well_name, simStep);
                if (well.isProducer()) {
                    int cur_prod_ctrl = 0;
                    // Find control mode for well
                    std::string well_key_1 = "WMCTL:" + well_name;
                    if (sumState.has(well_key_1)) {
                        cur_prod_ctrl = static_cast<int>(sumState.get(well_key_1));
                    }
                    if (cur_prod_ctrl == wellCtrlMode::Group) {
                        controllable = true;
                        continue;
                    }
                }
            }
        }
        return controllable;
    } else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}

bool groupInjectionControllable(const Opm::Schedule& sched, const Opm::SummaryState& sumState, const Opm::Group& group, const Opm::Phase& iPhase, const size_t simStep)
{
    using wellCtrlMode   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;
    bool controllable = false;
    if (group.defined( simStep )) {
        if (!group.wellgroup()) {
            if(!group.groups().empty()) {
                for (const auto& group_name : group.groups()) {
                    if (groupInjectionControllable(sched, sumState, sched.getGroup(group_name, simStep), iPhase, simStep)) {
                        controllable = true;
                        continue;
                    }
                }
            }
        }
        else {
            for (const auto& well_name : group.wells()) {
                const auto& well = sched.getWell(well_name, simStep);
                if (well.isInjector()) {
                    if (((iPhase == Opm::Phase::WATER) && (well.injectionControls(sumState).injector_type ==  Opm::InjectorType::WATER)) ||
                        ((iPhase == Opm::Phase::GAS) && (well.injectionControls(sumState).injector_type ==  Opm::InjectorType::GAS))
                    ) {
                        int cur_inj_ctrl = 0;
                        // Find control mode for well
                        std::string well_key_1 = "WMCTL:" + well_name;
                        if (sumState.has(well_key_1)) {
                            cur_inj_ctrl = static_cast<int>(sumState.get(well_key_1));
                        }
                        if (cur_inj_ctrl == wellCtrlMode::Group) {
                            controllable = true;
                            continue;
                        }
                    }
                }
            }
        }
        return controllable;
    } else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}



int higherLevelProdControlGroupSeqIndex(const Opm::Schedule& sched,
                       const Opm::SummaryState& sumState,
                       const Opm::Group& group,
                       const size_t simStep)
//
// returns the sequence number of higher (highest) level group with active control different from (NONE or FLD)
//
{
    int ctrl_grup_seq_no = -1;
    if (group.defined( simStep )) {
        auto current = group;
        double cur_prod_ctrl = -1.;
        while (current.name() != "FIELD" && ctrl_grup_seq_no < 0) {
            current = sched.getGroup(current.parent(), simStep);
            cur_prod_ctrl = -1.;
            if (sumState.has_group_var(current.name(), "GMCTP")) {
                cur_prod_ctrl = sumState.get_group_var(current.name(), "GMCTP");
            }
            else {
#if ENABLE_GCNTL_DEBUG_OUTPUT
                std::cout << "Current group control is not defined for group: " << current.name() << " at timestep: " << simStep  << std::endl;
#endif // ENABLE_GCNTL_DEBUG_OUTPUT
                cur_prod_ctrl = 0.;
            }
            if (cur_prod_ctrl > 0. && ctrl_grup_seq_no < 0) {
                ctrl_grup_seq_no = current.insert_index();
            }
        }
        return ctrl_grup_seq_no;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}

int higherLevelProdControlMode(const Opm::Schedule& sched,
                       const Opm::SummaryState& sumState,
                       const Opm::Group& group,
                       const size_t simStep)
//
// returns the sequence number of higher (highest) level group with active control different from (NONE or FLD)
//
{
    int ctrl_mode = -1;
    if (group.defined( simStep )) {
        auto current = group;
        double  cur_prod_ctrl = -1.;
        while (current.name() != "FIELD" && ctrl_mode < 0.) {
            current = sched.getGroup(current.parent(), simStep);
            cur_prod_ctrl = -1.;
            if (sumState.has_group_var(current.name(), "GMCTP")) {
                cur_prod_ctrl = sumState.get_group_var(current.name(), "GMCTP");
            }
            else {
#if ENABLE_GCNTL_DEBUG_OUTPUT
                std::cout << "Current group control is not defined for group: " << current.name() << " at timestep: " << simStep  << std::endl;
#endif // ENABLE_GCNTL_DEBUG_OUTPUT
                cur_prod_ctrl = 0.;
            }
            if (cur_prod_ctrl > 0. && ctrl_mode < 0) {
                ctrl_mode = static_cast<int>(cur_prod_ctrl);
            }
        }
        return ctrl_mode;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}





int higherLevelInjControlGroupSeqIndex(const Opm::Schedule& sched,
                       const Opm::SummaryState& sumState,
                       const Opm::Group& group,
                       const std::string curInjCtrlKey,
                       const size_t simStep)
//
// returns the sequence number of higher (highest) level group with active control different from (NONE or FLD)
//
{
    int ctrl_grup_seq_no = -1;
    if (group.defined( simStep )) {
        auto current = group;
        double cur_inj_ctrl = -1.;
        while (current.name() != "FIELD" && ctrl_grup_seq_no < 0) {
            current = sched.getGroup(current.parent(), simStep);
            cur_inj_ctrl = -1.;
            if (sumState.has_group_var(current.name(), curInjCtrlKey)) {
                cur_inj_ctrl = sumState.get_group_var(current.name(), curInjCtrlKey);
            }
            else {
#if ENABLE_GCNTL_DEBUG_OUTPUT
                std::cout << "Current injection group control: " << curInjCtrlKey << " is not defined for group: " << current.name() << " at timestep: " << simStep << std::endl;
#endif // ENABLE_GCNTL_DEBUG_OUTPUT
                cur_inj_ctrl = 0.;
            }
            if (cur_inj_ctrl > 0. && ctrl_grup_seq_no < 0) {
                ctrl_grup_seq_no = current.insert_index();
            }
        }
        return ctrl_grup_seq_no;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}

std::vector<std::size_t> groupParentSeqIndex(const Opm::Schedule& sched,
                       const Opm::Group& group,
                       const size_t simStep)
//
// returns a vector with the group sequence index of all parent groups from current parent group to Field level
//
{
    std::vector<std::size_t> seq_numbers;
    if (group.defined( simStep )) {
        auto current = group;
        while (current.name() != "FIELD") {
            current = sched.getGroup(current.parent(), simStep);
            seq_numbers.push_back(current.insert_index());
        }
        return seq_numbers;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}



bool higherLevelProdCMode_NotNoneFld(const Opm::Schedule& sched,
                       const Opm::SummaryState& sumState,
                       const Opm::Group& group,
                       const size_t simStep)
{
    bool ctrl_mode_not_none_fld = false;
    if (group.defined( simStep )) {
        auto current = group;
        while (current.name() != "FIELD" && ctrl_mode_not_none_fld == false) {
            current = sched.getGroup(current.parent(), simStep);
            const auto& prod_cmode = current.productionControls(sumState).cmode;
            if ((prod_cmode != Opm::Group::ProductionCMode::FLD) || (prod_cmode!= Opm::Group::ProductionCMode::NONE)) {
                ctrl_mode_not_none_fld = true;
            }
        }
        return ctrl_mode_not_none_fld;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}

int higherLevelInjCMode_NotNoneFld_SeqIndex(const Opm::Schedule& sched,
                       const Opm::SummaryState& sumState,
                       const Opm::Group& group,
                       const Opm::Phase& phase,
                       const size_t simStep)
{
    int ctrl_mode_not_none_fld = -1;
    if (group.defined( simStep )) {
        auto current = group;
        while (current.name() != "FIELD" && ctrl_mode_not_none_fld < 0) {
            current = sched.getGroup(current.parent(), simStep);
            const auto& inj_cmode = (current.hasInjectionControl(phase)) ?
            current.injectionControls(phase, sumState).cmode : Opm::Group::InjectionCMode::NONE;
            if ((inj_cmode != Opm::Group::InjectionCMode::FLD) || (inj_cmode!= Opm::Group::InjectionCMode::NONE)) {
                if (ctrl_mode_not_none_fld == -1) {
                     ctrl_mode_not_none_fld = current.insert_index();
                }
            }
        }
        return ctrl_mode_not_none_fld;
    }
    else {
        std::stringstream str;
        str << "actual group has not been defined at report time: " << simStep;
        throw std::invalid_argument(str.str());
    }
}


int groupType(const Opm::Group& group) {
    if (group.wellgroup())
        return 0;
    else
        return 1;
}


std::size_t groupSize(const Opm::Group& group) {
    if (group.wellgroup())
        return group.wells().size();
    else
        return group.groups().size();
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
void staticContrib(const Opm::Schedule&     sched,
                   const Opm::Group&        group,
                   const int                nwgmax,
                   const int                ngmaxz,
                   const std::size_t        simStep,
                   const Opm::SummaryState& sumState,
                   const std::map<int, Opm::Group::ProductionCMode>& pCtrlToPCmode,
                   const std::map<Opm::Group::InjectionCMode, int>& cmodeToNum,
                   IGrpArray&               iGrp)
{
    const bool is_field = group.name() == "FIELD";
    if (group.wellgroup()) {
        int igrpCount = 0;
        //group has child wells
        //store the well number (sequence index) in iGrp according to the sequence they are defined
        for (const auto& well_name : group.wells()) {
            const auto& well = sched.getWell(well_name, simStep);
            iGrp[igrpCount] = well.seqIndex() + 1;
            igrpCount += 1;
        }
    } else if (!group.groups().empty()) {
        int igrpCount = 0;
        for (const auto& group_name : group.groups()) {
            const auto& child_group = sched.getGroup(group_name, simStep);
            iGrp[igrpCount] = child_group.insert_index();
            igrpCount += 1;
        }
    }

    //assign the number of child wells or child groups to
    // location nwgmax
    iGrp[nwgmax] = groupSize(group);

    // Find number of active production wells and injection wells for group
    const double g_act_pwells = is_field ? sumState.get("FMWPR", 0) : sumState.get_group_var(group.name(), "GMWPR", 0);
    const double g_act_iwells = is_field ? sumState.get("FMWIN", 0) : sumState.get_group_var(group.name(), "GMWIN", 0);
    iGrp[nwgmax + 33] = g_act_pwells + g_act_iwells;

    //Treat groups that have production
    if ((group.getGroupType() == Opm::Group::GroupType::NONE) || (group.getGroupType() == Opm::Group::GroupType::PRODUCTION)
         || (group.getGroupType() == Opm::Group::GroupType::MIXED)) {

        const auto& prod_cmode = group.productionControls(sumState).cmode;
        const auto& prod_guide_rate_def = group.productionControls(sumState).guide_rate_def;
        const auto& p_exceed_act = group.productionControls(sumState).exceed_action;
        // Find production control mode for group
        const double cur_prod_ctrl = is_field ? sumState.get("FMCTP", -1) : sumState.get_group_var(group.name(), "GMCTP", -1);
        Opm::Group::ProductionCMode pctl_mode = Opm::Group::ProductionCMode::NONE;
        if (cur_prod_ctrl >= 0) {
            const auto it_ctrl = pCtrlToPCmode.find(cur_prod_ctrl);
            if (it_ctrl != pCtrlToPCmode.end()) {
                pctl_mode = it_ctrl->second;
            }
        }
#if ENABLE_GCNTL_DEBUG_OUTPUT
        else {
            //std::stringstream str;
            //str << "Current group production control is not defined for group: " << group.name() << " at timestep: " << simStep;
            std::cout << "Current group production control is not defined for group: " << group.name() << " at timestep: " << simStep << std::endl;
            //throw std::invalid_argument(str.str());
        }
#endif // ENABLE_GCNTL_DEBUG_OUTPUT

        /*IGRP[NWGMAX + 5]
         - the value is determined by a relatively complex logic, a pseudo code scetch follows:
         if (group is free to respond to higher level production rate target_reinj_fraction)
             iGrp[nwgmax + 5] = 0
         else if (group::control mode > 0) (controlled by own limit)
             iGrp[nwgmax + 5] = -1
         else if (a higher level group control is active constraint)
             if (group control mode is NOT == ("FLD" OR "NONE"))
                 iGrp[nwgmax + 5] = group_sequence_no_of controlling group
             else
                 iGrp[nwgmax + 5] = 1
         else if (a higher level group has a control mode NOT == ("FLD" OR "NONE"))
             if (group control mode is NOT == ("FLD" OR "NONE"))
                iGrp[nwgmax + 5] = -1
             else
                 iGrp[nwgmax + 5] = 1
         else if (group control mode is == ("FLD" OR "NONE"))
                iGrp[nwgmax + 5] = -1
             else
                 iGrp[nwgmax + 5] = -1

        */
        if (group.name() != "FIELD") {
            //default value
            iGrp[nwgmax + 5] = -1;
            int higher_lev_ctrl = higherLevelProdControlGroupSeqIndex(sched, sumState, group, simStep);
            int higher_lev_ctrl_mode = higherLevelProdControlMode(sched, sumState, group, simStep);
            //Start branching for determining iGrp[nwgmax + 5]
            //use default value if group is not available for group control
            if (groupProductionControllable(sched, sumState, group, simStep)) {
                //this section applies if group is controllable - i.e. has wells that may be controlled
                if (!group.productionGroupControlAvailable() && (higher_lev_ctrl <= 0)) {
                    //group can respond to higher level control
                    iGrp[nwgmax + 5] = 0;
                }
                else if (((pctl_mode != Opm::Group::ProductionCMode::NONE)) && (higher_lev_ctrl < 0)) {
                    //group is constrained by its own limits or controls
                    // if (pctl_mode != Opm::Group::ProductionCMode::FLD)  -  need to use this test? - else remove
                    iGrp[nwgmax + 5] = -1;  // only value that seems to work when no group at higher level has active control
                }
                else if (higher_lev_ctrl > 0) {
                    if (((prod_cmode == Opm::Group::ProductionCMode::FLD) || (prod_cmode == Opm::Group::ProductionCMode::NONE))
                        && (group.productionControls(sumState).guide_rate_def != Opm::Group::GuideRateTarget::NO_GUIDE_RATE)) {
                        iGrp[nwgmax + 5] = higher_lev_ctrl;
                    }
                    else {
                        iGrp[nwgmax + 5] = 1;
                    }
                }
                else if (higherLevelProdCMode_NotNoneFld(sched, sumState, group, simStep)) {
                    if ((prod_cmode != Opm::Group::ProductionCMode::FLD) || (prod_cmode!= Opm::Group::ProductionCMode::NONE)) {
                        iGrp[nwgmax + 5] = -1;
                    }
                    else {
                        iGrp[nwgmax + 5] = 1;
                    }
                }
                else if ((prod_cmode == Opm::Group::ProductionCMode::FLD) || (prod_cmode == Opm::Group::ProductionCMode::NONE)) {
                    iGrp[nwgmax + 5] = -1;
                }
                else {
                    iGrp[nwgmax + 5] = -1;
                }
            }
            else if (prod_cmode == Opm::Group::ProductionCMode::NONE){
                iGrp[nwgmax + 5] = 1;
            }

            // Set iGrp for [nwgmax + 7]
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

            if (higher_lev_ctrl > 0 && (group.getGroupType() != Opm::Group::GroupType::NONE)) {
                iGrp[nwgmax + 1] = ( prod_guide_rate_def != Opm::Group::GuideRateTarget::NO_GUIDE_RATE ) ? higher_lev_ctrl_mode : 0;
            }
            else {
                switch (pctl_mode) {
                    case Opm::Group::ProductionCMode::NONE:
                        iGrp[nwgmax + 1] = 0;
                        break;
                    case Opm::Group::ProductionCMode::ORAT:
                        iGrp[nwgmax + 1] = 1;
                        break;
                    case Opm::Group::ProductionCMode::WRAT:
                        iGrp[nwgmax + 1] = 2;
                        break;
                    case Opm::Group::ProductionCMode::GRAT:
                        iGrp[nwgmax + 1] = 3;
                        break;
                    case Opm::Group::ProductionCMode::LRAT:
                        iGrp[nwgmax + 1] = 4;
                        break;
                    case Opm::Group::ProductionCMode::RESV:
                        iGrp[nwgmax + 1] = 5;
                        break;
                    case Opm::Group::ProductionCMode::FLD:
                        iGrp[nwgmax + 1] = 0;   // need to be checked!!
                        break;
                    default:
                        iGrp[nwgmax + 1] = 0;
                }
            }
            iGrp[nwgmax + 9] = iGrp[nwgmax + 1];

            switch (prod_cmode) {
                case Opm::Group::ProductionCMode::NONE:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? 0 : 4;
                    iGrp[nwgmax + 10] = 0;
                    break;
                case Opm::Group::ProductionCMode::ORAT:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -40000 : 4;
                    iGrp[nwgmax + 10] = 1;
                    break;
                case Opm::Group::ProductionCMode::WRAT:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -4000 : 4;
                    iGrp[nwgmax + 10] = 2;
                    break;
                case Opm::Group::ProductionCMode::GRAT:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -400 : 4;
                    iGrp[nwgmax + 10] = 3;
                    break;
                case Opm::Group::ProductionCMode::LRAT:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -40 : 4;
                    iGrp[nwgmax + 10] = 4;
                    break;
                case Opm::Group::ProductionCMode::RESV:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? -4 : 4;  // need to be checked
                    iGrp[nwgmax + 10] = 5;
                    break;
                case Opm::Group::ProductionCMode::FLD:
                    iGrp[nwgmax + 6]  = 0;
                    if (( higher_lev_ctrl > 0 ) && ( prod_guide_rate_def != Opm::Group::GuideRateTarget::NO_GUIDE_RATE )) {
                        iGrp[nwgmax + 6]  = 8;
                    }
                    iGrp[nwgmax + 7] = (p_exceed_act == Opm::Group::ExceedAction::NONE) ? 4 : 4;
                    iGrp[nwgmax + 10] = 0;   // need to be checked!!
                    break;
                default:
                    iGrp[nwgmax + 6]  = 0;
                    iGrp[nwgmax + 7]  = 0;
                    iGrp[nwgmax + 10] = 0;   // need to be checked!!
                }
        }
        else {  // group name is "FIELD"
            iGrp[nwgmax + 6]  = 0;
            iGrp[nwgmax + 7] = 0;
            switch (prod_cmode) {
                case Opm::Group::ProductionCMode::NONE:
                    iGrp[nwgmax + 10] = 0;
                    break;
                case Opm::Group::ProductionCMode::ORAT:
                    iGrp[nwgmax + 10] = 1;
                    break;
                case Opm::Group::ProductionCMode::WRAT:
                    iGrp[nwgmax + 10] = 2;
                    break;
                case Opm::Group::ProductionCMode::GRAT:
                    iGrp[nwgmax + 10] = 3;
                    break;
                case Opm::Group::ProductionCMode::LRAT:
                    iGrp[nwgmax + 10] = 4;
                    break;
                case Opm::Group::ProductionCMode::RESV:
                    iGrp[nwgmax + 10] = 5;
                    break;
                case Opm::Group::ProductionCMode::FLD:
                    iGrp[nwgmax + 10] = 0;
                    break;
                default:
                    iGrp[nwgmax + 10] = 0;
            }
        }
    }
    //default value -
    iGrp[nwgmax + 17] = -1;
    iGrp[nwgmax + 22] = -1;
    if (group.isInjectionGroup() || (group.getGroupType() == Opm::Group::GroupType::MIXED) || (group.getGroupType() == Opm::Group::GroupType::NONE)) {
        auto group_parent_list = groupParentSeqIndex(sched, group, simStep);

        //set "default value" in case a group is only injection group
        if (group.isInjectionGroup() && !group.isProductionGroup()) {
            iGrp[nwgmax + 5] = 1;
        }
        //use default value if group is not available for group control
        if (groupInjectionControllable(sched, sumState, group, Opm::Phase::WATER, simStep)) {
            if ((group.hasInjectionControl(Opm::Phase::WATER))  || (group.getGroupType() == Opm::Group::GroupType::NONE)){
                const double cur_winj_ctrl = is_field ? sumState.get("FMCTW", -1) : sumState.get_group_var(group.name(), "GMCTW", -1);
                const auto& winj_cmode = (group.hasInjectionControl(Opm::Phase::WATER))?
                group.injectionControls(Opm::Phase::WATER, sumState).cmode : Opm::Group::InjectionCMode::NONE;
                if (!is_field) {
                    int higher_lev_winj_ctrl = higherLevelInjControlGroupSeqIndex(sched, sumState, group, "GMCTW", simStep);
                    int higher_lev_winj_cmode = higherLevelInjCMode_NotNoneFld_SeqIndex(sched, sumState, group, Opm::Phase::WATER, simStep);
                    std::size_t winj_control_ind = 0;
                    std::size_t inj_cmode_ind = 0;

                    //WATER INJECTION GROUP CONTROL

                    //Start branching for determining iGrp[nwgmax + 17]
                    if (cur_winj_ctrl > 0.) {
                            iGrp[nwgmax + 17] = 0;
                    }
                    if (!group.injectionGroupControlAvailable(Opm::Phase::WATER) && (higher_lev_winj_ctrl <= 0)) {
                        //group can respond to higher level control
                        iGrp[nwgmax + 17] = 0;
                    }
                    else if (higher_lev_winj_ctrl > 0 || higher_lev_winj_cmode > 0) {
                        if ((winj_cmode != Opm::Group::InjectionCMode::FLD) || (winj_cmode!= Opm::Group::InjectionCMode::NONE)) {
                            if (!(higher_lev_winj_ctrl == higher_lev_winj_cmode)) {

                                auto result = findInVector<std::size_t>(group_parent_list, higher_lev_winj_ctrl);
                                if (result.first) {
                                    winj_control_ind = result.second;
                                }
                                else {
                                    winj_control_ind =  99999;
                                }

                                result = findInVector<std::size_t>(group_parent_list, higher_lev_winj_cmode);
                                if (result.first) {
                                    inj_cmode_ind = result.second;
                                }
                                else {
                                    inj_cmode_ind =  99999;
                                }

                                if (winj_control_ind < inj_cmode_ind) {
                                    iGrp[nwgmax + 17] = higher_lev_winj_ctrl;
                                }
                                else {
                                    iGrp[nwgmax + 17] = higher_lev_winj_cmode;
                                }
                                iGrp[nwgmax + 17] = higher_lev_winj_ctrl;
                            }
                            else {
                                iGrp[nwgmax + 17] = higher_lev_winj_ctrl;
                            }
                        }
                        else {
                            iGrp[nwgmax + 17] = 1;
                        }
                    }
                    else {
                        iGrp[nwgmax + 17] = -1;
                    }
                }
                else {  // group name "FIELD"
                    iGrp[nwgmax + 17] = 0;
                    iGrp[nwgmax + 22] = 0;
                }
                //item[nwgmax + 16] - mode for operation for water injection
                // 1 - RATE
                // 2 - RESV
                // 3 - REIN
                // 4 - VREP
                // 0 - ellers
                const auto& inj_mode = (group.hasInjectionControl(Opm::Phase::WATER)) ?
                group.injectionControls(Opm::Phase::WATER, sumState).cmode : Opm::Group::InjectionCMode::NONE;
                const auto it = cmodeToNum.find(inj_mode);
                if (it != cmodeToNum.end()) {
                    iGrp[nwgmax + 16] = it->second;
                    iGrp[nwgmax + 18] = iGrp[nwgmax + 16];
                    iGrp[nwgmax + 19] = iGrp[nwgmax + 16];
                }
            }
        }

        //use default value if group is not available for group control
        if (groupInjectionControllable(sched, sumState, group, Opm::Phase::GAS, simStep)) {
            if ((group.hasInjectionControl(Opm::Phase::GAS)) || (group.getGroupType() == Opm::Group::GroupType::NONE)) {
                const double cur_ginj_ctrl = is_field ? sumState.get("FMCTG", -1) : sumState.get_group_var(group.name(), "GMCTG", -1);
                const auto& ginj_cmode = (group.hasInjectionControl(Opm::Phase::GAS))?
                group.injectionControls(Opm::Phase::GAS, sumState).cmode : Opm::Group::InjectionCMode::NONE;
                if (!is_field) {
                    int higher_lev_ginj_ctrl = higherLevelInjControlGroupSeqIndex(sched, sumState, group, "GMCTG", simStep);
                    int higher_lev_ginj_cmode = higherLevelInjCMode_NotNoneFld_SeqIndex(sched, sumState, group, Opm::Phase::GAS, simStep);
                    std::size_t ginj_control_ind = 0;
                    std::size_t inj_cmode_ind = 0;
                    //GAS INJECTION GROUP CONTROL
                    //Start branching for determining iGrp[nwgmax + 22]
                    if (cur_ginj_ctrl > 0.) {
                            iGrp[nwgmax + 22] = 0;
                    }
                    if (!group.injectionGroupControlAvailable(Opm::Phase::GAS) && (higher_lev_ginj_ctrl <= 0)) {
                        //group can respond to higher level control
                        iGrp[nwgmax + 22] = 0;
                    }
                    else if (higher_lev_ginj_ctrl > 0 || higher_lev_ginj_cmode > 0) {
                        if ((ginj_cmode != Opm::Group::InjectionCMode::FLD) || (ginj_cmode!= Opm::Group::InjectionCMode::NONE)) {
                            if (!(higher_lev_ginj_ctrl == higher_lev_ginj_cmode)) {

                                auto result = findInVector<std::size_t>(group_parent_list, higher_lev_ginj_ctrl);
                                if (result.first) {
                                    ginj_control_ind = result.second;
                                }
                                else {
                                    ginj_control_ind =  99999;
                                }

                                result = findInVector<std::size_t>(group_parent_list, higher_lev_ginj_cmode);
                                if (result.first) {
                                    inj_cmode_ind = result.second;
                                }
                                else {
                                    inj_cmode_ind =  99999;
                                }

                                if (ginj_control_ind < inj_cmode_ind) {
                                    iGrp[nwgmax + 22] = higher_lev_ginj_ctrl;
                                }
                                else {
                                    iGrp[nwgmax + 22] = higher_lev_ginj_cmode;
                                }
                                iGrp[nwgmax + 22] = higher_lev_ginj_ctrl;
                            }
                            else {
                                iGrp[nwgmax + 22] = higher_lev_ginj_ctrl;
                            }
                        }
                        else {
                            iGrp[nwgmax + 22] = 1;
                        }
                    }
                    else {
                        iGrp[nwgmax + 22] = -1;
                    }
                }
                else {  // group name "FIELD"
                    iGrp[nwgmax + 17] = 0;
                    iGrp[nwgmax + 22] = 0;
                    //parameters connected to oil injection - not implemented in flow yet
                    iGrp[nwgmax+11] = 0;
                    iGrp[nwgmax+12] = 0;

                }
                //item[nwgmax + 21] - mode for operation for gas injection
                // 1 - RATE
                // 2 - RESV
                // 3 - REIN
                // 4 - VREP
                // 0 - ellers
                const auto& inj_mode =  (group.hasInjectionControl(Opm::Phase::GAS)) ?
                group.injectionControls(Opm::Phase::GAS, sumState).cmode : Opm::Group::InjectionCMode::NONE;
                const auto it = cmodeToNum.find(inj_mode);
                if (it != cmodeToNum.end()) {
                    iGrp[nwgmax + 21] = it->second;
                    iGrp[nwgmax + 23] = iGrp[nwgmax + 21];
                    iGrp[nwgmax + 24] = iGrp[nwgmax + 21];
                }
            }
        }
    }
    
    iGrp[nwgmax + 26] = groupType(group);

    //find group level ("FIELD" is level 0) and store the level in
    //location nwgmax + 27
    iGrp[nwgmax+27] = currentGroupLevel(sched, group, simStep);

    // set values for group probably connected to GCONPROD settings
    //
    if (group.name() != "FIELD")
    {
        //parameters connected to oil injection - not implemented in flow yet
        iGrp[nwgmax+11] = 0;
        iGrp[nwgmax+12] = -1;

        //assign values to group number (according to group sequence)
        iGrp[nwgmax+88] = group.insert_index();
        iGrp[nwgmax+89] = group.insert_index();
        iGrp[nwgmax+95] = group.insert_index();
        iGrp[nwgmax+96] = group.insert_index();
    }
    else
    {
        //assign values to group number (according to group sequence)
        iGrp[nwgmax+88] = ngmaxz;
        iGrp[nwgmax+89] = ngmaxz;
        iGrp[nwgmax+95] = ngmaxz;
        iGrp[nwgmax+96] = ngmaxz;
    }

    //find parent group and store index of parent group in
    //location nwgmax + 28
    if (group.name() == "FIELD")
        iGrp[nwgmax+28] = 0;
    else {
        const auto& parent_group = sched.getGroup(group.parent(), simStep);
        if (parent_group.name() == "FIELD")
            iGrp[nwgmax+28] = ngmaxz;
        else
            iGrp[nwgmax+28] = parent_group.insert_index();
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

template <class SGrpArray>
void staticContrib(const Opm::Group&        group,
                   const Opm::SummaryState& sumState,
                   const Opm::UnitSystem& units,
                   SGrpArray&               sGrp)
{
    using Isp = ::Opm::RestartIO::Helpers::VectorItems::SGroup::prod_index;
    using Isi = ::Opm::RestartIO::Helpers::VectorItems::SGroup::inj_index;
    using M = ::Opm::UnitSystem::measure;
    
    const auto dflt   = -1.0e+20f;
    const auto dflt_2 = -2.0e+20f;
    const auto infty  =  1.0e+20f;
    const auto zero   =  0.0f;
    const auto one    =  1.0f;

    const auto init = std::vector<float> { // 112 Items (0..111)
                                          // 0     1      2      3      4
                                          infty, infty, dflt , infty,  zero ,     //   0..  4  ( 0)
                                          zero , infty, infty, infty , infty,     //   5..  9  ( 1)
                                          infty, infty, infty, infty , dflt ,     //  10.. 14  ( 2)
                                          infty, infty, infty, infty , dflt ,     //  15.. 19  ( 3)
                                          infty, infty, infty, infty , dflt ,     //  20.. 24  ( 4)
                                          zero , zero , zero , dflt_2, zero ,     //  24.. 29  ( 5)
                                          zero , zero , zero , zero  , zero ,     //  30.. 34  ( 6)
                                          infty ,zero , zero , zero  , infty,     //  35.. 39  ( 7)
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

    const auto sz = static_cast<
        decltype(init.size())>(sGrp.size());

    auto b = std::begin(init);
    auto e = b + std::min(init.size(), sz);

    std::copy(b, e, std::begin(sGrp));
    
    auto sgprop = [&units](const M u, const double x) -> float
    {
        return static_cast<float>(units.from_si(u, x));
    };
    
    if (group.isProductionGroup()) {
        const auto& prod_cntl = group.productionControls(sumState);
        
        if (prod_cntl.oil_target > 0.) {
            sGrp[Isp::OilRateLimit] = sgprop(M::liquid_surface_rate, prod_cntl.oil_target);
            sGrp[52] = sGrp[Isp::OilRateLimit];  // "ORAT" control
        }
        if (prod_cntl.water_target > 0.) {
            sGrp[Isp::WatRateLimit] = sgprop(M::liquid_surface_rate, prod_cntl.water_target);
            sGrp[53] = sGrp[Isp::WatRateLimit];  //"WRAT" control
        }
        if (prod_cntl.gas_target > 0.) {
            sGrp[Isp::GasRateLimit] = sgprop(M::gas_surface_rate, prod_cntl.gas_target);
            sGrp[39] = sGrp[Isp::GasRateLimit];
        }
        if (prod_cntl.liquid_target > 0.) {
            sGrp[Isp::LiqRateLimit] = sgprop(M::liquid_surface_rate, prod_cntl.liquid_target);
            sGrp[54] = sGrp[Isp::LiqRateLimit];  //"LRAT" control
        }
    }

    if ((group.name() == "FIELD") && (group.getGroupType() == Opm::Group::GroupType::NONE)) {
          sGrp[2] = 0.;
          sGrp[14] = 0.;
          sGrp[19] = 0.;
          sGrp[24] = 0.;
    }
    
    if (group.isInjectionGroup()) {
        if (group.hasInjectionControl(Opm::Phase::GAS)) {
            const auto& inj_cntl = group.injectionControls(Opm::Phase::GAS, sumState);
            if (inj_cntl.surface_max_rate > 0.) {
                sGrp[Isi::gasSurfRateLimit] = sgprop(M::gas_surface_rate, inj_cntl.surface_max_rate);
                sGrp[65] =  sGrp[Isi::gasSurfRateLimit];
            }
            if (inj_cntl.resv_max_rate > 0.) {
                sGrp[Isi::gasResRateLimit] = sgprop(M::rate, inj_cntl.resv_max_rate);
                sGrp[66] =  sGrp[Isi::gasResRateLimit];
            }
            if (inj_cntl.target_reinj_fraction > 0.) {
                sGrp[Isi::gasReinjectionLimit] = inj_cntl.target_reinj_fraction;
                sGrp[67] =  sGrp[Isi::gasReinjectionLimit];
            }
            if (inj_cntl.target_void_fraction > 0.) {
                sGrp[Isi::gasVoidageLimit] = inj_cntl.target_void_fraction;
                sGrp[68] =  sGrp[Isi::gasVoidageLimit];
            }
        }

        if (group.hasInjectionControl(Opm::Phase::WATER)) {
            const auto& inj_cntl = group.injectionControls(Opm::Phase::WATER, sumState);
            if (inj_cntl.surface_max_rate > 0.) {
                sGrp[Isi::waterSurfRateLimit] = sgprop(M::liquid_surface_rate, inj_cntl.surface_max_rate);
                sGrp[61] =  sGrp[Isi::waterSurfRateLimit];
            }
            if (inj_cntl.resv_max_rate > 0.) {
                sGrp[Isi::waterResRateLimit] = sgprop(M::rate, inj_cntl.resv_max_rate);
                sGrp[62] =  sGrp[Isi::waterResRateLimit];
            }
            if (inj_cntl.target_reinj_fraction > 0.) {
                sGrp[Isi::waterReinjectionLimit] = inj_cntl.target_reinj_fraction;
                sGrp[63] =  sGrp[Isi::waterReinjectionLimit];
            }
            if (inj_cntl.target_void_fraction > 0.) {
                sGrp[Isi::waterVoidageLimit] = inj_cntl.target_void_fraction;
                sGrp[64] =  sGrp[Isi::waterVoidageLimit];
            }
        }

        if (group.hasInjectionControl(Opm::Phase::OIL)) {
            const auto& inj_cntl = group.injectionControls(Opm::Phase::OIL, sumState);
            if (inj_cntl.surface_max_rate > 0.) {
                sGrp[Isi::oilSurfRateLimit] = sgprop(M::liquid_surface_rate, inj_cntl.surface_max_rate);
                sGrp[57] =  sGrp[Isi::oilSurfRateLimit];
            }
            if (inj_cntl.resv_max_rate > 0.) {
                sGrp[Isi::oilResRateLimit] = sgprop(M::rate, inj_cntl.resv_max_rate);
                sGrp[58] =  sGrp[Isi::oilResRateLimit];
            }
            if (inj_cntl.target_reinj_fraction > 0.) {
                sGrp[Isi::oilReinjectionLimit] = inj_cntl.target_reinj_fraction;
                sGrp[59] =  sGrp[Isi::oilReinjectionLimit];
            }
            if (inj_cntl.target_void_fraction > 0.) {
                sGrp[Isi::oilVoidageLimit] = inj_cntl.target_void_fraction;
                sGrp[60] =  sGrp[Isi::oilVoidageLimit];
            }
        }

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

    groupLoop(curGroups, [&sched, simStep, sumState, this]
              (const Group& group, const std::size_t groupID) -> void
                         {
                             auto ig = this->iGroup_[groupID];

                             IGrp::staticContrib(sched, group, this->nWGMax_, this->nGMaxz_,
                                                 simStep, sumState, this->PCntlModeToPCMode, this->cmodeToNum, ig);
                         });

    // Define Static Contributions to SGrp Array.
    groupLoop(curGroups,
              [&sumState, &units, this](const Group& group , const std::size_t groupID) -> void
              {
                  auto sw = this->sGroup_[groupID];
                  SGrp::staticContrib(group, sumState, units, sw);
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
