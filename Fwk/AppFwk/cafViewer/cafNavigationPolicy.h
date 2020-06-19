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

#include "cvfAssert.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <QPointer>

namespace caf
{
class Viewer;
}

class QInputEvent;

namespace caf
{
class NavigationPolicy : public cvf::Object
{
public:
    NavigationPolicy();
    ~NavigationPolicy() override;

    friend class Viewer;

public: // protected: // Should be protected but this friending does not work on gcc 4.1.2
    // Interface to be implement by subclass
    virtual void       init(){};
    virtual bool       handleInputEvent( QInputEvent* inputEvent )                                = 0;
    virtual void       setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection ) = 0;
    virtual cvf::Vec3d pointOfInterest();
    virtual void       setPointOfInterest( cvf::Vec3d poi ) = 0;

    // Interface used by caf::ViewerBase
    void             setViewer( Viewer* viewer );
    QPointer<Viewer> m_viewer;
};

} // End namespace caf
