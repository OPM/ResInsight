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


// Doxygen module definition
/// \ingroup CeeVizFramework
/// @{
/// \defgroup Geometry Geometry module
/// @}

#include "cvfArrowGenerator.h"
#include "cvfBoundingBox.h"
#include "cvfBoxGenerator.h"
#include "cvfEdgeKey.h"
#include "cvfFrustum.h"
#include "cvfGeometryBuilder.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfOutlineEdgeExtractor.h"
#include "cvfPatchGenerator.h"
#include "cvfRay.h"
#include "cvfTriangleMeshEdgeExtractor.h"
#include "cvfTriangleVertexSplitter.h"
#include "cvfVertexCompactor.h"
#include "cvfVertexWelder.h"

