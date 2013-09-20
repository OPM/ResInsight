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
#include "cvfShaderSourceRepositoryFile.h"

#include <fstream>

namespace cvf {


//==================================================================================================
///
/// \class cvf::ShaderSourceRepositoryFile
/// \ingroup Render
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
///
/// \param directory Directory with trailing slash
//--------------------------------------------------------------------------------------------------
ShaderSourceRepositoryFile::ShaderSourceRepositoryFile(const String& directory)
:   m_directory(directory)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderSourceRepositoryFile::rawShaderSource(ShaderIdent shaderIdent, CharArray* rawSource)
{
    String fullFileName = fileNameFromIdent(shaderIdent);

#ifdef WIN32
    std::ifstream file(fullFileName.c_str());
#else
    std::ifstream file(fullFileName.toUtf8().ptr());
#endif

    if (file.is_open())
    {
        std::string line;
        while (file.good())
        {
            getline(file, line);

            size_t c;
            for (c = 0; c < line.length(); c++)
            {
                rawSource->push_back(line[c]);
            }

            rawSource->push_back('\n');
        }

        file.close();

        return true;
    }

    return ShaderSourceRepository::rawShaderSource(shaderIdent, rawSource);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
String ShaderSourceRepositoryFile::fileNameFromIdent(ShaderIdent shaderIdent)
{
    String fileName = m_directory + shaderIdentString(shaderIdent) + ".glsl";
    return fileName;
}


} // namespace cvf
