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

namespace cvf {
    class ManipulatorTrackball;
    class Ray;
}

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

class TrackBallBasedNavigation: public NavigationPolicy
{
public:
    TrackBallBasedNavigation();
    virtual ~TrackBallBasedNavigation();
    void enableEventEating(bool enable) { m_consumeEvents = enable; }
    void enableRotation(bool enable)    { m_isRotationEnabled = enable; }

protected:
    // General navigation policy overrides
    virtual void                        init();

    virtual void                        setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection );
    virtual cvf::Vec3d                  pointOfInterest(); 
    virtual void                        setPointOfInterest(cvf::Vec3d poi);

    // Track ball navigation specific
    void                                initializeRotationCenter();
    cvf::ref<cvf::ManipulatorTrackball> m_trackball;
    bool                                m_isRotCenterInitialized; 
    cvf::Vec3d                          m_pointOfInterest;

    bool                                m_isNavigating;
    bool                                m_hasMovedMouseDuringNavigation;

    void                                cvfEventPos(int qtX, int qtY, int* x, int* y);

    // Zooming towards cursor
    cvf::ref<cvf::Ray>                  createZoomRay(int cvfXPos, int cvfYPos);
    void                                zoomAlongRay( cvf::Ray* ray, int delta );
    bool                                m_isZooming;
    cvf::ref<cvf::Ray>                  m_zoomRay;
    int                                 m_lastPosX;  /// Previous mouse position
    int                                 m_lastPosY;

    bool                                isSupposedToConsumeEvents() { return m_consumeEvents; }
    bool                                isRotationEnabled() { return m_isRotationEnabled; }

private:
    bool                                m_consumeEvents;
    bool                                m_isRotationEnabled;
};

} // End namespace caf


