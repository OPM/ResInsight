/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include <string>
#include <vector>

#include <QString>

namespace RigFlowDiagDefines
{
struct RelPermCurve
{
    enum Ident
    {
        KRW,
        KRG,
        KROW,
        KROG,
        PCOW,
        PCOG
    };
    enum EpsMode
    {
        EPS_ON,
        EPS_OFF
    };
    enum CurveSet
    {
        DRAINAGE,
        IMBIBITION
    };

    bool isWaterCurve() const { return ( ident == KRW || ident == KROW || ident == PCOW ); }
    bool isGasCurve() const { return ( ident == KRG || ident == KROG || ident == PCOG ); }

    Ident               ident;
    std::string         name;
    EpsMode             epsMode;
    CurveSet            curveSet;
    std::vector<double> saturationVals;
    std::vector<double> yVals;
};

enum PvtCurveType
{
    PVT_CT_FVF,
    PVT_CT_VISCOSITY
};

struct PvtCurve
{
    enum Phase
    {
        OIL,
        GAS
    };
    enum Ident
    {
        Unknown,
        Bo,
        Bg,
        Visc_o,
        Visc_g
    };

    Ident               ident;
    Phase               phase;
    std::vector<double> pressureVals;
    std::vector<double> yVals;
    std::vector<double> mixRatVals;
};

struct FlowCharacteristicsResultFrame
{
    using Curve = std::pair<std::vector<double>, std::vector<double>>;

    Curve  m_storageCapFlowCapCurve;
    Curve  m_dimensionlessTimeSweepEfficiencyCurve;
    double m_lorenzCoefficient = HUGE_VAL;
};

QString tofResultName();
QString cellFractionResultName();
QString maxFractionTracerResultName();
QString communicationResultName();
QString numFloodedPv();

QString flowTotalName();
QString flowOilName();
QString flowGasName();
QString flowWaterName();

QString reservoirTracerName();
QString tinyTracerGroupName();

} // namespace RigFlowDiagDefines
