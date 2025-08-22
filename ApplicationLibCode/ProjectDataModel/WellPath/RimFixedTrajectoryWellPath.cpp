/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimFixedTrajectoryWellPath.h"

#include "Well/RigWellPath.h"

// Make sure to include cafPdmFieldScriptingCapabilityCvfVec3d. Include of general cafPdmFieldScriptingCapability causes unity build issues.
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimFixedTrajectoryWellPath, "FixedTrajectoryWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFixedTrajectoryWellPath::RimFixedTrajectoryWellPath()
{
    CAF_PDM_InitScriptableObject( "Fixed Trajectory Well Path", ":/Well.svg", "", "FixedTrajectoryWellPath" );

    CAF_PDM_InitScriptableField( &m_trajectoryPoints, "TrajectoryPoints", std::vector<cvf::Vec3d>(), "Trajectory Points" );
    m_trajectoryPoints.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFixedTrajectoryWellPath::~RimFixedTrajectoryWellPath()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFixedTrajectoryWellPath::setTrajectoryPoints( const std::vector<cvf::Vec3d>& wellTargets )
{
    m_trajectoryPoints = wellTargets;
    createWellPathGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimFixedTrajectoryWellPath::trajectoryPoints() const
{
    return m_trajectoryPoints();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFixedTrajectoryWellPath::createWellPathGeometry()
{
    if ( m_trajectoryPoints().empty() )
    {
        return;
    }

    // Create well path geometry from the target points
    std::vector<cvf::Vec3d> wellPathPoints = m_trajectoryPoints();
    std::vector<double>     measuredDepths;

    // Calculate measured depths along the path
    double totalMD = 0.0;
    measuredDepths.push_back( totalMD );

    for ( size_t i = 1; i < wellPathPoints.size(); ++i )
    {
        cvf::Vec3d segmentVector = wellPathPoints[i] - wellPathPoints[i - 1];
        double     segmentLength = segmentVector.length();
        totalMD += segmentLength;
        measuredDepths.push_back( totalMD );
    }

    auto wellPathGeometry = new RigWellPath( wellPathPoints, measuredDepths );

    setWellPathGeometry( wellPathGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFixedTrajectoryWellPath::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellPath::defineUiOrdering( uiConfigName, uiOrdering );

    // Hide the well targets field from UI since this is Python-only
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFixedTrajectoryWellPath::initAfterRead()
{
    createWellPathGeometry();
}
