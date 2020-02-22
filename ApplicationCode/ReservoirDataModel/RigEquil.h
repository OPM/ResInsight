/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include <QString>

//--------------------------------------------------------------------------------------------------
//
// Inspired by /opm-common/src/opm/parser/eclipse/EclipseState/InitConfig/Equil.cpp
//
//--------------------------------------------------------------------------------------------------
class RigEquil
{
public:
    explicit RigEquil( double datumDepth,
                       double datuDepthPressure,
                       double waterOilContactDepth,
                       double waterOilContactCapillaryPressure,
                       double gasOilContactDepth,
                       double gasOilContactCapillaryPressure,
                       bool   liveOilInitConstantRs,
                       bool   wetGasInitConstantRv,
                       int    initializationTargetAccuracy );

    double datumDepth() const;
    double datumDepthPressure() const;
    double waterOilContactDepth() const;
    double waterOilContactCapillaryPressure() const;
    double gasOilContactDepth() const;
    double gasOilContactCapillaryPressure() const;

    bool liveOilInitConstantRs() const;
    bool wetGasInitConstantRv() const;
    int  initializationTargetAccuracy() const;

    static RigEquil defaultObject();
    static RigEquil parseString( const QString& keywordData );

private:
    double datum_depth;
    double datum_depth_ps;
    double water_oil_contact_depth;
    double water_oil_contact_capillary_pressure;
    double gas_oil_contact_depth;
    double gas_oil_contact_capillary_pressure;

    bool live_oil_init_proc;
    bool wet_gas_init_proc;
    int  init_target_accuracy;
};
