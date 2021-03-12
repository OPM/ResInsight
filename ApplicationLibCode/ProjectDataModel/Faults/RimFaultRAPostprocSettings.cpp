/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimFaultRAPostprocSettings.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPostprocSettings::RimFaultRAPostprocSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPostprocSettings::~RimFaultRAPostprocSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPostprocSettings::setBaseDirectory( QString baseDir )
{
    m_baseDir = baseDir + "/base_dir";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPostprocSettings::setStepsToLoad( std::list<int> steps )
{
    m_steps.clear();
    m_steps.insert( m_steps.begin(), steps.begin(), steps.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<int>& RimFaultRAPostprocSettings::stepsToLoad()
{
    return m_steps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::postprocParameterFilename() const
{
    return m_baseDir + "/post_processing.json";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::databaseDirectory() const
{
    return m_baseDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::macrisCalcCalibPath() const
{
    return m_baseDir + "/MacrisCalcCalibration.sqlite3";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::macrisCalcPath() const
{
    return m_baseDir + "/MacrisCalcResult.sqlite3";
}
