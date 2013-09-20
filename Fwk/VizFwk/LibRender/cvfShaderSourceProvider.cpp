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
#include "cvfShaderSourceProvider.h"
#include "cvfShaderProgram.h"

#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

namespace cvf {


//==================================================================================================
///
/// \class cvf::ShaderSourceProvider
/// \ingroup Render
///
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Private constructor
//--------------------------------------------------------------------------------------------------
ShaderSourceProvider::ShaderSourceProvider()
{
    m_sourceRepository = new ShaderSourceRepository;
}


//--------------------------------------------------------------------------------------------------
/// Private destructor
//--------------------------------------------------------------------------------------------------
ShaderSourceProvider::~ShaderSourceProvider()
{
}


//--------------------------------------------------------------------------------------------------
/// Get a pointer to the singleton instance.
//--------------------------------------------------------------------------------------------------
ShaderSourceProvider* ShaderSourceProvider::instance()
{
    static ref<ShaderSourceProvider> staticInstance = new ShaderSourceProvider;

    return staticInstance.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ShaderSourceProvider::getSourceFromRepository(ShaderSourceRepository::ShaderIdent shaderIdent)
{
    CVF_ASSERT(m_sourceRepository.notNull());

    return m_sourceRepository->shaderSource(shaderIdent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ShaderSourceProvider::getSourceFromFile(String shaderName)
{
    const String fileName = shaderName + ".glsl";

    size_t i;
    for (i = 0; i < m_searchDirectories.size(); i++)
    {
        String fullPath = m_searchDirectories[i] + fileName;

        CharArray fileContents;
        if (loadFile(fullPath, &fileContents))
        {
            return String(fileContents.ptr());
        }
    }

    return "";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderSourceProvider::setSourceRepository(ShaderSourceRepository* repository)
{
    m_sourceRepository = repository;
}


//--------------------------------------------------------------------------------------------------
/// Add directory to list of search directories
///
/// \param directory Directory with trailing slash
//--------------------------------------------------------------------------------------------------
void ShaderSourceProvider::addFileSearchDirectory(String directory)
{
    m_searchDirectories.push_back(directory);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderSourceProvider::loadFile(const String& fullFileName, CharArray* fileContents)
{
#ifdef WIN32
    std::ifstream file(fullFileName.c_str());
#else
    std::ifstream file(fullFileName.toUtf8().ptr());
#endif

    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (file.good())
    {
        getline(file, line);

        size_t c;
        for (c = 0; c < line.length(); c++)
        {
            fileContents->push_back(line[c]);
        }

        fileContents->push_back('\n');
    }

    file.close();

    return true;
}


} // namespace cvf

