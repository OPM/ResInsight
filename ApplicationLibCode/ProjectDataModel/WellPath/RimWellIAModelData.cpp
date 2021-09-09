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

#include "RimWellIAModelData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAModelData::RimWellIAModelData()
{
    m_displacements.resize( 8 );
    m_casingPressure    = 0.0;
    m_formationPressure = 0.0;
    m_temperature       = 0.0;
    m_dayoffset         = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIAModelData::~RimWellIAModelData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellIAModelData::displacements() const
{
    return m_displacements;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIAModelData::setDisplacement( int cornerIndex, cvf::Vec3d displacement )
{
    size_t ci = cornerIndex;

    if ( ( cornerIndex >= 0 ) && ( ci < m_displacements.size() ) ) m_displacements[ci] = displacement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIAModelData::setCasingPressure( double pressure )
{
    m_casingPressure = pressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAModelData::casingPressure() const
{
    return m_casingPressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIAModelData::setFormationPressure( double pressure )
{
    m_formationPressure = pressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAModelData::formationPressure() const
{
    return m_formationPressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIAModelData::setTemperature( double temp )
{
    m_temperature = temp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIAModelData::temperature() const
{
    return m_temperature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellIAModelData::dayOffset() const
{
    return m_dayoffset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIAModelData::setDayOffset( int days )
{
    m_dayoffset = days;
}
