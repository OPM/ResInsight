//##################################################################################################
//
//  Copyright (C) 2011, Ceetron AS
//  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Ceetron AS. The contents of this file may 
//  not be disclosed to third parties, copied or duplicated in any form, in whole or in part,
//  without the prior written permission of Ceetron AS.
//##################################################################################################

#pragma once

#include "cvfBase.h"
#include "cafNavigationPolicy.h"
#include "cvfManipulatorTrackball.h"
#include "cvfRay.h"



namespace caf
{

class CeetronPlusNavigation : public NavigationPolicy
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

} // End namespace caf

