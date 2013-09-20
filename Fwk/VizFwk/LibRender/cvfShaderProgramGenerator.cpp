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
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderProgram.h"
#include "cvfShaderSourceProvider.h"
#include "cvfMath.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::ShaderProgramGenerator
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderProgramGenerator::ShaderProgramGenerator(String shaderProgramName, ShaderSourceProvider* sourceProvider)
{
    CVF_ASSERT(sourceProvider);
    m_sourceProvider = sourceProvider;
    m_shaderProgramName = shaderProgramName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addVertexCode(ShaderSourceRepository::ShaderIdent shaderIdent)
{
    String code = m_sourceProvider->getSourceFromRepository(shaderIdent);
    m_vertexCodes.push_back(code);
    m_vertexNames.push_back(ShaderSourceRepository::shaderIdentString(shaderIdent));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addFragmentCode(ShaderSourceRepository::ShaderIdent shaderIdent)
{
    String code = m_sourceProvider->getSourceFromRepository(shaderIdent);
    m_fragmentCodes.push_back(code);
    m_fragmentNames.push_back(ShaderSourceRepository::shaderIdentString(shaderIdent));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addGeometryCode(ShaderSourceRepository::ShaderIdent shaderIdent)
{
    String code = m_sourceProvider->getSourceFromRepository(shaderIdent);
    m_geometryCodes.push_back(code);
    m_geometryNames.push_back(ShaderSourceRepository::shaderIdentString(shaderIdent));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addVertexCode(const String& vertShaderCode)
{
    m_vertexNames.push_back("Vert");
    m_vertexCodes.push_back(vertShaderCode);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addFragmentCode(const String& fragShaderCode)
{
    m_fragmentNames.push_back("Frag");
    m_fragmentCodes.push_back(fragShaderCode);  
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addVertexCodeFromFile(String shaderName)
{
    String code = m_sourceProvider->getSourceFromFile(shaderName);
    m_vertexCodes.push_back(code);
    m_vertexNames.push_back(shaderName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::addFragmentCodeFromFile(String shaderName)
{
    String code = m_sourceProvider->getSourceFromFile(shaderName);
    m_fragmentCodes.push_back(code);
    m_fragmentNames.push_back(shaderName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::configureStandardHeadlightColor()
{
    addVertexCode(ShaderSourceRepository::vs_Standard);
    addFragmentCode(ShaderSourceRepository::src_Color);
    addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    addFragmentCode(ShaderSourceRepository::fs_Standard);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderProgramGenerator::configureStandardHeadlightTexture()
{
    addVertexCode(ShaderSourceRepository::vs_Standard);
    addFragmentCode(ShaderSourceRepository::src_Texture);
    addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    addFragmentCode(ShaderSourceRepository::fs_Standard);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ShaderProgram> ShaderProgramGenerator::generate()
{
    CVF_ASSERT((m_vertexCodes.size() > 0) && (m_fragmentCodes.size() > 0));

    ShaderSourceCombiner vertexCombiner(m_vertexCodes, m_vertexNames);
    ShaderSourceCombiner fragmentCombiner(m_fragmentCodes, m_fragmentNames);
    String vertexSource = vertexCombiner.combinedSource();
    String fragmentSource = fragmentCombiner.combinedSource();

//     String vertexSource = combineShaders(m_vertexCodes, m_vertexNames);
//     String fragmentSource = combineShaders(m_fragmentCodes, m_fragmentNames);

    ref<ShaderProgram> prog = new ShaderProgram(m_shaderProgramName);
    ref<Shader> vertexShader = new Shader(Shader::VERTEX_SHADER, m_shaderProgramName + "-vert", vertexSource);
    ref<Shader> fragmentShader = new Shader(Shader::FRAGMENT_SHADER, m_shaderProgramName + "-frag", fragmentSource);

    prog->addShader(vertexShader.p());
    prog->addShader(fragmentShader.p());

    if (m_geometryCodes.size() > 0)
    {
        ShaderSourceCombiner geometryCombiner(m_geometryCodes, m_geometryNames);
        String geometrySource = geometryCombiner.combinedSource();

//        String geometrySource = combineShaders(m_geometryCodes, m_geometryNames);

        ref<Shader> geometryShader = new Shader(Shader::GEOMETRY_SHADER, m_shaderProgramName + "-geo", geometrySource);
        prog->addShader(geometryShader.p());
    }

    return prog;
}


//==================================================================================================
///
/// \class cvf::ShaderSourceCombiner
/// \ingroup Render
///
/// Helper class for combining shader source code (bits) into one source code string
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ShaderSourceCombiner::ShaderSourceCombiner(const std::vector<String>& shaderCodes, const std::vector<String>& shaderNames)
:   m_shaderCodes(shaderCodes),
    m_shaderNames(shaderNames),
    m_enableDebugComments(true)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ShaderSourceCombiner::combinedSource() const
{
    uint maxVersion = 0;
    size_t i;
    for (i = 0; i < m_shaderCodes.size(); i++)
    {
        uint thisVer = findVersion(m_shaderCodes[i]);
        maxVersion = CVF_MAX(thisVer, maxVersion);
    }

    String combinedSource;
    std::set<String> globalKeywordLinesAlreadyAdded;

    if (maxVersion > 0)
    {
        combinedSource = String("#version %1\n\n").arg(maxVersion);
    }
    
#ifdef CVF_OPENGL_ES
    combinedSource += "#define CVF_OPENGL_ES\n";
#endif

    for (i = 0; i < m_shaderCodes.size(); i++)
    {
        if (m_enableDebugComments)
        {
            combinedSource += "\n//=========================================================================================================\n//\n";
            combinedSource += String("// File: %1\n//-------------------------------------\n\n").arg(m_shaderNames[i]);
        }

        String source = m_shaderCodes[i];
        std::vector<String> sourceLines = source.split("\n");

        std::vector<String>::iterator it;
        for (it = sourceLines.begin(); it != sourceLines.end(); ++it)
        {
            String sourceLine = *it;
            bool addThisLine = true;
            
            if (sourceLine.find("#version") != String::npos)
            {
                addThisLine = false;
            }
            
            if (startsWithGlobalKeyword(sourceLine))
            {
                String simplifiedLine = sourceLine.simplified();
                if (globalKeywordLinesAlreadyAdded.find(simplifiedLine) != globalKeywordLinesAlreadyAdded.end())
                {
                    // Line already added
                    addThisLine = false;
                }
                else
                {
                    globalKeywordLinesAlreadyAdded.insert(simplifiedLine);                    
                }
            }

            if (addThisLine)
            {
                combinedSource += (sourceLine + "\n");
            }
        }
    }

    return combinedSource;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderSourceCombiner::enableDebugComments(bool enableDebugComments)
{
    m_enableDebugComments = enableDebugComments;
}


//--------------------------------------------------------------------------------------------------
/// Reads the first line of the shader and checks for "#version xxx"
//--------------------------------------------------------------------------------------------------
uint ShaderSourceCombiner::findVersion(const String& shaderCode)
{
    size_t firstNewline = shaderCode.find("\n");
    if (firstNewline == String::npos) 
    {
        return 0;
    }

    String firstLine = shaderCode.subString(0, firstNewline);
    std::vector<String> firstLineArr = firstLine.split(" ");

    if (firstLineArr.size() == 0)
    {
        return 0;
    }

    if (firstLineArr[0] != "#version") 
    {
        return 0;
    }

    uint version = static_cast<uint>(firstLineArr[1].toInt(0));

    return version;
}


//--------------------------------------------------------------------------------------------------
/// Check if the code line starts with one of the global keywords uniform, varying or attribute
//--------------------------------------------------------------------------------------------------
bool ShaderSourceCombiner::startsWithGlobalKeyword(const String& codeLine)
{
    if (codeLine.size() < 1)
    {
        return false;
    }

    std::wstring str = codeLine.trimmedLeft().toStdWString();

    // Same whitespace characters as isspace()
    const std::wstring whitespaces(L" \t\n\v\f\r");
    size_t pos = str.find_first_of(whitespaces); 
    if (pos != std::wstring::npos && pos > 0)
    {
        const std::wstring firstWord = str.substr(0, pos);

        if (firstWord.compare(L"uniform") == 0   ||
            firstWord.compare(L"attribute") == 0 ||
            firstWord.compare(L"varying") == 0)
        {
            return true;
        }
    }

    return false;
}


} // namespace cvf
