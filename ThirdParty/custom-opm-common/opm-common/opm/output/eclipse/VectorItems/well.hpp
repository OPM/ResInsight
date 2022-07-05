/*
  Copyright (c) 2018 Equinor ASA

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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_WELL_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_WELL_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {

    namespace IWell {
        enum index : std::vector<int>::size_type {
            IHead    =  0, // I-location (one-based) of well head
            JHead    =  1, // J-location (one-based) of well head
            FirstK   =  2, // Layer ID (one-based) of top/first connection
            LastK    =  3, // Layer ID (one-based) of bottom/last connection
            NConn    =  4, // Number of active cells connected to well
            Group    =  5, // Index (one-based) of well's current group
            WType    =  6, // Well type (producer vs. injector)
            ActWCtrl =  7, // Well's active target control mode (constraint).

            item9    =  8, // Unknown
            Status   = 10, // Well status
            VFPTab   = 11, // ID (one-based) of well's current VFP table.

            EconWorkoverProcedure = 14, // Economic limit workover procedure (WECON(7)).
                                        //   0 => No action taken ("NONE"),
                                        //   1 => Close worst-offending connection ("CON"),
                                        //   2 => Close worst-offending connection and
                                        //        all other connections below this ("+CON"),
                                        //   3 => Shut/stop well ("WELL"),
                                        //   6 => Plug well ("PLUG").

            PreferredPhase = 15, // Well's preferred phase (from WELSPECS)

            item18 = 17, // Unknown

            XFlow  = 22, // Whether or not well supports cross flow.
                         //   0 => Cross flow NOT supported
                         //   1 => Cross flow IS supported.

            WGrupConControllable = 24, // Well controllable by group (WGRUPCON(2))
                                       // -1 => YES, 0 => NO

            EconLimitEndRun = 29,      // Whether or not to end simulation run at next report time
                                       // if well is shut or stopped for any reason (WECON(8)).
                                       // 0 => No, 1 => Yes.

            item32 = 31,               // Unkown

            WGrupConGRPhase = 32,      // Phase to which well's guiderate applies (WGRUPCON(4))
                                       //   0 => None/defaulted,
                                       //   1 => Oil,
                                       //   2 => Water,
                                       //   3 => Gas,
                                       //   4 => Liquid,
                                       //   5 => Surface flow rate of injecting phase (injectors only),
                                       //   6 => Reservoir fluid volume rate.

            WTestCloseReason = 39,     // Dynamic reason for closing a well
                                       //   0 => Flowing or manually SHUT/STOPped
                                       //   3 => Well closed for Physical reasons (P)
                                       //   5 => Well closed for Economic reasons (E)
                                       //   6 => Well closed for Group/Field reasons (G)
                                       //   9 => Well closed for THP design limit (D)

            WTestConfigReason = 40,    // Which checks to perform when deciding to
                                       // close a well in WTEST (WTEST(3)).
                                       //
                                       // Product of prime factors representing individual reasons.
                                       //    2 => Physical (P),
                                       //    3 => Economic (E),
                                       //    5 => Group or Field (G),
                                       //    7 => THP design limit (D),
                                       //   11 => Test connections individually (C).
                                       //
                                       // Example: PEG   = 2 * 3 * 5          =   30
                                       //          PEGDC = 2 * 3 * 5 * 7 * 11 = 2310

            WTestRemaining = 45,       // Remaining number of times well can be tested (WTEST(4)).
                                       //   0 => Unlimited number of tests
                                       //  >0 => Well can be tested at most this many more times

            item48 = 47, // Unknown

            HistReqWCtrl = 49, // Well's requested control mode from
                               // simulation deck (WCONHIST, WCONINJH)

            LiftOpt = 53, // Well's lift gas injection to be calculated by optimisation or not

            THPLookupVFPTable = 54, // How to look up VFP table values for THP controlled wells
                                    // (WVFPEXP(2)).  0 => Implicit, 1 => Explicit.

            EconLimitQuantity = 55, // Quantity to which well's economic limits apply (WECON(10))
                                    //   0 => Well's flow rate ("RATE")
                                    //   1 => Well's potential flow rates ("POTN")

            EconWorkoverProcedure_2 = 66, // Secondary economic limit workover procedure (WECON(12)).
                                          // Usually just a copy of EconWorkoverProcedure.

            MsWID  = 70, // Multisegment well ID
                         //   Value 0 for regular wells
                         //   Value 1..#MS wells for MS wells
            NWseg  = 71, // Number of well segments
                         //   Value 0 for regular wells
                         //   Value #segments for MS wells

            MSW_PlossMod  = 81, // index for Pressure loss model for well segments
                         //   ih value for this index is:
                         //     =  0 for regular wells
                         //     = 0 for MSW wells and HFA (WELSEGS item 6)
                         //     = 1 for MSW wells and HF- (WELSEGS item 6)
                         //     = 2 for MSW wells and H-- (WELSEGS item 6)

            MSW_MulPhaseMod  = 85, // index for Multiphase flow model for well segments - NOTE DF - model is not implemented yet!!
                         //   ih value for this index is:
                         //     = 0 for regular wells
                         //     = 1 for MSW wells and HO (WELSEGS item 7)
                         //     = 2 for MSW wells and DF (WELSEGS item 7)


            CloseWellIfTHPStabilised = 86, // Whether or not to close well
                                           // if operating in "stabilised"
                                           // part of VFP curve (WVFPEXP(3))
                                           // 0 => No, 1 => Yes

            PreventTHPIfUnstable = 93, // Whether or not to prevent well
                                       // changing from rate control to THP
                                       // control if constrained to operate
                                       // on unstable side of VFP curve.
                                       // WVFPEXP(4).
                                       //   0 => No,
                                       //   2 => YES1,
                                       //   3 => YES2.

            CompOrd = 98, // Well's completion ordering scheme.

            LiftOptAllocExtra = 144,
        };

        namespace Value {

            enum WellCtrlMode : int {
                WMCtlUnk = -10,  // Unknown well control mode (OPM only)
                Group    = - 1,  // Well under group control
                OilRate  =   1,  // Well controlled by oil rate
                WatRate  =   2,  // Well controlled by water rate
                GasRate  =   3,  // Well controlled by gas rate
                LiqRate  =   4,  // Well controlled by liquid rate

                ResVRate =   5,  // Well controlled by
                                 // reservoir voidage rate

                THP      =   6,  // Well controlled by
                                 // tubing head pressure target

                BHP      =   7,  // Well controlled by
                                 // bottom-hole pressure target

                CombRate =   9,  // Well controlled by linearly
                                 // combined rate target
            };

            enum CompOrder : int {
                Track = 0, // Connections ordered along
                           // well track (increasing MD)

                Depth = 1, // Connections ordered by inceasing
                           // true vertical depth.  Not really
                           // supported in OPM Flow.

                Input = 2, // Connections listed in order of
                           // appearance in simulation model's
                           // COMPDAT keyword.
            };

            enum Preferred_Phase : int {
                Oil    = 1,
                Water  = 2,
                Gas    = 3,
                Liquid = 4,
            };

            enum PLossMod : int {
                HFA = 0, // Components of pressure loss in MSW model for well (WELSEGS item 6)
                         // Hydrostatic, Friction, Acceleration

                HF_ = 1, // Hydrostatic, Friction,

                H__ = 2, // Hydrostatic
            };

            /*enum MPMod : int {
                HO = 1, // Multiphase flow model for MSW well
                         // Homogeneous flow

                DF = 2, // Drift flux model
            };*/

            enum Status : int {
                Shut = -1000,
                Stop = 0,
                Open = 1,
                Auto = 3,
            };

            namespace WGrupCon {
                enum Controllable : int {
                    Yes = -1,
                    No = 0,
                };

                enum GRPhase : int {
                    Defaulted            = 0,
                    Oil                  = 1,
                    Water                = 2,
                    Gas                  = 3,
                    Liquid               = 4,
                    SurfaceInjectionRate = 5,
                    ReservoirVolumeRate  = 6,
                };
            } // namespace WGrupCon

            namespace WVfpExp {
                enum Lookup : int {
                    Implicit = 0,
                    Explicit = 1,
                };

                enum class CloseStabilised : int {
                    No  = 0,
                    Yes = 1,
                };

                enum class PreventTHP : int {
                    No   = 0,
                    Yes1 = 2,
                    Yes2 = 3,
                };
            } // namespace WVfpExp

            namespace EconLimit {
                enum WOProcedure : int {
                    None        = 0, // NONE
                    Con         = 1, // CON
                    ConAndBelow = 2, // +CON
                    StopOrShut  = 3, // WELL
                    Plug        = 6, // PLUG
                };

                enum EndRun : int {
                    No  = 0,    // Run continues if well shut/stopped
                    Yes = 1,    // Run terminates if well shut/stopped
                };

                enum Quantity : int {
                    Rate      = 0, // Apply limits to actual flow rates ("RATE")
                    Potential = 1, // Apply limits to potential flow rates ("POTN")
                };
            } // namespace EconLimit

        } // Value
    } // IWell

    namespace SWell {
        enum index : std::vector<float>::size_type {
            OilRateTarget  = 0, // Well's current oil rate production target
            WatRateTarget  = 1, // Well's current water rate production target
            GasRateTarget  = 2, // Well's current gas rate production target
            LiqRateTarget  = 3, // Well's current liquid rate production target
            ResVRateTarget = 4, // Well's current reservoir voidate rate
                                // production target

            THPTarget      = 5, // Well's tubing head pressure target
            BHPTarget      = 6, // Well's bottom hole pressure target

            DatumDepth     = 9, // Well's reference depth for BHP
            Alq_value      = 10, // Well's artificial lift quantity

            EconLimitMinOil = 12, // Well's minimum oil production rate economic limit (WECON(2))
            EconLimitMinGas = 13, // Well's minimum gas production rate economic limit (WECON(3))
            EconLimitMaxWct = 14, // Well's maximum water cut economic limit (WECON(4))
            EconLimitMaxGor = 15, // Well's maximum gas/oil ratio economic limit (WECON(5))

            DrainageRadius = 16, // Well's drainage radius (WELSPECS(7))

            WGrupConGuideRate = 17, // Well's guide rate (WGRUPCON(3))

            EconLimitMaxWgr   = 18, // Well's maximum water/gas ratio economic limit (WECON(6))

            EfficiencyFactor1 = 24, // Well's efficiency factor (WEFAC(2))

            EfficiencyFactor2 = 31, // Well's efficiency factor (WEFAC(2), copy of EfficiencyFactor1)
            WTestInterval     = 32, // Well's WTEST interval (WTEST(2))
            HistLiqRateTarget = 33, // Well's historical/observed liquid
                                    // rate target/limit

            WTestStartupTime  = 39, // Well's WTEST startup time (WTEST(5))

            HistGasRateTarget = 54, // Well's historical/observed gas rate
                                    // target/limit

            HistBHPTarget     = 55, // Well's historical/observed bottom
                                    // hole pressure target/limit

            LOmaxRate         = 56, // Well's maximum lift gas rate
            LOweightFac       = 57, // Well's wighting factor for preferential allocation of lift gas
            LOminRate         = 67, // Well's mimimum lift gas rate

            EconLimitMaxWct_2 = 71, // Well's secondary maximum water cut economic limit (WECON(11)).

            EconLimitMinLiq   = 82, // Well's minimum liquid production rate economic limit (WECON(14)).

            WGrupConGRScaling = 84, // Guide rate scaling factor (WGRUPCON(5))

            LOincFac          = 115,

            TracerOffset      = 122, // Tracer data start at this index
        };
    } // SWell

    namespace XWell {
        enum index : std::vector<double>::size_type {
            OilPrRate  =  0, // Well's oil production rate
            WatPrRate  =  1, // Well's water production rate
            GasPrRate  =  2, // Well's gas production rate
            LiqPrRate  =  3, // Well's liquid production rate
            VoidPrRate =  4, // Well's reservoir voidage production rate
            TubHeadPr  =  5, // Well's tubing head pressure
            FlowBHP    =  6, // Well's flowing/producing bottom hole pressure
            WatCut     =  7, // Well's producing water cut
            GORatio    =  8, // Well's producing gas/oil ratio

            OilPrTotal   = 18, // Well's total cumulative oil production
            WatPrTotal   = 19, // Well's total cumulative water production
            GasPrTotal   = 20, // Well's total cumulative gas production
            VoidPrTotal  = 21, // Well's total cumulative reservoir
                               // voidage production

            OilInjTotal  = 22, // Well's total cumulative oil injection
            WatInjTotal  = 23, // Well's total cumulative water injection
            GasInjTotal  = 24, // Well's total cumulative gas injection
            VoidInjTotal = 25, // Well's total cumulative reservoir volume injection

            GasFVF      = 34,  // Well's producing gas formation volume factor.

            item36      = 35,   // Unknown
            item37      = 36,   // Unknown
            item38      = 37,   // Unknown

            BHPTarget   = 41,   // Well's current BHP Target/Limit

            PrimGuideRate   = 48, // Well's "primary" guide rate (oil for producers,
                                  // preferred phase for injectors)
            WatPrGuideRate  = 49, // Well's producer guide rate for water
            GasPrGuideRate  = 50, // Well's producer guide rate for gas
            VoidPrGuideRate = 68, // Well's producer guide rate for reservoir voidag volume

            OilPrTotalSolution = 73, // Well's total cumulative oil production in solution
            GasPrTotalSolution = 74, // Well's total cumulative gas production in solution

            HistOilPrTotal  = 75, // Well's total cumulative oil production
                                  // (observed/historical rates)
            HistWatPrTotal  = 76, // Well's total cumulative water
                                  // production (observed/historical rates)
            HistGasPrTotal  = 77, // Well's total cumulative gas production
                                  // (observed(historical rates)

            HistWatInjTotal = 81, // Well's total cumulative water injection
                                  // (observed/historical rates)
            HistGasInjTotal = 82, // Well's total cumulative gas injection
                                  // (observed/historical rates)

            PrimGuideRate_2   = 91, // Second copy of well's primary guide rate.
                                    // Not fully characterised.
            WatPrGuideRate_2  = 92, // Second copy of well's producer guide rate for water.
                                    // Not fully characterised.
            GasPrGuideRate_2  = 93, // Second copy of well's producer guide rate for gas
                                    // Not fully characterised.
            VoidPrGuideRate_2 = 94, // Second copy of well's producer guide rate for reservoir voidage
                                    // Not fully characterised.

            WatVoidPrRate = 122, // Well's voidage production rate
            GasVoidPrRate = 123, // Well's voidage production rate

            TracerOffset  = 130, // Tracer data start at this index
        };
    } // XWell

    namespace ZWell {
        enum index : std::vector<const char*>::size_type {
            WellName = 0, // Well name
            ActionX  = 2, // ActionX name
        };
    } // ZWell
}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_WELL_HPP
