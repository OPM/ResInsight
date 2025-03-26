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

#include "RigFloodingSettings.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFloodingSettings::RigFloodingSettings( FloodingType oilFloodingType,
                                          double       userDefFloodingOil,
                                          FloodingType gasFloodingType,
                                          double       userDefFloodingGas )
{
    m_gasFloodingType    = gasFloodingType;
    m_oilFloodingType    = oilFloodingType;
    m_userDefFloodingGas = userDefFloodingGas;
    m_userDefFloodingOil = userDefFloodingOil;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFloodingSettings::~RigFloodingSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFloodingSettings::FloodingType RigFloodingSettings::oilFlooding() const
{
    return m_oilFloodingType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFloodingSettings::FloodingType RigFloodingSettings::gasFlooding() const
{
    return m_gasFloodingType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFloodingSettings::oilUserDefFlooding() const
{
    return m_userDefFloodingOil;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFloodingSettings::gasUserDefFlooding() const
{
    return m_userDefFloodingGas;
}
