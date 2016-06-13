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

#include <sstream>
#include <stdexcept>
#include <opm/common/OpmLog/LogUtil.hpp>


namespace Opm {

namespace Log {

    bool isPower2(int64_t x) {
        return ((x != 0) && !(x & (x - 1)));
    }



    std::string fileMessage(const std::string& filename , int line , const std::string& message) {
        std::ostringstream oss;

        oss << filename << ":" << line << ": " << message;

        return oss.str();
    }

    std::string fileMessage(int64_t messageType , const std::string& filename , int line , const std::string& message) {
        return fileMessage( filename , line , prefixMessage( messageType , message ));
    }


    std::string prefixMessage(int64_t messageType, const std::string& message) {
        std::string prefix;
        switch (messageType) {
        case MessageType::Debug:
            prefix = "debug";
            break;
        case MessageType::Info:
            prefix = "info";
            break;
        case MessageType::Warning:
            prefix = "warning";
            break;
        case MessageType::Error:
            prefix = "error";
            break;
        case MessageType::Problem:
            prefix = "problem";
            break;
        case MessageType::Bug:
            prefix = "bug";
            break;
        default:
            throw std::invalid_argument("Unhandled messagetype");
        }

        return prefix + ": " + message;
    }
}
}
