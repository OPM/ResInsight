/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <optional>
#include <string>

// clang-format off


// The structures below represent data from various MSW tables
// The variable names correspond to the table names in Opm::ParserKeywords
// If a parameter has a default defined, std::optional is used
// 
// Example file path:
// custom-opm-common/generated-opm-common/include/opm/input/eclipse/Parser/ParserKeywords/W.hpp

//==================================================================================================
/// Header structure for WELSEGS table (well-level information)
//==================================================================================================
struct WelsegsHeader
{
    // type                     variableName;       // Opm::ParserKeywords::WELSEGS::...

    std::string                 well;               // WELNAME
    double                      topDepth;           // TOPDEP
    double                      topLength;          // TOPLEN
    std::optional<double>       wellboreVolume;     // WBORVOL
    std::string                 infoType;           // TUBOPT
    std::optional<std::string>  pressureComponents; // PRESOPT
    std::optional<std::string>  flowModel;          // FLOWOPT
};

//==================================================================================================
/// Row structure for WELSEGS table segment data
//==================================================================================================
struct WelsegsRow
{
    // type               variableName; // Opm::ParserKeywords::WELSEGS::...

    int                   segment1;     // ISEG1
    int                   segment2;     // ISEG2
    int                   branch;       // IBRANCH
    int                   joinSegment;  // ISEG3
    double                length;       // LENGTH
    double                depth;        // DEPTH
    std::optional<double> diameter;     // ID
    std::optional<double> roughness;    // EPSILON
    
    std::string           description;
};

//==================================================================================================
/// Row structure for COMPSEGS table data
//==================================================================================================
struct CompsegsRow
{
    // type               variableName;         // Opm::ParserKeywords::COMPSEGS::...

    size_t                      i;              // I
    size_t                      j;              // J
    size_t                      k;              // K
    int                         branch;         // IBRANCH
    double                      distanceStart;  // LENGTH1
    double                      distanceEnd;    // LENGTH2

    std::string gridName; // Empty for main grid, populated for LGR data

    bool isMainGrid() const { return gridName.empty(); }
    bool isLgrGrid() const { return !gridName.empty(); }
};

//==================================================================================================
/// Row structure for WSEGVALV table data
//==================================================================================================
struct WsegvalvRow
{
    // type                     variableName;   // Opm::ParserKeywords::WSEGVALV::...

    std::string                 well;           // WELNAME
    int                         segmentNumber;  // ISEG1
    double                      cv;             // ICDCV
    double                      area;           // AREARESET
    std::optional<double>       extraLength;    // SEGLEN
    std::optional<double>       pipeD;          // ID
    std::optional<double>       roughness;      // EPSILON
    std::optional<double>       pipeA;          // AREAPIPE
    std::optional<std::string>  status;         // STATUS
    std::optional<double>       maxA;           // AREAMAX
};

//==================================================================================================
/// Row structure for WSEGAICD table data
//==================================================================================================
struct WsegaicdRow
{
    // type                     variableName;       // Opm::ParserKeywords::WSEGAICD::...

    std::string                 well;               //  1 WELNAME
    int                         segment1;           //  2 ISEG1
    int                         segment2;           //  3 ISEG2
    double                      strength;           //  4 ICDSTREN
    std::optional<double>       length;             //  5 ICDLEN
    std::optional<double>       densityCali;        //  6 CALDEN
    std::optional<double>       viscosityCali;      //  7 CALVISC
    std::optional<double>       criticalValue;      //  8 EMLCRT
    std::optional<double>       widthTrans;         //  9 EMLTRANS
    std::optional<double>       maxViscRatio;       // 10 EMLMAX
    std::optional<int>          methodScalingFactor;// 11 NSCALFAC
    double                      maxAbsRate;         // 12 CALRATE
    double                      flowRateExponent;   // 13 RATEXP
    double                      viscExponent;       // 14 VISCEXP
    std::optional<std::string>  status;             // 15 STATUS
    std::optional<double>       oilFlowFraction;    // 16 A1
    std::optional<double>       waterFlowFraction;  // 17 A2
    std::optional<double>       gasFlowFraction;    // 18 A3
    std::optional<double>       oilViscFraction;    // 19 B1
    std::optional<double>       waterViscFraction;  // 20 B2
    std::optional<double>       gasViscFraction;    // 21 B3

    std::string                 description;
};
