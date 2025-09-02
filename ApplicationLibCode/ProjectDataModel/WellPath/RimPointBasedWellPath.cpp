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

#include "RimPointBasedWellPath.h"

#include "Well/RigWellPath.h"
#include "Well/RigWellPathGeometryTools.h"

// Make sure to include cafPdmFieldScriptingCapabilityCvfVec3d. Include of general cafPdmFieldScriptingCapability causes unity build issues.
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimPointBasedWellPath, "PointBasedWellPath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPointBasedWellPath::RimPointBasedWellPath()
{
    CAF_PDM_InitScriptableObject( "Point-Based Well Path", ":/Well.svg", "", "PointBasedWellPath" );

    CAF_PDM_InitScriptableField( &m_trajectoryPoints, "TrajectoryPoints", std::vector<cvf::Vec3d>(), "Trajectory Points" );
    m_trajectoryPoints.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPointBasedWellPath::~RimPointBasedWellPath()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPointBasedWellPath::setTrajectoryPoints( const std::vector<cvf::Vec3d>& wellTargets )
{
    m_trajectoryPoints = wellTargets;
    createWellPathGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimPointBasedWellPath::trajectoryPoints() const
{
    return m_trajectoryPoints();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPointBasedWellPath::createWellPathGeometry()
{
    if ( m_trajectoryPoints().empty() )
    {
        return;
    }

    // Create well path geometry from the target points
    auto measuredDepths = RigWellPathGeometryTools::calculateMeasuredDepth( m_trajectoryPoints );

    auto wellPathGeometry = new RigWellPath( m_trajectoryPoints, measuredDepths );

    setWellPathGeometry( wellPathGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPointBasedWellPath::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellPath::defineUiOrdering( uiConfigName, uiOrdering );

    // Hide the well targets field from UI since this is Python-only
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPointBasedWellPath::initAfterRead()
{
    createWellPathGeometry();
}
