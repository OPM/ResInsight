/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimCameraPosition.h"

#include "RimEclipseCase.h"

CAF_PDM_SOURCE_INIT( RimCameraPosition, "RimCameraPosition" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCameraPosition::RimCameraPosition()
{
    CAF_PDM_InitObject( "Camera Position for Case" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );

    CAF_PDM_InitField( &m_cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "" );
    m_cameraPosition.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_cameraPointOfInterest, "CameraPointOfInterest", cvf::Vec3d::ZERO, "" );
    m_cameraPointOfInterest.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimCameraPosition::eclipseCase() const
{
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCameraPosition::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Mat4d RimCameraPosition::cameraPosition() const
{
    return m_cameraPosition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCameraPosition::setCameraPosition( const cvf::Mat4d& cameraPosition )
{
    m_cameraPosition = cameraPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimCameraPosition::cameraPointOfInterest() const
{
    return m_cameraPointOfInterest();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCameraPosition::setCameraPointOfInterest( const cvf::Vec3d& cameraPointOfInterest )
{
    m_cameraPointOfInterest = cameraPointOfInterest;
}
