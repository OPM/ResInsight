//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
bool OverlayItem::pick(uint oglXCoord, uint oglYCoord, const Vec2ui& position, const Vec2ui& size)
{
    Rectui oglRect(position, size.x(), size.y());

    if ((oglXCoord > oglRect.min().x()) && (oglXCoord < oglRect.max().x()) &&
        (oglYCoord > oglRect.min().y()) && (oglYCoord < oglRect.max().y()))
    {
        return true;
    }

    return false;
}

} // namespace cvf
