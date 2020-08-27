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

#ifndef PARSER_ENUMS_H
#define PARSER_ENUMS_H

#include <string>

namespace Opm {

    enum ParserKeywordSizeEnum {
        SLASH_TERMINATED = 0,
        FIXED = 1,
        OTHER_KEYWORD_IN_DECK = 2,
        UNKNOWN = 3,
        FIXED_CODE = 4,
        DOUBLE_SLASH_TERMINATED = 5
    };


    enum ParserKeywordActionEnum {
        INTERNALIZE = 0,
        IGNORE = 1,
        IGNORE_WARNING = 2,
        THROW_EXCEPTION = 3
    };

    const std::string ParserKeywordActionEnum2String(ParserKeywordActionEnum enumValue);
    const std::string ParserKeywordSizeEnum2String(ParserKeywordSizeEnum enumValue);

    ParserKeywordSizeEnum ParserKeywordSizeEnumFromString(const std::string& stringValue);
    ParserKeywordActionEnum ParserKeywordActionEnumFromString(const std::string& stringValue);
}

#endif
