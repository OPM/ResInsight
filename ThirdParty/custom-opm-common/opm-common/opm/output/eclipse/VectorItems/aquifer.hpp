/*
  Copyright (c) 2021 Equinor ASA
  Copyright (c) 2019 Equinor ASA

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

#ifndef OPM_OUTPUT_ECLIPSE_VECTOR_AQUIFER_HPP
#define OPM_OUTPUT_ECLIPSE_VECTOR_AQUIFER_HPP

#include <vector>

namespace Opm { namespace RestartIO { namespace Helpers { namespace VectorItems {

    namespace IAnalyticAquifer {
        enum index : std::vector<int>::size_type {
            NumAquiferConn = 0, // Number of active aquifer connections for this aquifer
            WatPropTable = 1,   // PVT number (ACUCT(10) or AQUFETP(7))

            CTInfluenceFunction =  9, // AQUCT(11)
            TypeRelated1 = 10,        // =1 for CT, =0 for FETP

            Unknown_1 = 11,     // Unknown item.  =1 in all cases seen thus far.
        };

        namespace Value {
            enum ModelType : int {
                Fetkovich = 0,
                CarterTracy = 1,
            };
        } // Value
    } // IAnalyticAquifer

    namespace IAnalyticAquiferConn {
        enum index : std::vector<int>::size_type {
            Index_I = 0,        // One-based I index of connecting cell
            Index_J = 1,        // One-based J index of connecting cell
            Index_K = 2,        // One-based K index of connecting cell
            ActiveIndex = 3,    // One-based columnar active index of connecting cell
            FaceDirection = 4,  // Direction of connecting face
        };

       namespace Value {
           enum FaceDirection {
               IMinus = 1, IPlus = 2, JMinus = 3, JPlus = 4, KMinus = 5, KPlus = 6,
           };
       } // Value
   } // IAnalyticAquiferConn

    namespace INumericAquifer {
        enum index : std::vector<int>::size_type {
            AquiferID = 0,      // ID of numeric aquifer
            Cell_I = 1,         // I coordinate of aquifer cell
            Cell_J = 2,         // J coordinate of aquifer cell
            Cell_K = 3,         // K coordinate of aquifer cell
            PVTTableID = 4,     // PVT Table ID of numeric aquifer
            SatFuncID = 5,      // Saturation function ID of numeric aquifer
        };
    } // INumericAquifer

    namespace RNumericAquifer {
        enum index : std::vector<double>::size_type {
            Area = 0,           // Aquifer inflow area, AQUNUM(5)
            Length = 1,         // Aquifer length, AQUNUM(6)
            Porosity = 2,       // Aquifer porosity, AQUNUM(7)
            Permeability = 3,   // Aquifer permeability, AQUNUM(8)
            Depth = 4,          // Aquifer depth, AQUNUM(9)
            Pressure = 5,       // Aquifer pressure, AQUNUM(10)

            Unknown_1 = 6,      // Unknown item, = 1.0
            Unknown_2 = 7,      // Unknown item, = 1.0
            Unknown_3 = 8,      // Unknown item, = 1.0

            PoreVolume = 9,     // Total aquifer pore-volume (= Area * Length * Porosity)

            FlowRate = 10,      // Aquifer inflow rate (ANQR:N)
            ProdVolume = 11,    // Total liquid volume produced from aquifer (AQNT:N)
            DynPressure = 12,   // Dynamic aquifer pressure (ANQP:N)
        };
    } // RNumericAquifer

    namespace SAnalyticAquifer {
        enum index : std::vector<float>::size_type {
            Compressibility = 0, // Total aquifer compressibility (AQUCT(6), AQUFETP(5))

            FetInitVol = 1,     // Initial aquifer volume (AQUFETP(4))
            FetProdIndex = 2,   // Aquifer productivity index (AQUFETP(6))
            FetTimeConstant = 3, // Fetkovich Aquifer time constant (Compressibility * InitVol / ProdIndex)

            CTRadius = 1,       // CT aquifer external radius (AQUCT(7))
            CTPermeability = 2, // CT aquifer permeability (AQUCT(4))
            CTPorosity = 3,     // CT aquifer porosity (AQUCT(5))

            InitPressure = 4,   // Initial aquifer pressure (AQUCT(3), AQUFETP(3))
            DatumDepth = 5,     // Aquifer datum depth (AQUCT(2), AQUFETP(2))

            CTThickness = 6,    // CT aquifer thickness (AQUCT(8))
            CTAngle = 7,        // CT aquifer angle of influence (AQUCT(9) / 360.0)
            CTWatMassDensity = 8,    // Water density at reservoir conditions
            CTWatViscosity = 9,      // Water viscosity at reservoir conditions
        };
    } // SAnalyticAquifer

    namespace SAnalyticAquiferConn {
        enum index : std::vector<float>::size_type {
            InfluxFraction = 0, // Connection's fraction of total aquifer influx coefficient
            FaceAreaToInfluxCoeff = 1, // Connection's effective face area divided by aquifer's total influx coefficient
        };
    } // SAnalyticAquiferConn

    namespace XAnalyticAquifer {
        enum index : std::vector<double>::size_type {
            FlowRate = 0,         // Aquifer rate (AAQR:N)
            Pressure = 1,         // Dynamic aquifer pressure (AAQP:N)
            ProdVolume = 2,       // Liquid volume produced from aquifer (into reservoir, AAQT:N)
            TotalInfluxCoeff = 3, // Total aquifer influx coefficient across all aquifer connections

            CTRecipTimeConst = 4, // Reciprocal time constant for CT aquifer
            CTInfluxConstant = 5, // Influx constant "beta" for CT aquifer

            CTDimensionLessTime = 8, // Dimensionless time for CT aquifer (AAQTD:N)
            CTDimensionLessPressure = 9, // Dimensionless pressure for CT aquifer (AAQPD:N)
        };
    } // XAnalyticAquifer

}}}} // Opm::RestartIO::Helpers::VectorItems

#endif // OPM_OUTPUT_ECLIPSE_VECTOR_AQUIFER_HPP
