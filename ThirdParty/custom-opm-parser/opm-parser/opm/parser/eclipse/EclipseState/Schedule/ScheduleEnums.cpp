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

#include <string>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>

namespace Opm {

    namespace WellCompletion {

        std::string
        DirectionEnum2String(const DirectionEnum enumValue)
        {
            std::string stringValue;

            switch (enumValue) {
            case DirectionEnum::X:
                stringValue = "X";
                break;

            case DirectionEnum::Y:
                stringValue = "Y";
                break;

            case DirectionEnum::Z:
                stringValue = "Z";
                break;
            }

            return stringValue;
        }

        DirectionEnum
        DirectionEnumFromString(const std::string& stringValue)
        {
            std::string s(stringValue);
            boost::algorithm::trim(s);

            DirectionEnum direction;

            if      (s == "X") { direction = DirectionEnum::X; }
            else if (s == "Y") { direction = DirectionEnum::Y; }
            else if (s == "Z") { direction = DirectionEnum::Z; }
            else {
                std::string msg = "Unsupported completion direction " + s;
                throw std::invalid_argument(msg);
            }

            return direction;
        }


        const std::string StateEnum2String( StateEnum enumValue ) {
            switch( enumValue ) {
            case OPEN:
                return "OPEN";
            case AUTO:
                return "AUTO";
            case SHUT:
                return "SHUT";
            default:
                throw std::invalid_argument("Unhandled enum value");
            }
        }


