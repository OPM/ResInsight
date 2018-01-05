/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaRftPltCurveDefinition.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaRftPltCurveDefinition::RiaRftPltCurveDefinition(RifDataSourceForRftPlt address, const QDateTime timeStep)
{
    m_curveDefinition = std::make_pair(address, timeStep);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifDataSourceForRftPlt RiaRftPltCurveDefinition::address() const
{
    return m_curveDefinition.first;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RiaRftPltCurveDefinition::timeStep() const
{
    return m_curveDefinition.second;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaRftPltCurveDefinition::operator<(const RiaRftPltCurveDefinition& other) const
{
    return m_curveDefinition < other.m_curveDefinition;
}
