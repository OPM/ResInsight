/*
  Copyright 2020 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RST_STATE
#define RST_STATE

#include <vector>
#include <string>

#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/rst/header.hpp>
#include <opm/io/eclipse/rst/group.hpp>
#include <opm/io/eclipse/rst/well.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/Tuning.hpp>


namespace Opm {

namespace RestartIO {
struct RstState {
    RstState(const ::Opm::UnitSystem& unit_system,
             const std::vector<int>& intehead,
             const std::vector<bool>& logihead,
             const std::vector<double>& doubhead);

    RstState(const ::Opm::UnitSystem& unit_system,
             const std::vector<int>& intehead,
             const std::vector<bool>& logihead,
             const std::vector<double>& doubhead,
             const std::vector<std::string>& zgrp,
             const std::vector<int>& igrp,
             const std::vector<float>& sgrp,
             const std::vector<double>& xgrp,
             const std::vector<std::string>& zwel,
             const std::vector<int>& iwel,
             const std::vector<float>& swel,
             const std::vector<double>& xwel,
             const std::vector<int>& icon,
             const std::vector<float>& scon,
             const std::vector<double>& xcon);

    RstState(const ::Opm::UnitSystem& unit_system,
             const std::vector<int>& intehead,
             const std::vector<bool>& logihead,
             const std::vector<double>& doubhead,
             const std::vector<std::string>& zgrp,
             const std::vector<int>& igrp,
             const std::vector<float>& sgrp,
             const std::vector<double>& xgrp,
             const std::vector<std::string>& zwel,
             const std::vector<int>& iwel,
             const std::vector<float>& swel,
             const std::vector<double>& xwel,
             const std::vector<int>& icon,
             const std::vector<float>& scon,
             const std::vector<double>& xcon,
             const std::vector<int>& iseg,
             const std::vector<double>& rseg);


    static RstState load(EclIO::ERst& rst_file, int report_step);

    const RstWell& get_well(const std::string& wname) const;

    const ::Opm::UnitSystem unit_system;
    RstHeader header;
    std::vector<RstWell> wells;
    std::vector<RstGroup> groups;
    Tuning tuning;

private:
    void load_tuning(const std::vector<int>& intehead,
                     const std::vector<double>& doubhead);

    void add_groups(const std::vector<std::string>& zgrp,
                    const std::vector<int>& igrp,
                    const std::vector<float>& sgrp,
                    const std::vector<double>& xgrp);
};
}
}




#endif
