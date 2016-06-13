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

#include <stdexcept>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

namespace Opm {



    const std::string ParserItemSizeEnum2String(ParserItemSizeEnum enumValue) {
        switch (enumValue) {
        case ALL:
            return "ALL";
            break;
        case SINGLE:
            return "SINGLE";
            break;
        default:
            throw std::invalid_argument("Implementation error - should NOT be here");
        }
    }


    ParserItemSizeEnum ParserItemSizeEnumFromString(const std::string& stringValue) {
        if (stringValue == "ALL")
            return ALL;
        else if (stringValue == "SINGLE")
            return SINGLE;
        else
            throw std::invalid_argument("String: " + stringValue + " can not be converted to enum value");
    }


    /*****************************************************************/

    const std::string ParserKeywordSizeEnum2String(ParserKeywordSizeEnum enumValue) {
        switch (enumValue) {
        case SLASH_TERMINATED:
            return "SLASH_TERMINATED";
            break;
        case FIXED:
            return "FIXED";
            break;
        case OTHER_KEYWORD_IN_DECK:
            return "OTHER_KEYWORD_IN_DECK";
            break;
        case UNKNOWN:
            return "UNKNOWN";
            break;
        default:
            throw std::invalid_argument("Implementation error - should NOT be here");
        }
    }



    ParserKeywordSizeEnum ParserKeywordSizeEnumFromString(const std::string& stringValue) {
        if (stringValue == "SLASH_TERMINATED")
            return SLASH_TERMINATED;
        else if (stringValue == "FIXED")
            return FIXED;
        else if (stringValue == "OTHER_KEYWORD_IN_DECK")
                return OTHER_KEYWORD_IN_DECK;
        else if (stringValue == "UNKNOWN")
            return UNKNOWN;
        else
            throw std::invalid_argument("String: " + stringValue + " can not be converted to enum value");
    }

    /*****************************************************************/

    const std::string ParserValueTypeEnum2String(ParserValueTypeEnum enumValue) {
        switch (enumValue) {
        case INT:
            return "INT";
            break;
        case FLOAT:
            return "FLOAT";
            break;
        case STRING:
            return "STRING";
            break;
        default:
            throw std::invalid_argument("Implementation error - should NOT be here");
        }
    }


    ParserValueTypeEnum ParserValueTypeEnumFromString(const std::string& stringValue) {
        if (stringValue == "INT")
            return INT;
        else if (stringValue == "FLOAT")
            return FLOAT;
        else if (stringValue == "DOUBLE")
            return DOUBLE;
        else if (stringValue == "STRING")
            return STRING;
        else
            throw std::invalid_argument("String: " + stringValue + " can not be converted to enum value");
    }

    /*****************************************************************/

    const std::string ParserKeywordActionEnum2String(ParserKeywordActionEnum enumValue) {
        switch (enumValue) {
        case INTERNALIZE:
            return "INTERNALIZE";
            break;
        case IGNORE:
            return "IGNORE";
            break;
        case THROW_EXCEPTION:
            return "THROW_EXCEPTION";
            break;
        case IGNORE_WARNING:
            return "IGNORE_WARNING";
            break;
        default:
            throw std::invalid_argument("Implementation error - should NOT be here");
        }
    }

    ParserKeywordActionEnum ParserKeywordActionEnumFromString(const std::string& stringValue) {
        if (stringValue == "INTERNALIZE")
            return INTERNALIZE;
        else if (stringValue == "IGNORE")
            return IGNORE;
        else if (stringValue == "THROW_EXCEPTION")
            return THROW_EXCEPTION;
        else if (stringValue == "IGNORE_WARNING")
            return IGNORE_WARNING;
        else
            throw std::invalid_argument("String: " + stringValue + " can not be converted to enum value");
    }



}