        StateEnum StateEnumFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);
            if (stringValue == "OPEN")
                return OPEN;
            else if (stringValue == "SHUT")
                return SHUT;
            else if (stringValue == "STOP")
                return SHUT;
            else if (stringValue == "AUTO")
                return AUTO;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }


        const std::string CompletionOrderEnum2String( CompletionOrderEnum enumValue ) {
            switch( enumValue ) {
            case DEPTH:
                return "DEPTH";
            case INPUT:
                return "INPUT";
            case TRACK:
                return "TRACK";
            default:
                throw std::invalid_argument("Unhandled enum value");
            }

        }

        CompletionOrderEnum CompletionOrderEnumFromString(const std::string& comporderStringValue) {
            std::string stringValue(comporderStringValue);
            boost::algorithm::trim(stringValue);
            if (stringValue == "DEPTH")
                return DEPTH;
            else if (stringValue == "INPUT")
                return INPUT;
            else if (stringValue == "TRACK")
                return TRACK;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }


    /*****************************************************************/

    namespace GroupInjection {

        const std::string ControlEnum2String( ControlEnum enumValue ) {
            switch( enumValue ) {
            case NONE:
                return "NONE";
            case RATE:
                return "RATE";
            case RESV:
                return "RESV";
            case REIN:
                return "REIN";
            case VREP:
                return "VREP";
            case FLD:
                return "FLD";
            default:
                throw std::invalid_argument("Unhandled enum value");
            }
        }


        ControlEnum ControlEnumFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "NONE")
                return NONE;
            else if (stringValue == "RATE")
                return RATE;
            else if (stringValue == "RESV")
                return RESV;
            else if (stringValue == "REIN")
                return REIN;
            else if (stringValue == "VREP")
                return VREP;
            else if (stringValue == "FLD")
                return FLD;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }

    }

    /*****************************************************************/

    namespace GroupProduction {

        const std::string ControlEnum2String( ControlEnum enumValue ) {
            switch( enumValue ) {
            case NONE:
                return "NONE";
            case ORAT:
                return "ORAT";
            case WRAT:
                return "WRAT";
            case GRAT:
                return "GRAT";
            case LRAT:
                return "LRAT";
            case CRAT:
                return "CRAT";
            case RESV:
                return "RESV";
            case PRBL:
                return "PRBL";
            default:
                throw std::invalid_argument("Unhandled enum value");
            }
        }


        ControlEnum ControlEnumFromString( const std::string& stringValue ) {
            if (stringValue == "NONE")
                return NONE;
            else if (stringValue == "ORAT")
                return ORAT;
            else if (stringValue == "WRAT")
                return WRAT;
            else if (stringValue == "GRAT")
                return GRAT;
            else if (stringValue == "LRAT")
                return LRAT;
            else if (stringValue == "CRAT")
                return CRAT;
            else if (stringValue == "RESV")
                return RESV;
            else if (stringValue == "PRBL")
                return PRBL;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }

    /*****************************************************************/
    namespace GroupProductionExceedLimit {

        const std::string ActionEnum2String( ActionEnum enumValue ) {
            switch(enumValue) {
            case NONE:
                return "NONE";
            case CON:
                return "CON";
            case CON_PLUS:
                return "+CON";
            case WELL:
                return "WELL";
            case PLUG:
                return "PLUG";
            case RATE:
                return "RATE";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }


        ActionEnum ActionEnumFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "NONE")
                return NONE;
            else if (stringValue == "CON")
                return CON;
            else if (stringValue == "+CON")
                return CON_PLUS;
            else if (stringValue == "WELL")
                return WELL;
            else if (stringValue == "PLUG")
                return PLUG;
            else if (stringValue == "RATE")
                return RATE;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }

    }

    /*****************************************************************/

    namespace Phase {
        const std::string PhaseEnum2String( PhaseEnum enumValue ) {
            switch( enumValue ) {
            case OIL:
                return "OIL";
            case GAS:
                return "GAS";
            case WATER:
                return "WATER";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }

        PhaseEnum PhaseEnumFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "OIL")
                return OIL;
            else if (stringValue == "WATER")
                return WATER;
            else if (stringValue == "WAT")
                return WATER;
            else if (stringValue == "GAS")
                return GAS;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }

    /*****************************************************************/

    namespace WellProducer {

        const std::string ControlMode2String( ControlModeEnum enumValue ) {
            switch( enumValue ) {
            case ORAT:
                return "ORAT";
            case WRAT:
                return "WRAT";
            case GRAT:
                return "GRAT";
            case LRAT:
                return "LRAT";
            case CRAT:
                return "CRAT";
            case RESV:
                return "RESV";
            case BHP:
                return "BHP";
            case THP:
                return "THP";
            case GRUP:
                return "GRUP";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }

        ControlModeEnum ControlModeFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "ORAT")
                return ORAT;
            else if (stringValue == "WRAT")
                return WRAT;
            else if (stringValue == "GRAT")
                return GRAT;
            else if (stringValue == "LRAT")
                return LRAT;
            else if (stringValue == "CRAT")
                return CRAT;
            else if (stringValue == "RESV")
                return RESV;
            else if (stringValue == "BHP")
                return BHP;
            else if (stringValue == "THP")
                return THP;
            else if (stringValue == "GRUP")
                return GRUP;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }


    namespace WellInjector {

        const std::string Type2String( TypeEnum enumValue ) {
            switch( enumValue ) {
            case OIL:
                return "OIL";
            case GAS:
                return "GAS";
            case WATER:
                return "WATER";
            case MULTI:
                return "MULTI";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }

        TypeEnum TypeFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "OIL")
                return OIL;
            else if (stringValue == "WATER")
                return WATER;
            else if (stringValue == "WAT")
                return WATER;
            else if (stringValue == "GAS")
                return GAS;
            else if (stringValue == "MULTI")
                return MULTI;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }

        /*****************************************************************/

        const std::string ControlMode2String( ControlModeEnum enumValue ) {
            switch( enumValue ) {
            case RESV:
                return "RESV";
            case RATE:
                return "RATE";
            case BHP:
                return "BHP";
            case THP:
                return "THP";
            case GRUP:
                return "GRUP";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }

        ControlModeEnum ControlModeFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "RATE")
                return RATE;
            else if (stringValue == "RESV")
                return RESV;
            else if (stringValue == "BHP")
                return BHP;
            else if (stringValue == "THP")
                return THP;
            else if (stringValue == "GRUP")
                return GRUP;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }

    }


    namespace WellCommon {

        const std::string Status2String(StatusEnum enumValue) {
            switch( enumValue ) {
            case OPEN:
                return "OPEN";
            case SHUT:
                return "SHUT";
            case AUTO:
                return "AUTO";
            case STOP:
                return "STOP";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }


        StatusEnum StatusFromString(const std::string& origStringValue) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "OPEN")
                return OPEN;
            else if (stringValue == "SHUT")
                return SHUT;
            else if (stringValue == "STOP")
                return STOP;
            else if (stringValue == "AUTO")
                return AUTO;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }

    }

    namespace GuideRate {

        const std::string GuideRatePhaseEnum2String( GuideRatePhaseEnum enumValue ) {
            switch( enumValue ) {
            case OIL:
                return "OIL";
            case WAT:
                return "WAT";
            case GAS:
                return "GAS";
            case LIQ:
                return "LIQ";
            case COMB:
                return "COMB";
            case WGA:
                return "WGA";
            case CVAL:
                return "CVAL";
            case RAT:
                return "RAT";
            case RES:
                return "RES";
            case UNDEFINED:
                return "UNDEFINED";
            default:
                throw std::invalid_argument("unhandled enum value");
            }
        }

        GuideRatePhaseEnum GuideRatePhaseEnumFromString( const std::string& origStringValue ) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "OIL")
                return OIL;
            else if (stringValue == "WAT")
                return WAT;
            else if (stringValue == "GAS")
                return GAS;
            else if (stringValue == "LIQ")
                return LIQ;
            else if (stringValue == "COMB")
                return COMB;
            else if (stringValue == "WGA")
                return WGA;
            else if (stringValue == "CVAL")
                return CVAL;
            else if (stringValue == "RAT")
                return RAT;
            else if (stringValue == "RES")
                return RES;
            else if (stringValue == "UNDEFINED")
                return UNDEFINED;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }

    namespace RFTConnections {
        const std::string RFTEnum2String(RFTEnum enumValue) {
            switch (enumValue) {
                case YES:
                    return "YES";
                case REPT:
                    return "REPT";
                case TIMESTEP:
                    return "TIMESTEP";
                case FOPN:
                    return "FOPN";
                case NO:
                    return "NO";
                default:
                    throw std::invalid_argument("unhandled enum value");
            }
        }

        RFTEnum RFTEnumFromString(const std::string &origStringValue) {
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "YES")
                return YES;
            else if (stringValue == "REPT")
                return REPT;
            else if (stringValue == "TIMESTEP")
                return TIMESTEP;
            else if (stringValue == "FOPN")
                return FOPN;
            else if (stringValue == "NO")
                return NO;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue);
        }

    }
    namespace PLTConnections {
        const std::string PLTEnum2String(PLTEnum enumValue) {
            switch (enumValue) {
                case YES:
                    return "YES";
                case REPT:
                    return "REPT";
                case TIMESTEP:
                    return "TIMESTEP";
                case NO:
                    return "NO";
                default:
                    throw std::invalid_argument("unhandled enum value");
            }
        }

        PLTEnum PLTEnumFromString( const std::string& origStringValue){
            std::string stringValue(origStringValue);
            boost::algorithm::trim(stringValue);

            if (stringValue == "YES")
                return YES;
            else if (stringValue == "REPT")
                return REPT;
            else if (stringValue == "TIMESTEP")
                return TIMESTEP;
            else if (stringValue == "NO")
                return NO;
            else
                throw std::invalid_argument("Unknown enum state string: " + stringValue );
        }
    }

    namespace WellSegment{

        const std::string LengthDepthEnumToString(LengthDepthEnum enumValue) {
            switch (enumValue) {
                case INC:
                    return "INC";
                case ABS:
                    return "ABS";
                default:
                    throw std::invalid_argument("unhandled LengthDepthEnum value");
            }
        }

        LengthDepthEnum LengthDepthEnumFromString(const std::string& stringValue) {

            std::string trimmedString(stringValue);
            boost::algorithm::trim(trimmedString);

            if (trimmedString == "INC") {
                return INC;
            } else if (trimmedString == "ABS") {
                return ABS;
            } else {
                throw std::invalid_argument("Unknown enum string: " + trimmedString + " for LengthDepthEnum");
            }
        }

        const std::string CompPressureDropEnumToString(CompPressureDropEnum enumValue) {
            switch (enumValue) {
                case HFA:
                    return "HFA";
                case HF_:
                    return "HF-";
                case H__:
                    return "H--";
                default:
                    throw std::invalid_argument("unhandled CompPressureDropEnum value");
            }
        }

        CompPressureDropEnum CompPressureDropEnumFromString(const std::string& stringValue) {

            std::string trimmedString(stringValue);
            boost::algorithm::trim(trimmedString);

            if (trimmedString == "HFA") {
                return HFA;
            } else if (trimmedString == "HF-") {
                return HF_;
            } else if (trimmedString == "H--") {
                return H__;
            } else {
                throw std::invalid_argument("Unknown enum string: " + trimmedString + " for CompPressureDropEnum");
            }
        }

        const std::string MultiPhaseModelEnumToString(MultiPhaseModelEnum enumValue) {
            switch (enumValue) {
                case HO:
                    return "HO";
                case DF:
                    return "DF";
                default:
                    throw std::invalid_argument("unhandled MultiPhaseModelEnum value");
            }
        }

        MultiPhaseModelEnum MultiPhaseModelEnumFromString(const std::string& stringValue) {

            std::string trimmedString(stringValue);
            boost::algorithm::trim(trimmedString);

            if ((trimmedString == "HO") || (trimmedString == "H0")) {
                return HO;
            } else if (trimmedString == "DF") {
                return DF;
            } else {
                throw std::invalid_argument("Unknown enum string: " + trimmedString + " for MultiPhaseModelEnum");
            }

        }

    }


}
