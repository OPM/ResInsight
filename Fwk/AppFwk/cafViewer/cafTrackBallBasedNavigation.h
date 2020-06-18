//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2015 Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafNavigationPolicy.h"

#include <QDateTime>

namespace cvf
{
class ManipulatorTrackball;
class Ray;
} // namespace cvf

class QMouseEvent;

namespace caf
{
class RotationSensitivityCalculator
{
public:
    RotationSensitivityCalculator()
        : m_lastPosX( 0 )
        , m_lastPosY( 0 )
        , m_lastMouseVelocityLenght( 200 )
        , m_isEnabled( false )
        , m_fixedSensitivity( std::numeric_limits<double>::infinity() )
    {
    }

    void enableAdaptiveRotationSensitivity( bool enable )
    {
        m_isEnabled        = enable;
        m_fixedSensitivity = std::numeric_limits<double>::infinity();
    }
    void enableFixedSensitivity( double senstivity )
    {
        m_isEnabled        = true;
        m_fixedSensitivity = senstivity;
    }

    void init( QMouseEvent* eventAtRotationStart );

    double calculateSensitivity( QMouseEvent* eventWhenRotating );

private:
    bool          m_isEnabled;
    int           m_lastPosX; /// Previous mouse position
    int           m_lastPosY;
    unsigned long m_lastTime;
    double        m_lastMouseVelocityLenght;
    double        m_fixedSensitivity;
};

} // End namespace caf

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// This class is a work in progress to consolidate the different navigation policies that are similar.
/// It is not yet finished. We need to extract the Pan, Rotation, ... etc. codes from the
/// special input event handlers and invoke those general methods from the event handlers instead.
///  Some of the protected variables in this class is used by the
/// derived classes, and should rather be used from the general methods in this class and thus be private
///
//--------------------------------------------------------------------------------------------------

class TrackBallBasedNavigation : public NavigationPolicy
{
public:
    TrackBallBasedNavigation();
    ~TrackBallBasedNavigation() override;
    void enableEventEating( bool enable ) { m_consumeEvents = enable; }
    void enableRotation( bool enable ) { m_isRotationEnabled = enable; }
    void enableAdaptiveRotationSensitivity( bool enable )
    {
        m_roationSensitivityCalculator.enableAdaptiveRotationSensitivity( enable );
    }
    void enableFixedSensitivity( double senstivity )
    {
        m_roationSensitivityCalculator.enableFixedSensitivity( senstivity );
    }

protected:
    // General navigation policy overrides
    void init() override;

    void       setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection ) override;
    cvf::Vec3d pointOfInterest() override;
    void       setPointOfInterest( cvf::Vec3d poi ) override;
    void       pickAndSetPointOfInterest( int winPosX, int winPosY );
    void       updatePointOfInterestDuringZoomIfNecessary( int winPosX, int winPosY );
    void       forcePointOfInterestUpdateDuringNextWheelZoom();

    // Track ball navigation specific
    void                                initializeRotationCenter();
    cvf::ref<cvf::ManipulatorTrackball> m_trackball;
    bool                                m_isRotCenterInitialized;
    cvf::Vec3d                          m_pointOfInterest;
    RotationSensitivityCalculator       m_roationSensitivityCalculator;

    bool m_isNavigating;
    bool m_hasMovedMouseDuringNavigation;

    void cvfEventPos( int qtX, int qtY, int* x, int* y );

    // Zooming towards cursor
    cvf::ref<cvf::Ray> createZoomRay( int cvfXPos, int cvfYPos );
    void               zoomAlongRay( cvf::Ray* ray, int delta );
    bool               m_isZooming;
    cvf::ref<cvf::Ray> m_zoomRay;
    int                m_lastPosX; /// Previous mouse position
    int                m_lastPosY;

    bool isSupposedToConsumeEvents() { return m_consumeEvents; }
    bool isRotationEnabled() { return m_isRotationEnabled; }

private:
    void updateWheelZoomPosition( int winPosX, int winPosY );
    bool shouldRaytraceForNewPoiDuringWheelZoom( int winPosX, int winPosY ) const;

private:
    bool m_consumeEvents;
    bool m_isRotationEnabled;
    int  m_lastWheelZoomPosX;
    int  m_lastWheelZoomPosY;
};

} // End namespace caf
