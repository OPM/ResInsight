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

#include "cvfShaderSourceRepository.h"

namespace cvf {

class ShaderSourceProvider;
class ShaderProgram;


//==================================================================================================
//
// 
//
//==================================================================================================
class ShaderProgramGenerator
{
public:
    ShaderProgramGenerator(String shaderProgramName, ShaderSourceProvider* sourceProvider);

    void                addVertexCode(ShaderSourceRepository::ShaderIdent shaderIdent);
    void                addFragmentCode(ShaderSourceRepository::ShaderIdent shaderIdent);
    void                addGeometryCode(ShaderSourceRepository::ShaderIdent shaderIdent);

    void                addVertexCode(const String& vertShaderCode);
    void                addFragmentCode(const String& fragShaderCode);

    void                addVertexCodeFromFile(String shaderName);
    void                addFragmentCodeFromFile(String shaderName);

    void                configureStandardHeadlightColor();
    void                configureStandardHeadlightTexture();

    ref<ShaderProgram>  generate();

private:
    ShaderSourceProvider*   m_sourceProvider;
    String                  m_shaderProgramName;

    std::vector<String>     m_vertexCodes;
    std::vector<String>     m_fragmentCodes;
    std::vector<String>     m_geometryCodes;

    std::vector<String>     m_vertexNames;
    std::vector<String>     m_fragmentNames;
    std::vector<String>     m_geometryNames;
};


//==================================================================================================
//
// 
//
//==================================================================================================
class ShaderSourceCombiner
{
public:
    ShaderSourceCombiner(const std::vector<String>& shaderCodes, const std::vector<String>& shaderNames);

    void        enableDebugComments(bool enableDebugComments);
    String      combinedSource() const;

private:
    static uint     findVersion(const String& shaderCode);
    static bool     startsWithGlobalKeyword(const String& codeLine);

private:
    const std::vector<String>&  m_shaderCodes;
    const std::vector<String>&  m_shaderNames;
    bool                        m_enableDebugComments;
};


}
