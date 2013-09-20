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



// Define this one to tell windows.h to not define min() and max() as macros
#if defined WIN32 && !defined NOMINMAX
#define NOMINMAX
#endif


// Used to keep track of all instances of classes derived from Object.
// Can be used to detect memory leaks in combination with the static member 
// functions: Object::activeObjectInstances() and Object::dumpActiveObjectInstances()
//  0 - disable tracking of active object instances
//  1 - track active instances in debug builds
//  2 - track active instances in BOTH debug and release builds
#ifndef CVF_TRACK_ACTIVE_OBJECT_INSTANCES
#define CVF_TRACK_ACTIVE_OBJECT_INSTANCES  0
#endif


// Behavior of assert macros
// In debug builds, all asserts are in action, including tight asserts
// In release builds, tight asserts are disabled by default. Normal asserts are still in action (including CVF_FAIL_...)

#ifndef CVF_ENABLE_ASSERTS
#define CVF_ENABLE_ASSERTS 1
#endif


#ifndef CVF_ENABLE_TIGHT_ASSERTS
#ifdef _DEBUG
#define CVF_ENABLE_TIGHT_ASSERTS 1
#else
#define CVF_ENABLE_TIGHT_ASSERTS 0
#endif
#endif


