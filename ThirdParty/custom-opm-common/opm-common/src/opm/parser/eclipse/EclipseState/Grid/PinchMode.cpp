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

#include <opm/parser/eclipse/EclipseState/Grid/PinchMode.hpp>
#include <opm/common/utility/String.hpp>

#include <stdexcept>

namespace Opm {

    namespace PinchMode {

        const std::string PinchMode2String(const ModeEnum enumValue) {
            std::string stringValue;
            switch (enumValue) {
            case ModeEnum::ALL:
                stringValue = "ALL";
                break;

            case ModeEnum::TOPBOT:
                stringValue = "TOPBOT";
                break;

            case ModeEnum::TOP:
                stringValue = "TOP";
                break;
            }

            return stringValue;
        }

        ModeEnum PinchModeFromString(const std::string& stringValue) {
            std::string s = trim_copy(stringValue);

            ModeEnum mode;
            if      (s == "ALL")    { mode = ModeEnum::ALL;    }
            else if (s == "TOPBOT") { mode = ModeEnum::TOPBOT; }
            else if (s == "TOP")    { mode = ModeEnum::TOP;    }
            else {
                std::string msg = "Unsupported pinchout mode " + s;
                throw std::invalid_argument(msg);
            }

            return mode;
        }
    }
}
