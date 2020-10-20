//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2018- Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfColor4.h"
#include "cvfMatrixState.h"
#include "cvfOpenGLContext.h"

//==================================================================================================
//
// Tools to render legend stuff
//
//==================================================================================================
namespace caf
{
class InternalLegendRenderTools
{
    using OpenGLContext = cvf::OpenGLContext;
    using Color4f       = cvf::Color4f;
    using String        = cvf::String;
    using MatrixState   = cvf::MatrixState;
    using Vec2f         = cvf::Vec2f;

public:
    static void renderBackgroundUsingShaders( OpenGLContext*     oglContext,
                                              const MatrixState& matrixState,
                                              const Vec2f&       size,
                                              const Color4f&     backgroundColor,
                                              const Color4f&     backgroundFrameColor );
    static void renderBackgroundImmediateMode( OpenGLContext* oglContext,
                                               const Vec2f&   size,
                                               const Color4f& backgroundColor,
                                               const Color4f& backgroundFrameColor );
};

} // namespace caf
