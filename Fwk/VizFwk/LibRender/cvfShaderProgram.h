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

#include "cvfShader.h"
#include "cvfCollection.h"

#include <map>

namespace cvf {

class Uniform;
class UniformSet;
class MatrixState;
class OpenGLContext;
class OglRcProgram;


//==================================================================================================
//
// Encapsulates a GLSL shader program
//
//==================================================================================================
class ShaderProgram : public Object
{
public:
    enum FixedUniform
    {
        PROJECTION_MATRIX,
        VIEW_MATRIX,
        VIEW_MATRIX_INVERSE,
        MODEL_MATRIX,
        MODEL_MATRIX_INVERSE,
        MODEL_MATRIX_INVERSE_TRANSPOSE,
        MODEL_VIEW_MATRIX,
        MODEL_VIEW_MATRIX_INVERSE,
        MODEL_VIEW_PROJECTION_MATRIX,
        NORMAL_MATRIX,
        VIEWPORT_WIDTH,
        VIEWPORT_HEIGHT,
        PIXEL_HEIGHT_AT_UNIT_DISTANCE
    };

    enum FixedAttribute
    {
        VERTEX	            = 0,		// At least Nvidia has the following aliases for gl_Vertex, gl_Normal and gl_Color
        NORMAL	            = 2,		// Source: http://developer.download.nvidia.com/opengl/glsl/glsl_release_notes.pdf
		COLOR	            = 3,
        TEX_COORD_2F_0      = 8
    };

public:
    ShaderProgram(const String& programName = String());
    ~ShaderProgram();

    void        setProgramName(const String& programName);
    String      programName() const;

    void        addShader(Shader* shader);
    uint		shaderCount() const;
    Shader*		shader(uint index);

    bool        linkProgram(OpenGLContext* oglContext);
    bool        useProgram(OpenGLContext* oglContext) const;
    static void useNoProgram(OpenGLContext* oglContext);
    bool        isProgramUsed(OpenGLContext* oglContext) const;

	OglId       programOglId() const;
    void        deleteProgram(OpenGLContext* oglContext);

    bool        validateProgram(OpenGLContext* oglContext) const;
    String      programInfoLog(OpenGLContext* oglContext) const;

    int	        uniformLocation(const char* name) const;

    void        setDefaultUniform(Uniform* uniform);
    bool        applyDefaultUniforms(OpenGLContext* oglContext);

    bool        applyUniform(OpenGLContext* oglContext, const Uniform& uniform);
    bool        applyUniforms(OpenGLContext* oglContext, const UniformSet& uniformSet);
    void        applyActiveUniformsOnly(OpenGLContext* oglContext, const UniformSet& uniformSet);
    void        applyFixedUniforms(OpenGLContext* oglContext, const MatrixState& matrixState);

    void        clearUniformApplyTracking();
    void        checkReportAllUniformsApplied(OpenGLContext* oglContext) const;
    void        disableUniformTrackingForUniform(const char* name);

    int	        attributeLocation(OpenGLContext* oglContext, const char* name) const;

    static bool supportedOpenGL(OpenGLContext* oglContext);

private:
    bool        createProgram(OpenGLContext* oglContext);
    bool        needsLinking() const;

    void        discoverActiveUniforms(OpenGLContext* oglContext);
    static bool mapUniformNameToFixedUniformEnum(const char* uniformName, FixedUniform* fixedUniform);
    static void applyUniformAtLocation(OpenGLContext* oglContext, int location, const Uniform& uniform);

	void		bindFixedAttributes(OpenGLContext* oglContext);

private:
    String                      m_programName;                  // Name of this program. Primarily used when reporting errors. 
    ref<OglRcProgram>           m_oglRcProgram;			        // Reference to OglRc object that contains the program's OpenGL id 
    Collection<Shader>	        m_shaders;                      // 
    std::vector<int>            m_linkedShaderVersionTicks;
    bool                        m_needsLinking;					// Flag to indicate that this shader program must be linked

    ref<UniformSet>             m_defaultUniformSet;            // The shader program's default uniform set (if any). Applied using applyDefaultUniforms() (before any other appky functions)

    std::map<FixedUniform, int> m_fixedUniformsMap;             // Maps the CVF fixed uniforms that were discovered to their locations
    std::map<std::string, int>  m_fixedUniformsNameLocationMap; // Maps fixed uniform names to their locations. Populated during discovery of active uniforms
    std::map<std::string, int>  m_uniformsNameLocationMap;      // Maps general uniform names to their locations. Populated during discovery of active uniforms

    std::set<int>               m_appliedUniformLocations;      // Tracks the uniforms that have been applied since the last call to clearUniformApplyTracking()
    std::set<std::string>       m_uniformsDisabledFromTracking; // Array with the uniforms that should be exempt from the uniform tracking
};

}
