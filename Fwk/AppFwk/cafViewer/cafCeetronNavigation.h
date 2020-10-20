//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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
#include "cvfManipulatorTrackball.h"

class QMouseEvent;
class QWheelEvent;

namespace caf
{
class CeetronNavigation : public NavigationPolicy
{
public:
    CeetronNavigation();
    ~CeetronNavigation() override;

protected:
    // General navigation policy reimplememtation
    void       init() override;
    bool       handleInputEvent( QInputEvent* inputEvent ) override;
    void       setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection ) override;
    cvf::Vec3d pointOfInterest() override;
    void       setPointOfInterest( cvf::Vec3d poi ) override;

    // Ceetron navigation stuff
    void mouseMoveEvent( QMouseEvent* event );
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void wheelEvent( QWheelEvent* event );

    cvf::ManipulatorTrackball::NavigationType getNavigationTypeFromMouseButtons( Qt::MouseButtons mouseButtons );
    void                                      setCursorFromCurrentState();

    void initializeRotationCenter();

    cvf::ref<cvf::ManipulatorTrackball> m_trackball;
    bool                                m_isRotCenterInitialized;
    cvf::Vec3d                          m_pointOfInterest;
};

} // End namespace caf
