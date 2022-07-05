/*
  Copyright 2014 Statoil ASA.

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

#include <stdexcept>

#include <opm/input/eclipse/EclipseState/Grid/FaceDir.hpp>



namespace Opm {

    namespace FaceDir {

        DirEnum FromString(const std::string& stringValue) {
            if ((stringValue == "X") || (stringValue == "I") || (stringValue == "X+")  || (stringValue == "I+"))
                return XPlus;
            if ((stringValue == "X-") || (stringValue == "I-"))
                return XMinus;

            if ((stringValue == "Y") || (stringValue == "J") || (stringValue == "Y+")  || (stringValue == "J+"))
                return YPlus;
            if ((stringValue == "Y-") || (stringValue == "J-"))
                return YMinus;

            if ((stringValue == "Z") || (stringValue == "K") || (stringValue == "Z+")  || (stringValue == "K+"))
                return ZPlus;
            if ((stringValue == "Z-") || (stringValue == "K-"))
                return ZMinus;

            throw std::invalid_argument("The string value " + stringValue + " could not be converted to a FaceDir enum value");
        }


        int FromMULTREGTString(const std::string& stringValue) {
            if (stringValue == "X")
                return XPlus + XMinus;

            if (stringValue == "Y")
                return YPlus + YMinus;

            if (stringValue == "Z")
                return ZPlus + ZMinus;

            if (stringValue == "XY")
                return XPlus + YPlus + XMinus + YMinus;

            if (stringValue == "XZ")
                return XPlus + ZPlus + XMinus + ZMinus;

            if (stringValue == "YZ")
                return YPlus + ZPlus + YMinus + ZMinus;

            if (stringValue == "XYZ")
                return XPlus + YPlus + ZPlus + XMinus + YMinus + ZMinus;

            throw std::invalid_argument("The string " + stringValue + " is not a valid MULTREGT direction value");
        }


    }
}
