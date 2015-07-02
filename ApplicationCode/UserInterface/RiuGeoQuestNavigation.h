/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#pragma once

#include "cvfBase.h"
#include "cafNavigationPolicy.h"
#include "cvfManipulatorTrackball.h"
#include "cvfRay.h"


class RiuGeoQuestNavigation : public caf::NavigationPolicy
{
protected:
    // General navigation policy overrides
    virtual void                        init();
    virtual bool                        handleInputEvent(QInputEvent* inputEvent);

    virtual void                        setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection );
    virtual cvf::Vec3d                  pointOfInterest(); 
    virtual void                        setPointOfInterest(cvf::Vec3d poi);

    // PdvNavigation specific
    void                                initializeRotationCenter();
    cvf::ref<cvf::ManipulatorTrackball> m_trackball;
    bool                                m_isRotCenterInitialized; 
    cvf::Vec3d                          m_pointOfInterest;

    bool                                m_isNavigating;
    bool                                m_hasMovedMouseDuringNavigation;

    // Handle mid mouse button zoom
    void                                zoomAlongRay( cvf::Ray* ray, int delta );
    bool                                m_isZooming;
    cvf::ref<cvf::Ray>                  m_zoomRay;
    int                                 m_lastPosX;  /// Previous mouse position
    int                                 m_lastPosY;

};
