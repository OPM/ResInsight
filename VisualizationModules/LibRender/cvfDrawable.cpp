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

#include "cvfBase.h"
#include "cvfDrawable.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Drawable
/// \ingroup Render
///
/// Drawable is the base class for drawable items in a part. A part has one drawable for each lod level.
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Drawable::Drawable()
{

}


//--------------------------------------------------------------------------------------------------
/// \fn virtual size_t Drawable::vertexCount() const = 0;
/// 
/// Get the number of vertices (nodes, points) in the drawable
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// \fn virtual size_t Drawable::triangleCount() const = 0;
/// 
/// Get the number of triangles in the drawable. A quad is not 2 triangles.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// \fn virtual size_t Drawable::faceCount() const = 0;
/// 
/// Get the total number of OpenGL primitives (sum of lines, points, quads, etc.) 
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// \fn virtual bool Drawable::rayIntersect(const Ray& ray, Vec3d* intersectionPoint) const = 0;
/// 
/// Intersect the drawable with the ray and return the closest intersection point.
///
/// Returns true if anything was hit.
//--------------------------------------------------------------------------------------------------


} // namespace cvf

