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

#include <vector>

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class ShaderSourceProvider : public Object
{
public:
    static ShaderSourceProvider* instance();

    String  getSourceFromRepository(ShaderSourceRepository::ShaderIdent shaderIdent);
    String  getSourceFromFile(String shaderName);

    void    setSourceRepository(ShaderSourceRepository* repository);
    void    addFileSearchDirectory(String directory);

private:
    static bool loadFile(const String& fullFileName, CharArray* fileContents);

    ShaderSourceProvider();
    virtual ~ShaderSourceProvider();

private:
    ref<ShaderSourceRepository> m_sourceRepository;   // Reference to the shader source repository
    std::vector<String>         m_searchDirectories;  // Array of directories to look in when creating shader from file
};

}
