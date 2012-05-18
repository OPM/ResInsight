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

#pragma once

#include "cvfString.h"
#include "cvfShaderSourceRepository.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class ShaderSourceRepositoryFile : public ShaderSourceRepository
{
public:
    ShaderSourceRepositoryFile(const String& directory);

private:
    virtual bool    rawShaderSource(ShaderIdent shaderIdent, CharArray* rawSource);
    String          fileNameFromIdent(ShaderIdent shaderIdent);

private:
    String  m_directory;
};

}
