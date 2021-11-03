/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "cvfVector3.h"

#include <QString>
#include <map>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RimWellIAModelData
{
public:
    RimWellIAModelData();
    ~RimWellIAModelData();

    std::vector<cvf::Vec3d> displacements() const;
    void                    setDisplacement( int cornerIndex, cvf::Vec3d displacement );

    void   setCasingPressure( double pressure );
    double casingPressure() const;

    void   setFormationPressure( double pressure );
    double formationPressure() const;

    void   setTemperature( double temp );
    double temperature() const;

    int  dayOffset() const;
    void setDayOffset( int days );

private:
    std::vector<cvf::Vec3d> m_displacements;
    double                  m_casingPressure;
    double                  m_formationPressure;
    double                  m_temperature;
    int                     m_dayoffset;
};
