/*
  Copyright 2019 Equinor ASA.

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

#ifndef VALUE_STATUS
#define VALUE_STATUS

namespace Opm {

namespace value {

enum class status : unsigned char { uninitialized = 0,
                                    deck_value = 1,
                                    empty_default = 2,
                                    valid_default = 3 };


inline bool defaulted(status st) {
    if (st == status::empty_default)
        return true;

    if (st == status::valid_default)
        return true;

    return false;
}


inline bool has_value(status st) {
    if (st == status::deck_value)
        return true;

    if (st == status::valid_default)
        return true;

    return false;
}
}
}

#endif
