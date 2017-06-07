/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RigCellGeometryTools.h"

#include "cvfStructGrid.h"
#include "cvfGeometryTools.h"


//==================================================================================================
/// 
//==================================================================================================
void RigCellGeometryTools::findCellLocalXYZ(const std::array<cvf::Vec3d, 8>& hexCorners, cvf::Vec3d& localXdirection, cvf::Vec3d& localYdirection, cvf::Vec3d& localZdirection)
{
    cvf::ubyte faceVertexIndices[4];
    cvf::StructGridInterface::FaceEnum face;

    face = cvf::StructGridInterface::NEG_I;
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);
    cvf::Vec3d faceCenterNegI = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);
    //TODO: Should we use face centroids instead of face centers?

    face = cvf::StructGridInterface::POS_I;
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);
    cvf::Vec3d faceCenterPosI = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

    face = cvf::StructGridInterface::NEG_J;
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);
    cvf::Vec3d faceCenterNegJ = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

    face = cvf::StructGridInterface::POS_J;
    cvf::StructGridInterface::cellFaceVertexIndices(face, faceVertexIndices);
    cvf::Vec3d faceCenterPosJ = cvf::GeometryTools::computeFaceCenter(hexCorners[faceVertexIndices[0]], hexCorners[faceVertexIndices[1]], hexCorners[faceVertexIndices[2]], hexCorners[faceVertexIndices[3]]);

    cvf::Vec3d faceCenterCenterVectorI = faceCenterPosI - faceCenterNegI;
    cvf::Vec3d faceCenterCenterVectorJ = faceCenterPosJ - faceCenterNegJ;

    localZdirection.cross(faceCenterCenterVectorI, faceCenterCenterVectorJ);
    localZdirection.normalize();

    cvf::Vec3d crossPoductJZ;
    crossPoductJZ.cross(faceCenterCenterVectorJ, localZdirection);
    localXdirection = faceCenterCenterVectorI + crossPoductJZ;
    localXdirection.normalize();

    cvf::Vec3d crossPoductIZ;
    crossPoductIZ.cross(faceCenterCenterVectorI, localZdirection);
    localYdirection = faceCenterCenterVectorJ - crossPoductIZ;
    localYdirection.normalize();

    //TODO: Check if we end up with 0-vectors, and handle this case...
}
