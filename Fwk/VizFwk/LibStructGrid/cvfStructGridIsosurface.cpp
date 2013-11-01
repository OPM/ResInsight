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
#include "cvfStructGridIsosurface.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfDebugTimer.h"
#include "cvfPlane.h"
#include "cvfScalarMapper.h"
#include "cvfRectilinearGrid.h"

#include <map>

namespace cvf {



//==================================================================================================
///
/// \class cvf::StructGridIsosurface
/// \ingroup StructGrid
///
/// 
///
//==================================================================================================

// Based on description and implementation from Paul Bourke:
//   http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/

const uint StructGridIsosurface::sm_edgeTable[256] = 
{
    0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
    0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
    0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
    0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
    0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
    0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
    0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
    0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
    0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
    0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
    0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
    0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
    0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
    0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
    0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
    0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
    0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   
};

const int StructGridIsosurface::sm_triTable[256][16] = 
{
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};


//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
StructGridIsosurface::StructGridIsosurface(const RectilinearGrid* grid)
:   m_grid(grid),
    m_scalarSetIndex(0),
    m_isoValue(0),
    m_mapScalarSetIndex(UNDEFINED_UINT),
    m_scalarMapper(NULL)
{
    CVF_ASSERT(grid);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
StructGridIsosurface::~StructGridIsosurface()
{
}


//--------------------------------------------------------------------------------------------------
/// Set index of scalar set that should be used to generate the isosurface
/// 
/// \param scalarSetIndex  Index of the scalar set. Must be a valid index. The default index is 0.
//--------------------------------------------------------------------------------------------------
void StructGridIsosurface::setScalarSetIndex(uint scalarSetIndex)
{
    CVF_ASSERT(m_grid.notNull());
    CVF_ASSERT(scalarSetIndex < m_grid->scalarSetCount());

    m_scalarSetIndex = scalarSetIndex;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridIsosurface::setIsoValue(double value)
{
    m_isoValue = value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void StructGridIsosurface::setMapScalar(uint scalarSetIndex, const ScalarMapper* mapper)
{
    CVF_ASSERT(scalarSetIndex < m_grid->scalarSetCount());
    CVF_ASSERT(mapper);

    m_mapScalarSetIndex = scalarSetIndex;
    m_scalarMapper = mapper;
}


//--------------------------------------------------------------------------------------------------
/// Generate isosurface geometry based on grid point scalars (cell corner)
/// 
/// \return  Reference to created DrawableGeo object. Returns NULL if no isosurface was generated.
///          If a scalar mapper is defined, texture coords are generated based on scalar field used
///          to map onto the isosurface
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridIsosurface::generateSurface() const
{
    DebugTimer tim("StructGridIsosurface::generateSurface()", DebugTimer::DISABLED);

    uint cellCountI = m_grid->cellCountI();
    uint cellCountJ = m_grid->cellCountJ();
    uint cellCountK = m_grid->cellCountK();

    std::vector<Vec3f>  isoVertices;
    std::vector<double> isoVertexScalars;
    std::vector<uint>   isoIndices;

    bool useTextureMapping = mapScalar();

    // Maps from edge IDs in original grid to vertex indices in the generated isosurface
    std::map<size_t, size_t> edgeIdToIsoVertexIdxMap;


    // The indexing conventions for vertices and 
    // edges used in the algorithm:
    //                                                                   edg   verts
    //      4-------------5                     *------4------*           0    0 - 1
    //     /|            /|                    /|            /|           1    1 - 2
    //    / |           / |                  7/ |          5/ |           2    2 - 3
    //   /  |          /  |      |z          /  8          /  9           3    3 - 0
    //  7-------------6   |      | /y       *------6------*   |           4    4 - 5
    //  |   |         |   |      |/         |   |         |   |           5    5 - 6
    //  |   0---------|---1      *---x      |   *------0--|---*           6    6 - 7
    //  |  /          |  /                 11  /         10  /            7    7 - 4
    //  | /           | /                   | /3          | /1            8    0 - 4
    //  |/            |/                    |/            |/              9    1 - 5
    //  3-------------2                     *------2------*              10    2 - 6
    //  vertex indices                       edge indices                11    3 - 7
    //                                                                 


    GridCell cell;

    uint k;
    for (k = 0; k < cellCountK; k++)
    {
        uint j;
        for (j = 0; j < cellCountJ; j++)
        {
            uint i;
            for (i = 0; i < cellCountI; i++)
            {
                // Corner scalar values
                cell.val[0] = m_grid->gridPointScalar(m_scalarSetIndex, i,     j + 1, k);
                cell.val[1] = m_grid->gridPointScalar(m_scalarSetIndex, i + 1, j + 1, k);
                cell.val[2] = m_grid->gridPointScalar(m_scalarSetIndex, i + 1, j,     k);
                cell.val[3] = m_grid->gridPointScalar(m_scalarSetIndex, i,     j,     k);
                cell.val[4] = m_grid->gridPointScalar(m_scalarSetIndex, i,     j + 1, k + 1);
                cell.val[5] = m_grid->gridPointScalar(m_scalarSetIndex, i + 1, j + 1, k + 1);
                cell.val[6] = m_grid->gridPointScalar(m_scalarSetIndex, i + 1, j,     k + 1);
                cell.val[7] = m_grid->gridPointScalar(m_scalarSetIndex, i,     j,     k + 1);

                // Inexpensive optimization
                // Check if iso value intersects this cell before proceeding
                int aboveCount = 0;
                int belowCount = 0;
                int n;
                for (n = 0; n < 8; n++)
                {
                    if      (cell.val[n] < m_isoValue) belowCount++;
                    else if (cell.val[n] > m_isoValue) aboveCount++;
                }

                if (aboveCount == 8 || belowCount == 8)
                {
                    continue;
                }

                if (useTextureMapping)
                {
                    cell.mapVal[0] = m_grid->gridPointScalar(m_mapScalarSetIndex, i,     j + 1, k);
                    cell.mapVal[1] = m_grid->gridPointScalar(m_mapScalarSetIndex, i + 1, j + 1, k);
                    cell.mapVal[2] = m_grid->gridPointScalar(m_mapScalarSetIndex, i + 1, j,     k);
                    cell.mapVal[3] = m_grid->gridPointScalar(m_mapScalarSetIndex, i,     j,     k);
                    cell.mapVal[4] = m_grid->gridPointScalar(m_mapScalarSetIndex, i,     j + 1, k + 1);
                    cell.mapVal[5] = m_grid->gridPointScalar(m_mapScalarSetIndex, i + 1, j + 1, k + 1);
                    cell.mapVal[6] = m_grid->gridPointScalar(m_mapScalarSetIndex, i + 1, j,     k + 1);
                    cell.mapVal[7] = m_grid->gridPointScalar(m_mapScalarSetIndex, i,     j,     k + 1);
                }

                // Corner coordinates
                cell.p[0] = m_grid->gridPointCoordinate(i,     j + 1, k);
                cell.p[1] = m_grid->gridPointCoordinate(i + 1, j + 1, k);
                cell.p[2] = m_grid->gridPointCoordinate(i + 1, j,     k);
                cell.p[3] = m_grid->gridPointCoordinate(i,     j,     k);
                cell.p[4] = m_grid->gridPointCoordinate(i,     j + 1, k + 1);
                cell.p[5] = m_grid->gridPointCoordinate(i + 1, j + 1, k + 1);
                cell.p[6] = m_grid->gridPointCoordinate(i + 1, j,     k + 1);
                cell.p[7] = m_grid->gridPointCoordinate(i,     j,     k + 1);

                // Edged IDs
                cell.eid[ 0] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k);
                cell.eid[ 1] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k)     + 1;
                cell.eid[ 2] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k);
                cell.eid[ 3] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k)     + 1;
                cell.eid[ 4] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k + 1);
                cell.eid[ 5] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k + 1) + 1;
                cell.eid[ 6] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k + 1);
                cell.eid[ 7] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k + 1) + 1;
                cell.eid[ 8] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k)     + 2;
                cell.eid[ 9] = 3*m_grid->gridPointIndexFromIJK(i + 1, j + 1, k)     + 2;
                cell.eid[10] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k)     + 2;
                cell.eid[11] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k)     + 2;


                Triangle triangles[5];
                int numTriangles = polygonise(m_isoValue, cell, triangles, useTextureMapping);

                for (n = 0; n < numTriangles; n++)
                {
                    int m;
                    for (m = 0; m < 3; m++)
                    {
                        size_t vertexIdx = 0;

                        size_t sourceEdgeId = triangles[n].peid[m];
                        std::map<size_t,size_t>::iterator it = edgeIdToIsoVertexIdxMap.find(sourceEdgeId);
                        if (it == edgeIdToIsoVertexIdxMap.end())
                        {
                            Vec3f v(triangles[n].p[m]);
                            isoVertices.push_back(v);

                            if (useTextureMapping)
                            {
                                isoVertexScalars.push_back(triangles[n].scalars[m]);
                            }

                            vertexIdx = isoVertices.size() - 1;
                            edgeIdToIsoVertexIdxMap[sourceEdgeId] = vertexIdx;
                        }
                        else
                        {
                            vertexIdx = it->second;
                        }

                        isoIndices.push_back(static_cast<uint>(vertexIdx));
                    }
                }
            }
        }
    }

    tim.reportTime("Time generating iso");
    //Trace::show("Vertices:%d  Conns:%d   Tris:%d", isoVertices.size(), isoIndices.size(), isoIndices.size()/3);

    if (isoVertices.size() > 0 && isoIndices.size() > 0)
    {
        GeometryBuilderDrawableGeo builder;
        builder.addVertices(Vec3fArray(isoVertices));
        builder.addTriangles(UIntArray(isoIndices));

        ref<cvf::DrawableGeo> geo = builder.drawableGeo();

        if (useTextureMapping)
        {
            ref<Color3ubArray> vertexColors = new Color3ubArray;
            ref<Vec2fArray> textureCoords = new Vec2fArray;
            vertexColors->reserve(isoVertices.size());
            textureCoords->reserve(isoVertices.size());
            size_t i;
            for (i = 0; i < isoVertices.size(); i++)
            {
                Color3ub clr = m_scalarMapper->mapToColor(isoVertexScalars[i]);
                vertexColors->add(clr);

                Vec2f texCoord = m_scalarMapper->mapToTextureCoord(isoVertexScalars[i]);
                textureCoords->add(texCoord);
            }

            geo->setColorArray(vertexColors.p());
            geo->setTextureCoordArray(textureCoords.p());
        }

        return geo;
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Generate isosurface geometry from current configuration
/// 
/// \return  Reference to created DrawableGeo object. Returns NULL if no isosurface was generated
/// 
/// \remarks Currently, the isosurface is generated only from cell center scalar values. 
/// \todo    Must implement special handling of the outer edges of the grid. Currently half of the
///          outer edge cells is ignored in the isosurface computation.
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> StructGridIsosurface::generateSurfaceCellCenterBased() const
{
    //DebugTimer tim;

    uint cellCountI = m_grid->cellCountI();
    uint cellCountJ = m_grid->cellCountJ();
    uint cellCountK = m_grid->cellCountK();

    std::vector<Vec3f> isoVertices;
    std::vector<uint> isoIndices;

    // Maps from edge IDs in original grid to vertex indices in the generated isosurface
    std::map<size_t, size_t> edgeIdToIsoVertexIdxMap;


    // The indexing conventions for vertices and 
    // edges used in the algorithm:
    //                                                                   edg   verts
    //      4-------------5                     *------4------*           0    0 - 1
    //     /|            /|                    /|            /|           1    1 - 2
    //    / |           / |                  7/ |          5/ |           2    2 - 3
    //   /  |          /  |      |z          /  8          /  9           3    3 - 0
    //  7-------------6   |      | /y       *------6------*   |           4    4 - 5
    //  |   |         |   |      |/         |   |         |   |           5    5 - 6
    //  |   0---------|---1      *---x      |   *------0--|---*           6    6 - 7
    //  |  /          |  /                 11  /         10  /            7    7 - 4
    //  | /           | /                   | /3          | /1            8    0 - 4
    //  |/            |/                    |/            |/              9    1 - 5
    //  3-------------2                     *------2------*              10    2 - 6
    //  vertex indices                       edge indices                11    3 - 7
    //                                                                 


    GridCell cell;
    const DoubleArray* scalarSet = m_grid->scalarSet(m_scalarSetIndex);
    CVF_ASSERT(scalarSet);

    uint k;
    for (k = 0; k < cellCountK - 1; k++)
    {
        uint j;
        for (j = 0; j < cellCountJ - 1; j++)
        {
            uint i;
            for (i = 0; i < cellCountI - 1; i++)
            {
                size_t cellIndex0 = m_grid->cellIndexFromIJK(i,     j + 1, k);
                size_t cellIndex1 = m_grid->cellIndexFromIJK(i + 1, j + 1, k);
                size_t cellIndex2 = m_grid->cellIndexFromIJK(i + 1, j,     k);
                size_t cellIndex3 = m_grid->cellIndexFromIJK(i,     j,     k);
                size_t cellIndex4 = m_grid->cellIndexFromIJK(i,     j + 1, k + 1);
                size_t cellIndex5 = m_grid->cellIndexFromIJK(i + 1, j + 1, k + 1);
                size_t cellIndex6 = m_grid->cellIndexFromIJK(i + 1, j,     k + 1);
                size_t cellIndex7 = m_grid->cellIndexFromIJK(i,     j,     k + 1);

                // Corner scalar values
                cell.val[0] = scalarSet->get(cellIndex0);
                cell.val[1] = scalarSet->get(cellIndex1);
                cell.val[2] = scalarSet->get(cellIndex2);
                cell.val[3] = scalarSet->get(cellIndex3);
                cell.val[4] = scalarSet->get(cellIndex4);
                cell.val[5] = scalarSet->get(cellIndex5);
                cell.val[6] = scalarSet->get(cellIndex6);
                cell.val[7] = scalarSet->get(cellIndex7);


                // Inexpensive optimization
                // Check if iso value intersects this cell before proceeding
                int aboveCount = 0;
                int belowCount = 0;
                int n;
                for (n = 0; n < 8; n++)
                {
                    if      (cell.val[n] < m_isoValue) belowCount++;
                    else if (cell.val[n] > m_isoValue) aboveCount++;
                }

                if (aboveCount == 8 || belowCount == 8)
                {
                    continue;
                }


                // Corner coordinates
                cell.p[0] = m_grid->cellCentroid(cellIndex0);
                cell.p[1] = m_grid->cellCentroid(cellIndex1);
                cell.p[2] = m_grid->cellCentroid(cellIndex2);
                cell.p[3] = m_grid->cellCentroid(cellIndex3);
                cell.p[4] = m_grid->cellCentroid(cellIndex4);
                cell.p[5] = m_grid->cellCentroid(cellIndex5);
                cell.p[6] = m_grid->cellCentroid(cellIndex6);
                cell.p[7] = m_grid->cellCentroid(cellIndex7);

                // Edged IDs
                cell.eid[ 0] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k);
                cell.eid[ 1] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k)     + 1;
                cell.eid[ 2] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k);
                cell.eid[ 3] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k)     + 1;
                cell.eid[ 4] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k + 1);
                cell.eid[ 5] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k + 1) + 1;
                cell.eid[ 6] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k + 1);
                cell.eid[ 7] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k + 1) + 1;
                cell.eid[ 8] = 3*m_grid->gridPointIndexFromIJK(i,     j + 1, k)     + 2;
                cell.eid[ 9] = 3*m_grid->gridPointIndexFromIJK(i + 1, j + 1, k)     + 2;
                cell.eid[10] = 3*m_grid->gridPointIndexFromIJK(i + 1, j,     k)     + 2;
                cell.eid[11] = 3*m_grid->gridPointIndexFromIJK(i,     j,     k)     + 2;


                Triangle triangles[5];
                int numTriangles = polygonise(m_isoValue, cell, triangles, false);

                for (n = 0; n < numTriangles; n++)
                {
                    int m;
                    for (m = 0; m < 3; m++)
                    {
                        size_t vertexIdx = 0;

                        size_t sourceEdgeId = triangles[n].peid[m];
                        std::map<size_t,size_t>::iterator it = edgeIdToIsoVertexIdxMap.find(sourceEdgeId);
                        if (it == edgeIdToIsoVertexIdxMap.end())
                        {
                            Vec3f v(triangles[n].p[m]);
                            isoVertices.push_back(v);
                            vertexIdx = isoVertices.size() - 1;
                            edgeIdToIsoVertexIdxMap[sourceEdgeId] = vertexIdx;
                        }
                        else
                        {
                            vertexIdx = it->second;
                        }

                        isoIndices.push_back(static_cast<uint>(vertexIdx));
                    }
                }
            }
        }
    }

    //tim.reportTime("Time generating iso");
    //Trace::show("Vertices:%d  Conns:%d   Tris:%d", isoVertices.size(), isoIndices.size(), isoIndices.size()/3);

    if (isoVertices.size() > 0 && isoIndices.size() > 0)
    {
        GeometryBuilderDrawableGeo builder;
        builder.addVertices(Vec3fArray(isoVertices));
        builder.addTriangles(UIntArray(isoIndices));

        return builder.drawableGeo();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Given a cell and an isolevel, calculate the triangular facets to represent the isosurface through the cell
/// 
/// \return Return the number of triangular facets. 0 will be returned if the grid cell is either 
///         totally above of totally below the isolevel.
/// 
/// The array "triangles" will be loaded up with the vertices at most 5 triangular facets.
//--------------------------------------------------------------------------------------------------
int StructGridIsosurface::polygonise(double isolevel, const GridCell& grid, Triangle triangles[5], bool useTextureMapping)
{
    // Determine the index into the edge table which tells us which vertices are inside of the surface
    // Calculate from those vertices which are below the isolevel.
    int cubeindex = 0;
    if (grid.val[0] < isolevel) cubeindex |= 1;
    if (grid.val[1] < isolevel) cubeindex |= 2;
    if (grid.val[2] < isolevel) cubeindex |= 4;
    if (grid.val[3] < isolevel) cubeindex |= 8;
    if (grid.val[4] < isolevel) cubeindex |= 16;
    if (grid.val[5] < isolevel) cubeindex |= 32;
    if (grid.val[6] < isolevel) cubeindex |= 64;
    if (grid.val[7] < isolevel) cubeindex |= 128;

    // Cube is entirely in/out of the surface 
    if (sm_edgeTable[cubeindex] == 0)
    {
        return 0;
    }

    // Find the vertices where the surface intersects the cube 
    Vec3d vertlist[12];
    double mapScalars[12];
    if (useTextureMapping)
    {
        if (sm_edgeTable[cubeindex] & 1)    vertlist[ 0] =  vertexInterpolateWithMapping(isolevel, grid.p[0], grid.p[1], grid.val[0], grid.val[1], grid.mapVal[0], grid.mapVal[1], &mapScalars[ 0]);
        if (sm_edgeTable[cubeindex] & 2)    vertlist[ 1] =  vertexInterpolateWithMapping(isolevel, grid.p[1], grid.p[2], grid.val[1], grid.val[2], grid.mapVal[1], grid.mapVal[2], &mapScalars[ 1]);
        if (sm_edgeTable[cubeindex] & 4)    vertlist[ 2] =  vertexInterpolateWithMapping(isolevel, grid.p[2], grid.p[3], grid.val[2], grid.val[3], grid.mapVal[2], grid.mapVal[3], &mapScalars[ 2]);
        if (sm_edgeTable[cubeindex] & 8)    vertlist[ 3] =  vertexInterpolateWithMapping(isolevel, grid.p[3], grid.p[0], grid.val[3], grid.val[0], grid.mapVal[3], grid.mapVal[0], &mapScalars[ 3]);
        if (sm_edgeTable[cubeindex] & 16)   vertlist[ 4] =  vertexInterpolateWithMapping(isolevel, grid.p[4], grid.p[5], grid.val[4], grid.val[5], grid.mapVal[4], grid.mapVal[5], &mapScalars[ 4]);
        if (sm_edgeTable[cubeindex] & 32)   vertlist[ 5] =  vertexInterpolateWithMapping(isolevel, grid.p[5], grid.p[6], grid.val[5], grid.val[6], grid.mapVal[5], grid.mapVal[6], &mapScalars[ 5]);
        if (sm_edgeTable[cubeindex] & 64)   vertlist[ 6] =  vertexInterpolateWithMapping(isolevel, grid.p[6], grid.p[7], grid.val[6], grid.val[7], grid.mapVal[6], grid.mapVal[7], &mapScalars[ 6]);
        if (sm_edgeTable[cubeindex] & 128)  vertlist[ 7] =  vertexInterpolateWithMapping(isolevel, grid.p[7], grid.p[4], grid.val[7], grid.val[4], grid.mapVal[7], grid.mapVal[4], &mapScalars[ 7]);
        if (sm_edgeTable[cubeindex] & 256)  vertlist[ 8] =  vertexInterpolateWithMapping(isolevel, grid.p[0], grid.p[4], grid.val[0], grid.val[4], grid.mapVal[0], grid.mapVal[4], &mapScalars[ 8]);
        if (sm_edgeTable[cubeindex] & 512)  vertlist[ 9] =  vertexInterpolateWithMapping(isolevel, grid.p[1], grid.p[5], grid.val[1], grid.val[5], grid.mapVal[1], grid.mapVal[5], &mapScalars[ 9]);
        if (sm_edgeTable[cubeindex] & 1024) vertlist[10] =  vertexInterpolateWithMapping(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6], grid.mapVal[2], grid.mapVal[6], &mapScalars[10]);
        if (sm_edgeTable[cubeindex] & 2048) vertlist[11] =  vertexInterpolateWithMapping(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7], grid.mapVal[3], grid.mapVal[7], &mapScalars[11]);
    }
    else
    {
        if (sm_edgeTable[cubeindex] & 1)    vertlist[0] =   vertexInterpolate(isolevel, grid.p[0], grid.p[1], grid.val[0], grid.val[1]);
        if (sm_edgeTable[cubeindex] & 2)    vertlist[1] =   vertexInterpolate(isolevel, grid.p[1], grid.p[2], grid.val[1], grid.val[2]);
        if (sm_edgeTable[cubeindex] & 4)    vertlist[2] =   vertexInterpolate(isolevel, grid.p[2], grid.p[3], grid.val[2], grid.val[3]);
        if (sm_edgeTable[cubeindex] & 8)    vertlist[3] =   vertexInterpolate(isolevel, grid.p[3], grid.p[0], grid.val[3], grid.val[0]);
        if (sm_edgeTable[cubeindex] & 16)   vertlist[4] =   vertexInterpolate(isolevel, grid.p[4], grid.p[5], grid.val[4], grid.val[5]);
        if (sm_edgeTable[cubeindex] & 32)   vertlist[5] =   vertexInterpolate(isolevel, grid.p[5], grid.p[6], grid.val[5], grid.val[6]);
        if (sm_edgeTable[cubeindex] & 64)   vertlist[6] =   vertexInterpolate(isolevel, grid.p[6], grid.p[7], grid.val[6], grid.val[7]);
        if (sm_edgeTable[cubeindex] & 128)  vertlist[7] =   vertexInterpolate(isolevel, grid.p[7], grid.p[4], grid.val[7], grid.val[4]);
        if (sm_edgeTable[cubeindex] & 256)  vertlist[8] =   vertexInterpolate(isolevel, grid.p[0], grid.p[4], grid.val[0], grid.val[4]);
        if (sm_edgeTable[cubeindex] & 512)  vertlist[9] =   vertexInterpolate(isolevel, grid.p[1], grid.p[5], grid.val[1], grid.val[5]);
        if (sm_edgeTable[cubeindex] & 1024) vertlist[10] =  vertexInterpolate(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6]);
        if (sm_edgeTable[cubeindex] & 2048) vertlist[11] =  vertexInterpolate(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7]);
    }



    // Create the triangles
    int ntriang = 0;
    int i;
    for (i = 0; sm_triTable[cubeindex][i] != -1; i += 3) 
    {
        triangles[ntriang].p[0] = vertlist[sm_triTable[cubeindex][i  ]];
        triangles[ntriang].p[1] = vertlist[sm_triTable[cubeindex][i+1]];
        triangles[ntriang].p[2] = vertlist[sm_triTable[cubeindex][i+2]];

        triangles[ntriang].peid[0] = grid.eid[sm_triTable[cubeindex][i  ]];
        triangles[ntriang].peid[1] = grid.eid[sm_triTable[cubeindex][i+1]];
        triangles[ntriang].peid[2] = grid.eid[sm_triTable[cubeindex][i+2]];

        if (useTextureMapping)
        {
            triangles[ntriang].scalars[0] = mapScalars[sm_triTable[cubeindex][i  ]];
            triangles[ntriang].scalars[1] = mapScalars[sm_triTable[cubeindex][i+1]];
            triangles[ntriang].scalars[2] = mapScalars[sm_triTable[cubeindex][i+2]];
        }

        ntriang++;
    }

    return ntriang;
}


