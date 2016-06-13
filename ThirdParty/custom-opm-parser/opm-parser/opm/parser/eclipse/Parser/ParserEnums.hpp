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
        UNKNOWN=3
    };



    enum ParserItemSizeEnum {
        ALL = 0,
        SINGLE = 1
    };



    enum ParserValueTypeEnum {
        INT = 0,
        FLOAT = 1,
        STRING = 2,
        DOUBLE = 3
    };


    enum ParserKeywordActionEnum {
        INTERNALIZE = 0,
        IGNORE = 1,
        IGNORE_WARNING = 2,
        THROW_EXCEPTION = 3
    };

    const std::string ParserKeywordActionEnum2String(ParserKeywordActionEnum enumValue);
    const std::string ParserItemSizeEnum2String(ParserItemSizeEnum enumValue);
    const std::string ParserKeywordSizeEnum2String(ParserKeywordSizeEnum enumValue);
    const std::string ParserValueTypeEnum2String(ParserValueTypeEnum enumValue);

    ParserItemSizeEnum ParserItemSizeEnumFromString(const std::string& stringValue);
    ParserKeywordSizeEnum ParserKeywordSizeEnumFromString(const std::string& stringValue);
    ParserValueTypeEnum ParserValueTypeEnumFromString(const std::string& stringValue);
    ParserKeywordActionEnum ParserKeywordActionEnumFromString(const std::string& stringValue);
}

#endif
