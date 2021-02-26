/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswExportInfo.h"

#include "RicMswBranch.h"
#include "RicMswCompletions.h"

#include "RimMswCompletionParameters.h"
#include "RimWellPath.h"

#include "RigWellPath.h"

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo::RicMswExportInfo( const RimWellPath*            wellPath,
                                    RiaDefines::EclipseUnitSystem unitSystem,
                                    double                        initialMD,
                                    const QString&                lengthAndDepthText,
                                    const QString&                pressureDropText )
    : m_unitSystem( unitSystem )
    , m_topWellBoreVolume( RicMswExportInfo::defaultDoubleValue() )
    , m_linerDiameter( RimMswCompletionParameters::defaultLinerDiameter( unitSystem ) )
    , m_roughnessFactor( RimMswCompletionParameters::defaultRoughnessFactor( unitSystem ) )
    , m_lengthAndDepthText( lengthAndDepthText )
    , m_pressureDropText( pressureDropText )
    , m_hasSubGridIntersections( false )
    , m_mainBoreBranch(
          std::make_unique<RicMswBranch>( "Main Stem",
                                          wellPath,
                                          initialMD,
                                          -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath( initialMD ).z() ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setLinerDiameter( double linerDiameter )
{
    m_linerDiameter = linerDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setRoughnessFactor( double roughnessFactor )
{
    m_roughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setHasSubGridIntersections( bool subGridIntersections )
{
    m_hasSubGridIntersections = subGridIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RicMswExportInfo::unitSystem() const
{
    return m_unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::topWellBoreVolume() const
{
    return m_topWellBoreVolume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::linerDiameter() const
{
    return m_linerDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::roughnessFactor() const
{
    return m_roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicMswExportInfo::lengthAndDepthText() const
{
    return m_lengthAndDepthText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicMswExportInfo::pressureDropText() const
{
    return m_pressureDropText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswExportInfo::hasSubGridIntersections() const
{
    return m_hasSubGridIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::defaultDoubleValue()
{
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RicMswBranch* RicMswExportInfo::mainBoreBranch() const
{
    return m_mainBoreBranch.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswBranch* RicMswExportInfo::mainBoreBranch()
{
    return m_mainBoreBranch.get();
}
