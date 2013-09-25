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


#include "cvfBase.h"
#include "cvfOverlayTextBox.h"
#include "cvfDrawableText.h"
#include "cvfMatrixState.h"
#include "cvfCamera.h"
#include "cvfShaderProgram.h"
#include "cvfOpenGL.h"
#include "cvfViewport.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfUniform.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#endif

namespace cvf {

//==================================================================================================
///
/// \class cvf::OverlayItem
/// \ingroup Render
///
/// A base class for all overlay items
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Do hit test on the overlay item. Base class only does a check 
//--------------------------------------------------------------------------------------------------
bool OverlayItem::pick(int x, int y, const Vec2i& position, const Vec2ui& size)
{
    Recti oglRect(position, static_cast<int>(size.x()), static_cast<int>(size.y()));

    if ((x > oglRect.min().x()) && (x < oglRect.max().x()) &&
        (y > oglRect.min().y()) && (y < oglRect.max().y()))
    {
        return true;
    }

    return false;
}

} // namespace cvf
