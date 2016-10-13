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

#ifndef SCHEDULE_ENUMS_H
#define SCHEDULE_ENUMS_H

#include <string>

namespace Opm {
    namespace WellCommon {

        enum StatusEnum {
            OPEN = 1,
            STOP = 2,
            SHUT = 3,
            AUTO = 4
        };

        const std::string Status2String(StatusEnum enumValue);
        StatusEnum StatusFromString(const std::string& stringValue);
    }

    namespace WellCompletion {

        enum StateEnum {
            OPEN = 1,
            SHUT = 2,
            AUTO = 3
        };


        enum DirectionEnum {
            X = 1,
            Y = 2,
            Z = 3
        };


        enum CompletionOrderEnum{
            DEPTH,
            INPUT,
            TRACK
        };

        std::string   DirectionEnum2String(const DirectionEnum enumValue);
        DirectionEnum DirectionEnumFromString(const std::string& stringValue);

        const std::string StateEnum2String( StateEnum enumValue );
        StateEnum StateEnumFromString( const std::string& stringValue );

        const std::string CompletionOrderEnum2String( CompletionOrderEnum enumValue );
        CompletionOrderEnum CompletionOrderEnumFromString(const std::string& comporderStringValue);

    }


    namespace Phase {
        enum PhaseEnum {
            OIL   = 1,
            GAS   = 2,
            WATER = 4
        };

        const std::string PhaseEnum2String( PhaseEnum enumValue );
        PhaseEnum PhaseEnumFromString( const std::string& stringValue );
    }



    namespace WellInjector {
        enum TypeEnum {
            WATER = 1,
            GAS = 2,
            OIL = 3,
            MULTI = 4
        };


        enum ControlModeEnum {
            RATE =  1 ,
            RESV =  2 ,
            BHP  =  4 ,
            THP  =  8 ,
            GRUP = 16 ,
            CMODE_UNDEFINED = 512
        };
        /*
          The elements in this enum are used as bitmasks to keep track
          of which controls are present, i.e. the 2^n structure must
          be intact.
        */



        const std::string ControlMode2String( ControlModeEnum enumValue );
        ControlModeEnum ControlModeFromString( const std::string& stringValue );

        const std::string Type2String( TypeEnum enumValue );
        TypeEnum TypeFromString( const std::string& stringValue );
    }


    namespace WellProducer {

        enum ControlModeEnum {
            NONE =     0,
            ORAT =     1,
            WRAT =     2,
            GRAT =     4,
            LRAT =     8,
            CRAT =    16,
            RESV =    32,
            BHP  =    64,
            THP  =   128,
            GRUP =   256,
            CMODE_UNDEFINED = 1024
        };

        /*
          The items BHP, THP and GRUP only apply in prediction mode:
          WCONPROD. The elements in this enum are used as bitmasks to
          keep track of which controls are present, i.e. the 2^n
          structure must be intact.The NONE item is only used in WHISTCTL
          to cancel its effect.

          The properties are initialized with the CMODE_UNDEFINED
          value, but the undefined value is never assigned apart from
          that; and it is not part of the string conversion routines.
        */


        const std::string ControlMode2String( ControlModeEnum enumValue );
        ControlModeEnum ControlModeFromString( const std::string& stringValue );
    }


    namespace GroupInjection {

          enum ControlEnum {
            NONE = 0,
            RATE = 1,
            RESV = 2,
            REIN = 3,
            VREP = 4,
            FLD  = 5
        };

        const std::string ControlEnum2String( ControlEnum enumValue );
        ControlEnum ControlEnumFromString( const std::string& stringValue );
    }


    namespace GroupProductionExceedLimit {
        enum ActionEnum {
            NONE = 0,
            CON = 1,
            CON_PLUS = 2,   // String: "+CON"
            WELL = 3,
            PLUG = 4,
            RATE = 5
        };

        const std::string ActionEnum2String( ActionEnum enumValue );
        ActionEnum ActionEnumFromString( const std::string& stringValue );
    }



    namespace GroupProduction {

        enum ControlEnum {
            NONE = 0,
            ORAT = 1,
            WRAT = 2,
            GRAT = 3,
            LRAT = 4,
            CRAT = 5,
            RESV = 6,
            PRBL = 7,
            FLD  = 8
        };

        const std::string ControlEnum2String( GroupProduction::ControlEnum enumValue );
        GroupProduction::ControlEnum ControlEnumFromString( const std::string& stringValue );
    }

    namespace GuideRate {
        enum GuideRatePhaseEnum {
            OIL = 0,
            WAT = 1,
            GAS = 2,
            LIQ = 3,
            COMB = 4,
            WGA = 5,
            CVAL = 6,
            RAT = 7,
            RES = 8,
            UNDEFINED = 9
        };
        const std::string GuideRatePhaseEnum2String( GuideRatePhaseEnum enumValue );
        GuideRatePhaseEnum GuideRatePhaseEnumFromString( const std::string& stringValue );
    }

    namespace RFTConnections {
        enum RFTEnum {
            YES = 1,
            REPT = 2,
            TIMESTEP = 3,
            FOPN = 4,
            NO = 5
        };

        const std::string RFTEnum2String(RFTEnum enumValue);

        RFTEnum RFTEnumFromString(const std::string &stringValue);
    }
    namespace PLTConnections{
        enum PLTEnum{
            YES      = 1,
            REPT     = 2,
            TIMESTEP = 3,
            NO       = 4
        };
        const std::string PLTEnum2String( PLTEnum enumValue);
        PLTEnum PLTEnumFromString( const std::string& stringValue);
    }

    enum OilVaporizationEnum{
        UNDEF = 0,
        VAPPARS = 1,
        DRSDT = 2,
        DRVDT = 3
    };


    namespace WellSegment{

        enum LengthDepthEnum {
            INC = 0,
            ABS = 1
        };
        const std::string LengthDepthEnumToString(LengthDepthEnum enumValue);
        LengthDepthEnum LengthDepthEnumFromString(const std::string& stringValue);

        enum CompPressureDropEnum {
            HFA = 0,
            HF_ = 1,
            H__ = 2
        };
        const std::string CompPressureDropEnumToString(CompPressureDropEnum enumValue);
        CompPressureDropEnum CompPressureDropEnumFromString(const std::string& stringValue);

        enum MultiPhaseModelEnum {
            HO = 0,
            DF = 1
        };
        const std::string MultiPhaseModelEnumToString(MultiPhaseModelEnum enumValue);
        MultiPhaseModelEnum MultiPhaseModelEnumFromString(const std::string& stringValue);

    }


    namespace WellEcon {
        enum WorkoverEnum {
            NONE = 0,
            CON  = 1, // CON
            CONP = 2, // +CON
            WELL = 3,
            PLUG = 4,
            // the following two only related to workover action
            // on exceeding secondary water cut limit
            LAST = 5,
            RED  = 6
        };
        const std::string WorkoverEnumToString(WorkoverEnum enumValue);
        WorkoverEnum WorkoverEnumFromString(const std::string& stringValue);

        enum QuantityLimitEnum {
            RATE = 0,
            POTN = 1
        };
        const std::string QuantityLimitEnumToString(QuantityLimitEnum enumValue);
        QuantityLimitEnum QuantityLimitEnumFromString(const std::string& stringValue);
    }


}

#endif
