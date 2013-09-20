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


// Doxygen module definition
/// \ingroup VizFramework
/// @{
/// \defgroup Core Core module
/// @}

/// \defgroup VizFramework Framework Basis

// Intentionally on top to be included first
#include "cvfBase.h"

#include "cvfArray.h"
#include "cvfAssert.h"
#include "cvfBase64.h"
#include "cvfCharArray.h"
#include "cvfCollection.h"
#include "cvfColor3.h"
#include "cvfColor4.h"
#include "cvfDebugTimer.h"
#include "cvfFlags.h"
#include "cvfFunctorRange.h"
#include "cvfLogger.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfPlane.h"
#include "cvfRect.h"
#include "cvfQuat.h"
#include "cvfString.h"
#include "cvfSystem.h"
#include "cvfTBBControl.h"
#include "cvfTimer.h"
#include "cvfTrace.h"
#include "cvfVector2.h"
#include "cvfVector3.h"
#include "cvfVector4.h"
#include "cvfVersion.h"
