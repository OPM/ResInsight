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

#include "RicMswSegment.h"

#include "RimMswCompletionParameters.h"
#include "RimWellPath.h"

#include "RigWellPath.h"

#include <algorithm>
#include <limits>



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswExportInfo::RicMswExportInfo(const RimWellPath*              wellPath,
                                   RiaEclipseUnitTools::UnitSystem unitSystem,
                                   double                          initialMD,
                                   const QString&                  lengthAndDepthText,
                                   const QString&                  pressureDropText)
    : m_wellPath(wellPath)
    , m_initialMD(initialMD)
    , m_unitSystem(unitSystem)
    , m_topWellBoreVolume(RicMswExportInfo::defaultDoubleValue())
    , m_linerDiameter(RimMswCompletionParameters::defaultLinerDiameter(unitSystem))
    , m_roughnessFactor(RimMswCompletionParameters::defaultRoughnessFactor(unitSystem))
    , m_lengthAndDepthText(lengthAndDepthText)
    , m_pressureDropText(pressureDropText)
    , m_hasSubGridIntersections(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setTopWellBoreVolume(double topWellBoreVolume)
{
    m_topWellBoreVolume = topWellBoreVolume;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setLinerDiameter(double linerDiameter)
{
    m_linerDiameter = linerDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setRoughnessFactor(double roughnessFactor)
{
    m_roughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::setHasSubGridIntersections(bool subGridIntersections)
{
    m_hasSubGridIntersections = subGridIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::addWellSegment(std::shared_ptr<RicMswSegment> location)
{
    m_wellSegmentLocations.push_back(location);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswExportInfo::sortLocations()
{
    std::sort(m_wellSegmentLocations.begin(),
              m_wellSegmentLocations.end(),
        [](std::shared_ptr<RicMswSegment> lhs, std::shared_ptr<RicMswSegment> rhs)
    {
        return *lhs < *rhs;
    });
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPath* RicMswExportInfo::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::initialMD() const
{
    return m_initialMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswExportInfo::initialTVD() const
{
    return -m_wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(m_initialMD).z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RicMswExportInfo::unitSystem() const
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
const std::vector<std::shared_ptr<RicMswSegment>>& RicMswExportInfo::wellSegmentLocations() const
{
    return m_wellSegmentLocations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswSegment>>& RicMswExportInfo::wellSegmentLocations()
{
    return m_wellSegmentLocations;
}
