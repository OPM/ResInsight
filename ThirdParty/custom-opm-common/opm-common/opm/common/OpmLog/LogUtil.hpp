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

#ifndef OPM_LOG_UTIL_HPP
#define OPM_LOG_UTIL_HPP

#include <cstdint>
#include <string>

namespace Opm {

class KeywordLocation;

namespace Log {
    namespace MessageType {
        const int64_t Debug     =  1;   /* Excessive information */
        const int64_t Note      =  2;  /* Information that should only go into print file.*/
        const int64_t Info      =  4;   /* Normal status information */
        const int64_t Warning   =  8;   /* Input anomaly - possible error */
        const int64_t Error     = 16;   /* Error in the input data - should probably exit. */
        const int64_t Problem   = 32;   /* Calculation problems - e.g. convergence failure. */
        const int64_t Bug       = 64;   /* An inconsistent state has been encountered in the simulator - should probably exit. */
    }

    const int64_t DefaultMessageTypes = MessageType::Debug + MessageType::Note + MessageType::Info + MessageType::Warning + MessageType::Error + MessageType::Problem + MessageType::Bug;
    const int64_t NoDebugMessageTypes = MessageType::Info + MessageType::Note + MessageType::Warning + MessageType::Error + MessageType::Problem + MessageType::Bug;
    const int64_t StdoutMessageTypes = MessageType::Info + MessageType::Warning + MessageType::Error + MessageType::Problem + MessageType::Bug;

    /// Terminal codes for ANSI/vt100 compatible terminals.
    /// See for example http://ascii-table.com/ansi-escape-sequences.php
    namespace AnsiTerminalColors {
        const std::string none = "\033[0m";
        const std::string red = "\033[31m";
        const std::string red_strong = "\033[31;1m";
        const std::string yellow = "\033[33m";
        const std::string yellow_strong = "\033[33;1m";
        const std::string blue = "\033[34m";
        const std::string blue_strong = "\033[34;1m";
        const std::string magenta = "\033[35m";
        const std::string magenta_strong = "\033[35;1m";
        const std::string default_color = "\033[39m";
    }


    bool isPower2(int64_t x);
    std::string fileMessage(const KeywordLocation& location, const std::string& msg);
    std::string fileMessage(int64_t messageType , const KeywordLocation& location , const std::string& msg);
    std::string prefixMessage(int64_t messageType , const std::string& msg);
    std::string colorCodeMessage(int64_t messageType , const std::string& msg);

}
}

#endif
