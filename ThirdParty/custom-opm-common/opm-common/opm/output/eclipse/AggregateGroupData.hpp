/*
  Copyright (c) 2018 Statoil ASA

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

#ifndef OPM_AGGREGATE_GROUP_DATA_HPP
#define OPM_AGGREGATE_GROUP_DATA_HPP

#include <opm/output/eclipse/WindowedArray.hpp>

#include <opm/io/eclipse/PaddedOutputString.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <map>

namespace Opm {
class Schedule;
class SummaryState;
//class Group;
class UnitSystem;
} // Opm

namespace Opm { namespace RestartIO { namespace Helpers {

class AggregateGroupData
{
public:
    explicit AggregateGroupData(const std::vector<int>& inteHead);

    void captureDeclaredGroupData(const Opm::Schedule&        sched,
                         const Opm::UnitSystem&               units,
                         const std::size_t                    simStep,
                         const Opm::SummaryState&             sumState,
                         const std::vector<int>&              inteHead);

    const std::vector<int>& getIGroup() const
    {
        return this->iGroup_.data();
    }

    const std::vector<float>& getSGroup() const
    {
        return this->sGroup_.data();
    }

    const std::vector<double>& getXGroup() const
    {
        return this->xGroup_.data();
    }

    const std::vector<EclIO::PaddedOutputString<8>>& getZGroup() const
    {
        return this->zGroup_.data();
    }

    const std::vector<std::string> restart_group_keys = {"GOPP", "GWPP", "GOPR", "GWPR", "GGPR",
                                                         "GVPR", "GWIR", "GGIR", "GWCT", "GGOR",
                                                         "GOPT", "GWPT", "GGPT", "GVPT", "GWIT",
                                                         "GGIT", "GVIT",
                                                         "GOPTH", "GWPTH", "GGPTH",
                                                         "GWITH", "GGITH",
                                                         "GOPGR", "GWPGR", "GGPGR", "GVPGR",
                                                         "GOIGR", "GWIGR", "GGIGR",
                                                        };

    // Note: guide rates don't exist at the FIELD level.
    const std::vector<std::string> restart_field_keys = {"FOPP", "FWPP", "FOPR", "FWPR", "FGPR",
                                                         "FVPR", "FWIR", "FGIR", "FWCT", "FGOR",
                                                         "FOPT", "FWPT", "FGPT", "FVPT", "FWIT",
                                                         "FGIT", "FVIT",
                                                         "FOPTH", "FWPTH", "FGPTH",
                                                         "FWITH", "FGITH"};

    const std::map<std::string, size_t> groupKeyToIndex = {
                                                           {"GOPR",  0},
                                                           {"GWPR",  1},
                                                           {"GGPR",  2},
                                                           {"GVPR",  3},
                                                           {"GWIR",  5},
                                                           {"GGIR",  6},
                                                           {"GWCT",  8},
                                                           {"GGOR",  9},
                                                           {"GOPT", 10},
                                                           {"GWPT", 11},
                                                           {"GGPT", 12},
                                                           {"GVPT", 13},
                                                           {"GWIT", 15},
                                                           {"GGIT", 16},
                                                           {"GVIT", 17},
                                                           {"GOPP", 22},
                                                           {"GWPP", 23},
                                                           {"GOPGR", 85},
                                                           {"GWPGR", 86},
                                                           {"GGPGR", 87},
                                                           {"GVPGR", 88},
                                                           {"GOIGR", 89},
                                                           {"GWIGR", 91},
                                                           {"GGIGR", 93},
                                                           {"GOPTH", 135},
                                                           {"GWPTH", 139},
                                                           {"GWITH", 140},
                                                           {"GGPTH", 143},
                                                           {"GGITH", 144},
    };
    
    
    using inj_cmode_enum = Opm::Group::InjectionCMode;
    const std::map<inj_cmode_enum, int> cmodeToNum = {
        
        {inj_cmode_enum::NONE, 0},
        {inj_cmode_enum::RATE, 1},
        {inj_cmode_enum::RESV, 2},
        {inj_cmode_enum::REIN, 3},
        {inj_cmode_enum::VREP, 4},
        {inj_cmode_enum::FLD,  0},
        {inj_cmode_enum::SALE, 0},
    };

    // Note: guide rates don't exist at the FIELD level.
    const std::map<int, inj_cmode_enum> ICntlModeToiCMode = {
                                                    {0, inj_cmode_enum::NONE},
                                                    {1, inj_cmode_enum::RATE},
                                                    {2, inj_cmode_enum::RESV},
                                                    {3, inj_cmode_enum::REIN},
                                                    {4, inj_cmode_enum::VREP},    };

    using p_cmode = Opm::Group::ProductionCMode;
    const std::map<int, p_cmode> PCntlModeToPCMode = {
                                                    {0, p_cmode::NONE},
                                                    {1, p_cmode::ORAT},
                                                    {2, p_cmode::WRAT},
                                                    {3, p_cmode::GRAT},
                                                    {4, p_cmode::LRAT},
                                                    {9, p_cmode::CRAT},
                                                    {5, p_cmode::RESV},
                                                    {6, p_cmode::PRBL},
    };

    const std::map<std::string, size_t> fieldKeyToIndex = {
                                                           {"FOPR",  0},
                                                           {"FWPR",  1},
                                                           {"FGPR",  2},
                                                           {"FVPR",  3},
                                                           {"FWIR",  5},
                                                           {"FGIR",  6},
                                                           {"FWCT",  8},
                                                           {"FGOR",  9},
                                                           {"FOPT", 10},
                                                           {"FWPT", 11},
                                                           {"FGPT", 12},
                                                           {"FVPT", 13},
                                                           {"FWIT", 15},
                                                           {"FGIT", 16},
                                                           {"FVIT", 17},
                                                           {"FOPP", 22},
                                                           {"FWPP", 23},
                                                           {"FOPTH", 135},
                                                           {"FWPTH", 139},
                                                           {"FWITH", 140},
                                                           {"FGPTH", 143},
                                                           {"FGITH", 144},
    };

private:
    /// Aggregate 'IWEL' array (Integer) for all wells.
    WindowedArray<int> iGroup_;

    /// Aggregate 'SWEL' array (Real) for all wells.
    WindowedArray<float> sGroup_;

    /// Aggregate 'XWEL' array (Double Precision) for all wells.
    WindowedArray<double> xGroup_;

    /// Aggregate 'ZWEL' array (Character) for all wells.
    WindowedArray<EclIO::PaddedOutputString<8>> zGroup_;

    /// Maximum number of wells in a group.
    int nWGMax_;

    /// Maximum number of groups
    int nGMaxz_;
};

}}} // Opm::RestartIO::Helpers

#endif // OPM_AGGREGATE_WELL_DATA_HPP
