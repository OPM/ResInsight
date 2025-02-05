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

class RigFloodingSettings
{
public:
    enum class FloodingType
    {
        WATER_FLOODING,
        GAS_FLOODING,
        USER_DEFINED
    };

public:
    RigFloodingSettings( FloodingType oilFloodingType, double userDefFloodingOil, FloodingType gasFloodingType, double userDefFloodingGas );
    ~RigFloodingSettings();

    FloodingType oilFlooding() const;
    FloodingType gasFlooding() const;
    double       oilUserDefFlooding() const;
    double       gasUserDefFlooding() const;

    bool needsSogcr() const;
    bool needsSowcr() const;

protected:
    FloodingType m_oilFloodingType;
    double       m_userDefFloodingOil;

    FloodingType m_gasFloodingType;
    double       m_userDefFloodingGas;
};