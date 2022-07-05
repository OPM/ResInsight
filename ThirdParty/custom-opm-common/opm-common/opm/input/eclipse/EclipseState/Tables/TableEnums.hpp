/*
  Copyright 2015 Statoil ASA.

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


#ifndef OPM_TABLE_ENUMS_HPP
#define OPM_TABLE_ENUMS_HPP

namespace Opm {
    namespace Table {
        enum ColumnOrderEnum {
            INCREASING          = 1,
            STRICTLY_INCREASING = 2,
            DECREASING          = 3,
            STRICTLY_DECREASING = 4,
            RANDOM              = 5
        };

        enum DefaultAction {
            DEFAULT_NONE = 1,
            DEFAULT_CONST = 2,
            DEFAULT_LINEAR = 3,
        };

    }
}



#endif
