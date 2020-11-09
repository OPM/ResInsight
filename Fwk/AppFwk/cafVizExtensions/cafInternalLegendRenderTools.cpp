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

#include "cafInternalLegendRenderTools.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfRenderStateBlending.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStateLine.h"
#include "cvfRenderState_FF.h"
#include "cvfShaderProgram.h"
#include "cvfUniform.h"
#include <array>

using namespace cvf;

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Render a semi transparent background frame
//--------------------------------------------------------------------------------------------------
void InternalLegendRenderTools::renderBackgroundUsingShaders( OpenGLContext*     oglContext,
                                                              const MatrixState& matrixState,
                                                              const Vec2f&       size,
                                                              const Color4f&     backgroundColor,
                                                              const Color4f&     backgroundFrameColor )
{
    CVF_CALLSITE_OPENGL( oglContext );

    RenderStateDepth depth( false );
    depth.applyOpenGL( oglContext );

    RenderStateLine line( 1.0f );
    line.applyOpenGL( oglContext );

    RenderStateBlending blend;
    blend.configureTransparencyBlending();
    blend.applyOpenGL( oglContext );

    // Shader program

    ref<ShaderProgram> shaderProgram = oglContext->resourceManager()->getLinkedUnlitColorShaderProgram( oglContext );
    CVF_TIGHT_ASSERT( shaderProgram.notNull() );

    if ( shaderProgram->useProgram( oglContext ) )
    {
        shaderProgram->clearUniformApplyTracking();
        shaderProgram->applyFixedUniforms( oglContext, matrixState );
    }

    std::array<Vec3f, 4> vertexArray = {
        Vec3f( 1, 1, 0.0f ),
        Vec3f( size.x(), 1, 0.0f ),
        Vec3f( size.x(), size.y(), 0.0f ),
        Vec3f( 1, size.y(), 0.0f ),
    };

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glEnableVertexAttribArray( ShaderProgram::VERTEX );
    glVertexAttribPointer( ShaderProgram::VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertexArray.data() );

    // Draw frame background

    UniformFloat backgroundColorUniform( "u_color", backgroundColor );
    shaderProgram->applyUniform( oglContext, backgroundColorUniform );

    // Triangle indices for the frame background

    static const ushort backgroundTriangleIndices[] = { 0, 1, 2, 2, 3, 0 };

    glDrawRangeElements( GL_TRIANGLES, 0, 3, 6, GL_UNSIGNED_SHORT, backgroundTriangleIndices );

    // Draw frame border lines

    UniformFloat uniformColor( "u_color", backgroundFrameColor );
    shaderProgram->applyUniform( oglContext, uniformColor );

    static const ushort frameLineIndices[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

    glDrawRangeElements( GL_LINES, 0, 3, 8, GL_UNSIGNED_SHORT, frameLineIndices );

    glDisableVertexAttribArray( ShaderProgram::VERTEX );

    CVF_TIGHT_ASSERT( shaderProgram.notNull() );
    shaderProgram->useNoProgram( oglContext );

    // Reset render states
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL( oglContext );

    RenderStateLine resetLine;
    resetLine.applyOpenGL( oglContext );

    RenderStateBlending resetblend;
    resetblend.applyOpenGL( oglContext );

    CVF_CHECK_OGL( oglContext );
}

//--------------------------------------------------------------------------------------------------
/// Draw a background rectangle using OGL 1.1 compatibility
//--------------------------------------------------------------------------------------------------
void InternalLegendRenderTools::renderBackgroundImmediateMode( OpenGLContext* oglContext,
                                                               const Vec2f&   size,
                                                               const Color4f& backgroundColor,
                                                               const Color4f& backgroundFrameColor )
{
    RenderStateDepth depth( false );
    depth.applyOpenGL( oglContext );

    RenderStateLighting_FF lighting( false );
    lighting.applyOpenGL( oglContext );

    RenderStateBlending blend;
    blend.configureTransparencyBlending();
    blend.applyOpenGL( oglContext );

    // Frame vertices

    std::array<Vec3f, 4> vertexArray = {
        Vec3f( 1, 1, 0.0f ),
        Vec3f( size.x(), 1, 0.0f ),
        Vec3f( size.x(), size.y(), 0.0f ),
        Vec3f( 1, size.y(), 0.0f ),
    };

    glColor4fv( backgroundColor.ptr() );
    glBegin( GL_TRIANGLE_FAN );
    glVertex3fv( vertexArray[0].ptr() );
    glVertex3fv( vertexArray[1].ptr() );
    glVertex3fv( vertexArray[2].ptr() );
    glVertex3fv( vertexArray[3].ptr() );
    glEnd();

    // Render Line around

    {
        glColor4fv( backgroundFrameColor.ptr() );
        glBegin( GL_LINES );
        glVertex3fv( vertexArray[0].ptr() );
        glVertex3fv( vertexArray[1].ptr() );
        glVertex3fv( vertexArray[1].ptr() );
        glVertex3fv( vertexArray[2].ptr() );
        glVertex3fv( vertexArray[2].ptr() );
        glVertex3fv( vertexArray[3].ptr() );
        glVertex3fv( vertexArray[3].ptr() );
        glVertex3fv( vertexArray[0].ptr() );
        glEnd();
    }

    // Reset render states

    RenderStateLighting_FF resetLighting;
    resetLighting.applyOpenGL( oglContext );
    RenderStateDepth resetDepth;
    resetDepth.applyOpenGL( oglContext );
    RenderStateBlending resetblend;
    resetblend.applyOpenGL( oglContext );
    CVF_CHECK_OGL( oglContext );
}

} // namespace caf
