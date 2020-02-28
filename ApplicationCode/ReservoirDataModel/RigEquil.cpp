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

#include "RigEquil.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEquil::RigEquil( double datumDepth,
                    double datumDepthPressure,
                    double waterOilContactDepth,
                    double waterOilContactCapillaryPressure,
                    double gasOilContactDepth,
                    double gasOilContactCapillaryPressure,
                    bool   liveOilInitConstantRs,
                    bool   wetGasInitConstantRv,
                    int    initializationTargetAccuracy )
    : datum_depth( datumDepth )
    , datum_depth_ps( datumDepthPressure )
    , water_oil_contact_depth( waterOilContactDepth )
    , water_oil_contact_capillary_pressure( waterOilContactCapillaryPressure )
    , gas_oil_contact_depth( gasOilContactDepth )
    , gas_oil_contact_capillary_pressure( gasOilContactCapillaryPressure )
    , live_oil_init_proc( liveOilInitConstantRs )
    , wet_gas_init_proc( wetGasInitConstantRv )
    , init_target_accuracy( initializationTargetAccuracy )
{
}

double RigEquil::datumDepth() const
{
    return this->datum_depth;
}

double RigEquil::datumDepthPressure() const
{
    return this->datum_depth_ps;
}

double RigEquil::waterOilContactDepth() const
{
    return this->water_oil_contact_depth;
}

double RigEquil::waterOilContactCapillaryPressure() const
{
    return this->water_oil_contact_capillary_pressure;
}

double RigEquil::gasOilContactDepth() const
{
    return this->gas_oil_contact_depth;
}

double RigEquil::gasOilContactCapillaryPressure() const
{
    return this->gas_oil_contact_capillary_pressure;
}

bool RigEquil::liveOilInitConstantRs() const
{
    return this->live_oil_init_proc;
}

bool RigEquil::wetGasInitConstantRv() const
{
    return this->wet_gas_init_proc;
}

int RigEquil::initializationTargetAccuracy() const
{
    return this->init_target_accuracy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEquil RigEquil::defaultObject()
{
    double datumDepth                       = 0.0;
    double datuDepthPressure                = 0.0;
    double waterOilContactDepth             = 0.0;
    double waterOilContactCapillaryPressure = 0.0;
    double gasOilContactDepth               = 0.0;
    double gasOilContactCapillaryPressure   = 0.0;
    int    liveOilInitConstantRs            = -1;
    int    wetGasInitConstantRv             = -1;
    int    initializationTargetAccuracy     = -5;

    return RigEquil( datumDepth,
                     datuDepthPressure,
                     waterOilContactDepth,
                     waterOilContactCapillaryPressure,
                     gasOilContactDepth,
                     gasOilContactCapillaryPressure,
                     liveOilInitConstantRs,
                     wetGasInitConstantRv,
                     initializationTargetAccuracy );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEquil RigEquil::parseString( const QString& keywordData )
{
    double datumDepth                       = 0.0;
    double datuDepthPressure                = 0.0;
    double waterOilContactDepth             = 0.0;
    double waterOilContactCapillaryPressure = 0.0;
    double gasOilContactDepth               = 0.0;
    double gasOilContactCapillaryPressure   = 0.0;
    bool   liveOilInitConstantRs            = false;
    bool   wetGasInitConstantRv             = false;
    int    initializationTargetAccuracy     = -5;

    QString line( keywordData );
    line.replace( "\t", " " );

    QStringList items = line.split( " ", QString::SkipEmptyParts );
    if ( items.size() > 0 )
    {
        datumDepth = items.at( 0 ).toDouble();
    }

    if ( items.size() > 1 )
    {
        datuDepthPressure = items.at( 1 ).toDouble();
    }
    if ( items.size() > 2 )
    {
        waterOilContactDepth = items.at( 2 ).toDouble();
    }
    if ( items.size() > 3 )
    {
        waterOilContactCapillaryPressure = items.at( 3 ).toDouble();
    }
    if ( items.size() > 4 )
    {
        gasOilContactDepth = items.at( 4 ).toDouble();
    }
    if ( items.size() > 5 )
    {
        gasOilContactCapillaryPressure = items.at( 5 ).toDouble();
    }
    if ( items.size() > 6 )
    {
        liveOilInitConstantRs = items.at( 6 ).toInt() > 0 ? true : false;
    }
    if ( items.size() > 7 )
    {
        wetGasInitConstantRv = items.at( 7 ).toInt() > 0 ? true : false;
    }
    if ( items.size() > 8 )
    {
        initializationTargetAccuracy = items.at( 8 ).toInt();
    }

    return RigEquil( datumDepth,
                     datuDepthPressure,
                     waterOilContactDepth,
                     waterOilContactCapillaryPressure,
                     gasOilContactDepth,
                     gasOilContactCapillaryPressure,
                     liveOilInitConstantRs,
                     wetGasInitConstantRv,
                     initializationTargetAccuracy );
}
