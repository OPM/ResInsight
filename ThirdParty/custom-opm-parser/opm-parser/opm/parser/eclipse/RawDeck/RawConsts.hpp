/*
  Copyright 2013 Statoil ASA.

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

#ifndef RAWCONSTS_HPP
#define	RAWCONSTS_HPP

#include <string>

namespace Opm {

    /// Consts used in the non semantic, raw parsing of the eclipse file
    namespace RawConsts {
        const char slash = '/';
        const char quote = '\'';
        const std::string separators = "\t ";
        const std::string include = "INCLUDE";
        const std::string end = "END";
        const std::string endinclude = "ENDINC";
        const std::string paths = "PATHS";
        const unsigned int maxKeywordLength = 8;

        static inline bool is_separator( char ch ) {
            return ' ' == ch || '\n' == ch || '\t' == ch || '\r' == ch;
        }

        static inline bool is_quote( char ch ) {
            return ch == quote || ch == '"';
        }
    }
}


#endif	/* RAWCONSTS_HPP */

