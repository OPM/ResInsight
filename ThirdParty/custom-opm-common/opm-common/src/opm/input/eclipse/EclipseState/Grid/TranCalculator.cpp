/*
  Copyright 2020 Equinor AS.

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

#include <opm/input/eclipse/EclipseState/Grid/TranCalculator.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Keywords.hpp>

#include <limits>

namespace Opm
{

namespace Fieldprops
{

keywords::keyword_info<double> TranCalculator::make_kw_info(ScalarOperation op) {
    keywords::keyword_info<double> kw_info;
    switch (op) {
    case ScalarOperation::MUL:
        kw_info.init(1);
        break;
    case ScalarOperation::ADD:
        kw_info.init(0);
        break;
    case ScalarOperation::MAX:
        kw_info.init(std::numeric_limits<double>::max());
        break;
    case ScalarOperation::MIN:
        kw_info.init(std::numeric_limits<double>::lowest());
        break;
    default:
        break;
    }
    return kw_info;
}

} // end namespace Fieldprops
} // end namespace Opm
