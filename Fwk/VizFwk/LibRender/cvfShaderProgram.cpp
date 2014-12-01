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
#include "cvfShaderProgram.h"
#include "cvfOpenGL.h"
#include "cvfUniform.h"
#include "cvfUniformSet.h"
#include "cvfMatrixState.h"
#include "cvfTrace.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOglRc.h"
#include "cvfOpenGLCapabilities.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::ShaderProgram
/// \ingroup Render
///
/// Encapsulates a GLSL shader program.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
ShaderProgram::ShaderProgram(const String& programName)
:   m_programName(programName),
    m_needsLinking(true)
{
}


//--------------------------------------------------------------------------------------------------
/// Get name of this shader program
/// 
/// If no name has been set, one will be constructed by concatenating the name of the attached shaders.
//--------------------------------------------------------------------------------------------------
String ShaderProgram::programName() const
{
    if (m_programName.isEmpty())
    {
        String progName;
        uint numShaders = shaderCount();
        uint i;
        for (i = 0; i < numShaders; i++)
        {
            const Shader* shader = m_shaders.at(i);
            if (i > 0) progName += " # ";
            progName += shader->shaderName();
        }

        return progName;
    }
    else
    {
        return m_programName;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::setProgramName(const String& programName)
{
    m_programName = programName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderProgram::~ShaderProgram()
{
    // Just release our reference
    CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcProgram.p()));
    m_oglRcProgram = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::addShader(Shader* shader)
{
    CVF_ASSERT(shader);
    m_shaders.push_back(shader);
    m_linkedShaderVersionTicks.push_back(-1);
    m_needsLinking = true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint ShaderProgram::shaderCount() const
{
    return static_cast<uint>(m_shaders.size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Shader* ShaderProgram::shader(uint index)
{
    CVF_ASSERT(index < shaderCount());
    return m_shaders.at(index);
}


//--------------------------------------------------------------------------------------------------
/// Links the program if needed
/// 
/// Will create the shader program if needed. Will compile the program's shaders if needed.
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::linkProgram(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    if (OglRc::safeOglId(m_oglRcProgram.p()) != 0)
    {
        if (!needsLinking())
        {
            return true;
        }
    }


    // Do a delete/create cycle to avoid keeping tabs of which shaders
    // that need to be detached. Assumption is that relinking happens seldom
    deleteProgram(oglContext);
    if (!createProgram(oglContext))
    {
        CVF_LOG_RENDER_ERROR(oglContext, String("Unable to create program '%1'").arg(programName()));
        return false;
    }

    // For now, always call compile on all the shaders
    // The compile should be a no-op if everything is in sync
    uint numShaders = shaderCount();
    CVF_ASSERT(numShaders == static_cast<uint>(m_linkedShaderVersionTicks.size()));

    uint i;
    for (i = 0; i < numShaders; i++)
    {
        Shader* shader = m_shaders.at(i);
        if (!shader->compile(oglContext))
        {
            CVF_LOG_RENDER_ERROR(oglContext, String("Error compiling shader '%1' attached to program '%2'").arg(shader->shaderName()).arg(programName()));
            return false;
        }
        
        CVF_ASSERT(shader->shaderOglId() != 0);
        glAttachShader(m_oglRcProgram->oglId(), shader->shaderOglId());
        CVF_CHECK_OGL(oglContext);

        m_linkedShaderVersionTicks[i] = shader->compiledVersionTick();
    }

	bindFixedAttributes(oglContext);

    glLinkProgram(m_oglRcProgram->oglId());

    //Trace::show("%d", glGetAttribLocation(m_programObjID, "cvfa_vertex"));
    //Trace::show("%d", glGetAttribLocation(m_programObjID, "cvfa_normal"));
    //Trace::show("%d", glGetAttribLocation(m_programObjID, "cvfa_color"));
    //Trace::show(programInfoLog().toAscii().ptr());

    GLint iLinkStatus = 0;
    glGetProgramiv(m_oglRcProgram->oglId(), GL_LINK_STATUS, &iLinkStatus);
    if (iLinkStatus != GL_TRUE)
    {
        CVF_LOG_RENDER_ERROR(oglContext, String("Error linking shader program '%1', GLSL details:\n%2").arg(programName()).arg(programInfoLog(oglContext)));
        return false;
    }

    m_needsLinking = false;

    discoverActiveUniforms(oglContext);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Creates the program. Does nothing shader program is already created
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::createProgram(OpenGLContext* oglContext)
{
    CVF_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    if (OglRc::safeOglId(m_oglRcProgram.p()) == 0)
    {
        OpenGLResourceManager* resourceManager = oglContext->resourceManager();

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcProgram.p()));
        m_oglRcProgram = resourceManager->createOglRcProgram(oglContext);
        if (m_oglRcProgram.isNull())
        {
            CVF_CHECK_OGL(oglContext);
            return false;
        }

        m_needsLinking = true;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::needsLinking() const
{
    if (m_needsLinking) return true;

    uint numShaders = shaderCount();
    CVF_ASSERT(numShaders == static_cast<uint>(m_linkedShaderVersionTicks.size()));

    uint i;
    for (i = 0; i < numShaders; i++)
    {
        const Shader* shader = m_shaders.at(i);
        int compiledVersionTick = shader->compiledVersionTick();
        if (compiledVersionTick != m_linkedShaderVersionTicks[i] || compiledVersionTick == -1)
        {
            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::useProgram(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    if (m_needsLinking)
    {
        return false;
    }

    CVF_ASSERT(OglRc::safeOglId(m_oglRcProgram.p()) != 0);
#ifndef CVF_OSX
    CVF_ASSERT(validateProgram(oglContext));
#endif
    
    // Need this check to clear any "hanging" errors that is not produced by glUseProgram below, but still 
    // will make this method return false.
    CVF_CLEAR_OGL_ERROR(oglContext);

    glUseProgram(m_oglRcProgram->oglId());

    // TODO
    // Is there any lightweight way to check if the glUseCall was successful
    if (CVF_TEST_AND_REPORT_OPENGL_ERROR(oglContext, "Use shader program"))
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::useNoProgram(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(ShaderProgram::supportedOpenGL(oglContext));

    glUseProgram(0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::isProgramUsed(OpenGLContext* /*oglContext*/) const
{
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);  // Note: GL_CURRENT_PROGRAM is renamed to GL_ACTIVE_PROGRAM in 4.1

    if (currentProgram != 0)
    {
        if (static_cast<OglId>(currentProgram) == OglRc::safeOglId(m_oglRcProgram.p()))
        {
            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
OglId ShaderProgram::programOglId() const
{
    return OglRc::safeOglId(m_oglRcProgram.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::deleteProgram(OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);

    if (m_oglRcProgram.notNull())
    {
        m_oglRcProgram->deleteResource(oglContext);
        CVF_CHECK_OGL(oglContext);

        CVF_ASSERT(OglRc::isSafeToRelease(m_oglRcProgram.p()));
        m_oglRcProgram = NULL;

        uint numShaders = static_cast<uint>(m_linkedShaderVersionTicks.size());
        uint i;
        for (i = 0; i < numShaders; i++)
        {
            m_linkedShaderVersionTicks[i] = -1;
        }

        m_needsLinking = true;
    }

    CVF_ASSERT(m_needsLinking);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::validateProgram(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (OglRc::safeOglId(m_oglRcProgram.p()) == 0)
    {
        return false;
    }

    glValidateProgram(m_oglRcProgram->oglId());

    GLint validateStatus = GL_FALSE;
    glGetProgramiv(m_oglRcProgram->oglId(), GL_VALIDATE_STATUS, &validateStatus);
    if (validateStatus == GL_TRUE)
    {
        return true;
    }
    else
    {
        // Any textual result of the vaidation is presented in the log
        CVF_LOG_RENDER_ERROR(oglContext, programInfoLog(oglContext));
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the information log for this shader
/// 
/// See: http://www.opengl.org/sdk/docs/man/xhtml/glGetProgramInfoLog.xml
//--------------------------------------------------------------------------------------------------
String ShaderProgram::programInfoLog(OpenGLContext* oglContext) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    uint myOglId = OglRc::safeOglId(m_oglRcProgram.p());
    if (myOglId == 0)
    {
        return "Program object not created.";
    }

    if (!glIsProgram(myOglId))
    {
        return "Program object identifier does not correspond to a shader program object.";
    }

    GLint reqBufferSize = 0;    
    glGetProgramiv(myOglId, GL_INFO_LOG_LENGTH, &reqBufferSize);

    if (reqBufferSize > 0)
    {
        // TODO
        // This is a good place for using CharArray
        std::vector<GLchar> charBuffer;
        charBuffer.resize(static_cast<size_t>(reqBufferSize + 1));
        glGetProgramInfoLog(myOglId, reqBufferSize, NULL, &charBuffer[0]);

        String logString(&charBuffer[0]);
        return logString;
    }
    else
    {
        return "";
    }
}



//--------------------------------------------------------------------------------------------------
/// Set a default uniform for this shader program
///
/// Default uniforms are stored with the shader program and may be used to bundle 'default' values
/// for some of the uniforms that the shader program exposes.and The default uniforms should be 
/// applied using the applyDefaultUniforms() function.
//--------------------------------------------------------------------------------------------------
void ShaderProgram::setDefaultUniform(Uniform* uniform)
{
    CVF_ASSERT(uniform);

    if (m_defaultUniformSet.isNull())
    {
        m_defaultUniformSet = new UniformSet;
    }

    m_defaultUniformSet->setUniform(uniform);
}


//--------------------------------------------------------------------------------------------------
/// Apply the shader program's default uniforms
/// 
/// Will apply the shader program's default uniforms, if any. To get the intended behavior, the 
/// default uniforms should typically be applied before applying any other uniforms. Otherwise, the 
/// default values will overwrite any user specified uniforms.
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::applyDefaultUniforms(OpenGLContext* oglContext)
{
    bool allUniformsSuccess = true;

    if (m_defaultUniformSet.notNull())
    {
        size_t numUniforms = m_defaultUniformSet->count();
        size_t i;
        for (i = 0; i < numUniforms; i++)
        {
            const Uniform* uniform = m_defaultUniformSet->uniform(i);
            allUniformsSuccess &= applyUniform(oglContext, *uniform);
        }
    }

    return allUniformsSuccess;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::applyUniformAtLocation(OpenGLContext* oglContext, int location, const Uniform& uniform)
{
    CVF_CALLSITE_OPENGL(oglContext);
    CVF_TIGHT_ASSERT(location >= 0);

    const int valCount = uniform.valueCount();
    CVF_ASSERT_MSG(valCount > 0, "No data set for uniform");

    switch (uniform.type())
    {
        case Uniform::INT:          glUniform1iv(location, valCount, uniform.intPtr());   break;
        case Uniform::FLOAT:        glUniform1fv(location, valCount, uniform.floatPtr()); break;
        case Uniform::FLOAT_VEC2:   glUniform2fv(location, valCount, uniform.floatPtr()); break;
        case Uniform::FLOAT_VEC3:   glUniform3fv(location, valCount, uniform.floatPtr()); break;
        case Uniform::FLOAT_VEC4:   glUniform4fv(location, valCount, uniform.floatPtr()); break;

        case Uniform::FLOAT_MAT4:   glUniformMatrix4fv(location, valCount, GL_FALSE, uniform.floatPtr()); break;

        case Uniform::UNDEFINED:    CVF_FAIL_MSG("Unhandled type");
    }
}


//--------------------------------------------------------------------------------------------------
/// Specify value of a uniform variable in this shader program
/// 
/// \return Returns false if the uniform doesn't exist in this shader program.
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::applyUniform(OpenGLContext* oglContext, const Uniform& uniform) 
{
    int location = uniformLocation(uniform.name());
    if (location == -1)
    {
        // If you end up here one of the following might have happened
        //  - the shader program is not properly linked
        //  - the uniform name is wrong
        //  - the specified uniform name is declared but not actually used in the program
        //    It is actually necessary to make the texture have influence on the resulting color, not just touch
        //    the texture.
        CVF_LOG_RENDER_ERROR(oglContext, String("In program '%1', error applying uniform: %2").arg(programName()).arg(uniform.name()));

        return false;
    }

    applyUniformAtLocation(oglContext, location, uniform);
    m_appliedUniformLocations.insert(location);

    CVF_CHECK_OGL(oglContext);

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Specify values of uniform variables in this shader program
/// 
/// \return Returns true if all uniforms in the uniform set were applied successfully. Returns false 
///         if any of the uniforms didn't exist in the shader program. 
/// 
/// \sa applyUniform(), applyActiveUniformsOnly()
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::applyUniforms(OpenGLContext* oglContext, const UniformSet& uniformSet) 
{
    bool allUniformsSuccess = true;

    size_t numUniforms = uniformSet.count();
    size_t i;
    for (i = 0; i < numUniforms; i++)
    {
        const Uniform* uniform = uniformSet.uniform(i);
        allUniformsSuccess &= applyUniform(oglContext, *uniform);
    }

    return allUniformsSuccess;
}


//--------------------------------------------------------------------------------------------------
/// Apply only uniforms that are active in this shader program
/// 
/// As opposed to applyUniforms() this function will allow uniform sets consisting of uniforms that are
/// not active in the shader program.
//--------------------------------------------------------------------------------------------------
void ShaderProgram::applyActiveUniformsOnly(OpenGLContext* oglContext, const UniformSet& uniformSet) 
{
    size_t  numUniforms = uniformSet.count();
    size_t i;
    for (i = 0; i < numUniforms; i++)
    {
        const Uniform* uniform = uniformSet.uniform(i);
        int location = uniformLocation(uniform->name());
        if (location != -1)
        {
            applyUniformAtLocation(oglContext, location, *uniform);
            m_appliedUniformLocations.insert(location);
        }
    }

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::applyFixedUniforms(OpenGLContext* oglContext, const MatrixState& matrixState) 
{
    CVF_CALLSITE_OPENGL(oglContext);

    std::map<FixedUniform, int>::const_iterator it;
    for (it = m_fixedUniformsMap.begin(); it != m_fixedUniformsMap.end(); ++it)
    {
        int location = it->second;
        CVF_ASSERT(location >= 0);
        
        FixedUniform fixedUniform = it->first;
        switch (fixedUniform)
        {
            case PROJECTION_MATRIX:                 glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.projectionMatrix().ptr());            break;

            case VIEW_MATRIX:                       glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.viewMatrix().ptr());                  break;
            case VIEW_MATRIX_INVERSE:               glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.viewMatrixInverse().ptr());           break;

            case MODEL_MATRIX:                      glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelMatrix().ptr());                 break;
            case MODEL_MATRIX_INVERSE:              glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelMatrixInverse().ptr());          break;
            case MODEL_MATRIX_INVERSE_TRANSPOSE:    glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelMatrixInverseTranspose().ptr()); break;

            case MODEL_VIEW_MATRIX:                 glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelViewMatrix().ptr());             break;
            case MODEL_VIEW_MATRIX_INVERSE:         glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelViewMatrixInverse().ptr());      break;

            case MODEL_VIEW_PROJECTION_MATRIX:      glUniformMatrix4fv(location, 1, GL_FALSE, matrixState.modelViewProjectionMatrix().ptr());   break;

            case NORMAL_MATRIX:                     glUniformMatrix3fv(location, 1, GL_FALSE, matrixState.normalMatrix().ptr());                break;

            case VIEWPORT_WIDTH:                    glUniform1i(location,  static_cast<GLint>(matrixState.viewportSize().x()));                 break;
            case VIEWPORT_HEIGHT:                   glUniform1i(location,  static_cast<GLint>(matrixState.viewportSize().y()));                 break;
            case PIXEL_HEIGHT_AT_UNIT_DISTANCE:     glUniform1f(location,  matrixState.pixelHeightAtUnitDistance());                            break;

            default:
                CVF_FAIL_MSG("Unhandled fixed uniform");
                break;
        }

        // Assume that the uniform got set
        // Hitting default clause implies programming error
        m_appliedUniformLocations.insert(location);

        CVF_CHECK_OGL(oglContext);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::clearUniformApplyTracking()
{
    m_appliedUniformLocations.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::checkReportAllUniformsApplied(OpenGLContext* oglContext) const
{
    const size_t numUniformsApplied = m_appliedUniformLocations.size();
    const size_t numUniforms = m_fixedUniformsNameLocationMap.size() + m_uniformsNameLocationMap.size();

    if (numUniforms != numUniformsApplied)
    {
        std::map<std::string, int>::const_iterator it;
        for (it = m_uniformsNameLocationMap.begin(); it != m_uniformsNameLocationMap.end(); ++it)
        {
            int uniformLocation = it->second;
            if (m_appliedUniformLocations.find(uniformLocation) == m_appliedUniformLocations.end())
            {
                std::string uniformName(it->first);

                if (m_uniformsDisabledFromTracking.find(uniformName) == m_uniformsDisabledFromTracking.end())
                {
                    CVF_LOG_RENDER_ERROR(oglContext, String("In program '%1', uniform not set: %2").arg(programName()).arg(uniformName));
                }
            }
        }

        for (it = m_fixedUniformsNameLocationMap.begin(); it != m_fixedUniformsNameLocationMap.end(); ++it)
        {
            int uniformLocation = it->second;
            if (m_appliedUniformLocations.find(uniformLocation) == m_appliedUniformLocations.end())
            {
                CVF_LOG_RENDER_ERROR(oglContext, String("In program '%1', fixed uniform not set: %2").arg(programName()).arg(String(it->first)));
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::disableUniformTrackingForUniform(const char* name)
{
    m_uniformsDisabledFromTracking.insert(name);
}


//--------------------------------------------------------------------------------------------------
/// Returns integer that represents the location of the specific uniform variable
/// 
/// This function returns -1 if the name does not correspond to an active uniform variable in 
/// the program.
//--------------------------------------------------------------------------------------------------
int ShaderProgram::uniformLocation(const char* name) const
{
    std::map<std::string, int>::const_iterator it = m_uniformsNameLocationMap.find(name);
    if (it != m_uniformsNameLocationMap.end())
    {
        return it->second;
    }

    // Not found amongst general uniforms. Now try the fixed ones
    it = m_fixedUniformsNameLocationMap.find(name);
    if (it != m_fixedUniformsNameLocationMap.end())
    {
        return it->second;
    }

    return -1;
}


//--------------------------------------------------------------------------------------------------
/// Does discovery of active uniforms in this program.
/// 
/// Finds all active uniforms in the (linked) shader program and categorizes them as either fixed
/// or general uniforms. Stores the location for the uniforms for later usage.
//--------------------------------------------------------------------------------------------------
void ShaderProgram::discoverActiveUniforms(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);

    m_fixedUniformsMap.clear();
    m_fixedUniformsNameLocationMap.clear();
    m_uniformsNameLocationMap.clear();
    m_appliedUniformLocations.clear();

    GLint numActiveUniforms = 0;
    GLint maxUniformNameLength = 0;
    glGetProgramiv(m_oglRcProgram->oglId(), GL_ACTIVE_UNIFORMS, &numActiveUniforms);
    glGetProgramiv(m_oglRcProgram->oglId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
    if (numActiveUniforms <= 0 || maxUniformNameLength <= 0)
    {
        return;
    }

    CharArray uniformNameBuffer(static_cast<size_t>(maxUniformNameLength), 0);
    const GLsizei bufferSize = static_cast<GLsizei>(uniformNameBuffer.size());
    
    uint i;
    for (i = 0; i < static_cast<uint>(numActiveUniforms); i++)
    {
        GLsizei numCharsInName = 0;
        GLint uniformSize = 0;
        GLenum uniformDataType = 0;
        glGetActiveUniform(m_oglRcProgram->oglId(), i, bufferSize, &numCharsInName, &uniformSize, &uniformDataType, uniformNameBuffer.ptr());
        CVF_CHECK_OGL(oglContext);

        //Trace::show("%d:  uniformSize=%d  uniformDataType=%d  uniformName=%s", i, uniformSize, uniformDataType, uniformNameBuffer.ptr());

        if (numCharsInName > 0)
        {
            std::string uniformName = uniformNameBuffer.ptr();

            // Added this code to fix a bug in the Nvidia drivers that reports arrays as xxx[0], like u_ecClipPlanes reported as u_ecClipPlanes[0]
            size_t bracketPos = uniformName.find('[');
            if (bracketPos != std::string::npos)
            {
                uniformName.resize(bracketPos);
            }    

            int location = glGetUniformLocation(m_oglRcProgram->oglId(), uniformName.c_str());
            CVF_CHECK_OGL(oglContext);


            FixedUniform fixedUniform;
            if (ShaderProgram::mapUniformNameToFixedUniformEnum(uniformName.c_str(), &fixedUniform))
            {
                m_fixedUniformsMap[fixedUniform] = location;
                m_fixedUniformsNameLocationMap[uniformName] = location;
            }
            else
            {
                m_uniformsNameLocationMap[uniformName] = location;
            }

        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::mapUniformNameToFixedUniformEnum(const char* uniformName, FixedUniform* fixedUniform)
{
    CVF_ASSERT(uniformName);
    CVF_ASSERT(fixedUniform);

    if      (System::strcmp(uniformName, "cvfu_projectionMatrix") == 0)             { *fixedUniform = PROJECTION_MATRIX;                return true; }

    else if (System::strcmp(uniformName, "cvfu_viewMatrix") == 0)                   { *fixedUniform = VIEW_MATRIX;                      return true; }
    else if (System::strcmp(uniformName, "cvfu_viewMatrixInverse") == 0)            { *fixedUniform = VIEW_MATRIX_INVERSE;              return true; }

    else if (System::strcmp(uniformName, "cvfu_modelMatrix") == 0)                  { *fixedUniform = MODEL_MATRIX;                     return true; }
    else if (System::strcmp(uniformName, "cvfu_modelMatrixInverse") == 0)           { *fixedUniform = MODEL_MATRIX_INVERSE;             return true; }
    else if (System::strcmp(uniformName, "cvfu_modelMatrixInverseTranspose") == 0)  { *fixedUniform = MODEL_MATRIX_INVERSE_TRANSPOSE;   return true; }

    else if (System::strcmp(uniformName, "cvfu_modelViewMatrix") == 0)              { *fixedUniform = MODEL_VIEW_MATRIX;                return true; }
    else if (System::strcmp(uniformName, "cvfu_modelViewMatrixInverse") == 0)       { *fixedUniform = MODEL_VIEW_MATRIX_INVERSE;        return true; }

    else if (System::strcmp(uniformName, "cvfu_modelViewProjectionMatrix") == 0)    { *fixedUniform = MODEL_VIEW_PROJECTION_MATRIX;     return true; }

    else if (System::strcmp(uniformName, "cvfu_normalMatrix") == 0)                 { *fixedUniform = NORMAL_MATRIX;                    return true; }

    else if (System::strcmp(uniformName, "cvfu_viewportWidth") == 0)                { *fixedUniform = VIEWPORT_WIDTH;                   return true; }
    else if (System::strcmp(uniformName, "cvfu_viewportHeight") == 0)               { *fixedUniform = VIEWPORT_HEIGHT;                  return true; }
    else if (System::strcmp(uniformName, "cvfu_pixelHeightAtUnitDistance") == 0)    { *fixedUniform = PIXEL_HEIGHT_AT_UNIT_DISTANCE;    return true; }

    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the index of the vertex attribute that is bound to attribute variable specified by name
/// 
/// Returns -1 if the name does not correspond to an active attribute or if name starts with the 
/// reserved "gl_" prefix.
/// 
/// \sa http://www.opengl.org/sdk/docs/man3/xhtml/glGetAttribLocation.xml
//--------------------------------------------------------------------------------------------------
int ShaderProgram::attributeLocation(OpenGLContext* oglContext, const char* name) const
{
    CVF_CALLSITE_OPENGL(oglContext);

    if (OglRc::safeOglId(m_oglRcProgram.p()) != 0)
    {
        GLint attribIndex = glGetAttribLocation(m_oglRcProgram->oglId(), name);
        return attribIndex;
    }
	
    return -1;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgram::bindFixedAttributes(OpenGLContext* oglContext)
{
    CVF_CALLSITE_OPENGL(oglContext);
	CVF_ASSERT(OglRc::safeOglId(m_oglRcProgram.p()) != 0);

    glBindAttribLocation(m_oglRcProgram->oglId(), VERTEX,        "cvfa_vertex");
    glBindAttribLocation(m_oglRcProgram->oglId(), NORMAL,        "cvfa_normal");
    glBindAttribLocation(m_oglRcProgram->oglId(), COLOR,         "cvfa_color");
    glBindAttribLocation(m_oglRcProgram->oglId(), TEX_COORD_2F_0,"cvfa_texCoord");

    CVF_CHECK_OGL(oglContext);
}


//--------------------------------------------------------------------------------------------------
/// Query if the current OpenGL context supports shaders
//--------------------------------------------------------------------------------------------------
bool ShaderProgram::supportedOpenGL(OpenGLContext* oglContext)
{
    return oglContext->capabilities()->supportsOpenGL2();
}

} // namespace cvf