//--------------------------------------------------------------------------------------------------
/// Linearly interpolate the position where an isosurface cuts an edge between two vertices, each with their own scalar value
//--------------------------------------------------------------------------------------------------
Vec3d StructGridIsosurface::vertexInterpolate(double isoLevel, const Vec3d& p1, const Vec3d& p2, double valp1, double valp2)
{
    if (valp1 == valp2)
    {
        return p1;
    }
    else
    {
        double t = (isoLevel - valp1)/(valp2 - valp1);
        CVF_ASSERT(t >= 0.0 && t <= 1.0);
        if (t > 0.0 && t < 1.0)
        {
            Vec3d p;
            p.x() = p1.x() + t * (p2.x() - p1.x());
            p.y() = p1.y() + t * (p2.y() - p1.y());
            p.z() = p1.z() + t * (p2.z() - p1.z());

            return p;
        }
        else
        {
            if (t >= 1.0)
            {
                return p2;
            }
            else
            {
                return p1;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Linearly interpolate the position where an isosurface cuts an edge between two vertices, each with their own scalar value.
/// Also compute the mapping scalar value computed position
//--------------------------------------------------------------------------------------------------
Vec3d StructGridIsosurface::vertexInterpolateWithMapping(double isoLevel, const Vec3d& p1, const Vec3d& p2, const double valp1, const double valp2, const double mapValP1, const double mapValP2, double* mapVal)
{
    if (valp1 == valp2)
    {
        *mapVal = mapValP1;
        return p1;
    }
    else
    {
        double t = (isoLevel - valp1)/(valp2 - valp1);
        CVF_ASSERT(t >= 0.0 && t <= 1.0);
        if (t > 0.0 && t < 1.0)
        {
            Vec3d p;
            p.x() = p1.x() + t * (p2.x() - p1.x());
            p.y() = p1.y() + t * (p2.y() - p1.y());
            p.z() = p1.z() + t * (p2.z() - p1.z());
            
            *mapVal = mapValP1 + t * (mapValP2 - mapValP1);
            return p;
        }
        else
        {
            if (t >= 1.0)
            {
                *mapVal = mapValP2;
                return p2;
            }
            else
            {
                *mapVal = mapValP1;
                return p1;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// In order to get texture mapping, map scalar set index and scalar mapper object must be defined,
/// and the map scalar set index must be different to the scalar index used as basis for the iso surface
//--------------------------------------------------------------------------------------------------
bool StructGridIsosurface::mapScalar() const
{
    if (m_mapScalarSetIndex == UNDEFINED_UINT)
    {
        return false;
    }
    
    if (m_mapScalarSetIndex == m_scalarSetIndex)
    {
        return false;
    }

    if (m_scalarMapper.isNull())
    {
        return false;
    }

    return true;
}


} // namespace cvf

