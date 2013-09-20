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

namespace cvf {


// Define aliases for the OpenGL types so we can utilize them without including
// the actual OpenGL headers all over the place.
typedef unsigned int    cvfGLenum;
typedef unsigned char   cvfGLboolean;
typedef unsigned int    cvfGLbitfield;
typedef signed char     cvfGLbyte;         // 1-byte signed 
typedef short           cvfGLshort;        // 2-byte signed 
typedef int             cvfGLint;          // 4-byte signed 
typedef int             cvfGLsizei;        // 4-byte signed 
typedef unsigned char   cvfGLubyte;        // 1-byte unsigned 
typedef unsigned short  cvfGLushort;       // 2-byte unsigned 
typedef unsigned int    cvfGLuint;         // 4-byte unsigned 
typedef float           cvfGLfloat;        // single precision float 
typedef float           cvfGLclampf;       // single precision float in [0,1] 
typedef double          cvfGLdouble;       // double precision float 
typedef double          cvfGLclampd;       // double precision float in [0,1] 
typedef void            cvfGLvoid;


// Type used to hold OpenGL identificators (object names)
// Typically the return values from glCreateShader() etc
// May eventually wrap this up in a class
typedef unsigned int OglId;


}
