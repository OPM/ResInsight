/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#pragma once

#include "cvfBase.h"
#include "cvfArray.h"
#include "cvfPart.h"
#include "cvfColor3.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf {
    class DrawableGeo;
    class ScalarMapper;
}

class RivObjectSourceInfo;

class RivPipeGeometryGenerator : public cvf::Object
{
public:
    RivPipeGeometryGenerator();
    ~RivPipeGeometryGenerator();

    // Coordinates and orientations
    void setPipeCenterCoords(const cvf::Vec3dArray* coords);
        
    // Pipe bends with a opening angle below given angle is modified with extra bend coordinates
    void setMinimumBendAngle(double degrees);
    
    // Scaling factor used to control how far from original pipe position the extra bend coordinates are located
    // This will affect how sharp or smooth bend will appear
    void setBendScalingFactor(double scaleFactor);

    // Appearance
    void setRadius(double radius);
    void setCrossSectionVertexCount(size_t vertexCount);
    void setPipeColor(cvf::Color3f val) { m_pipeColor = val; }

    cvf::ref<cvf::DrawableGeo> createPipeSurface();
    cvf::ref<cvf::DrawableGeo> createCenterLine();

    void pipeSurfaceTextureCoords(cvf::Vec2fArray* textureCoords, const std::vector<double>& segmentResults, const cvf::ScalarMapper* mapper) const;
    void centerlineTextureCoords(cvf::Vec2fArray* textureCoords, const std::vector<double>& segmentResults, const cvf::ScalarMapper* mapper) const;

    void    setFirstVisibleSegmentIndex(size_t segmentIndex);
    size_t  pipeSegmentIndexFromTriangleIndex(size_t triangleIndex) const;

    void cylinderWithCenterLineParts(cvf::Collection<cvf::Part>* destinationParts, const std::vector<cvf::Vec3d>& centerCoords, const cvf::Color3f& color, double radius);
private:
    void clearComputedData();
    void updateFilteredPipeCenterCoords();

    size_t findFirstSegmentWithLenght(double squareDistanceTolerance);

    static void computeCircle(double radius, size_t tesselationCount, const cvf::Vec3d& center, const cvf::Vec3d& orient1, const cvf::Vec3d& orient2, std::vector<cvf::Vec3d>* nodes);

    static cvf::ref<cvf::DrawableGeo> generateLine(const cvf::Vec3dArray* coords);
    static cvf::ref<cvf::DrawableGeo> generateExtrudedCylinder(double radius, size_t crossSectionNodeCount,const cvf::Vec3dArray* cylinderCenterCoords);

    static void computeExtrudedCoordsAndNormals(cvf::Vec3d intersectionCoord,
                                                cvf::Vec3d intersectionPlaneNormal,
                                                cvf::Vec3d segmentDirection,
                                                size_t crossSectionNodeCount,
                                                std::vector<cvf::Vec3d>* extrudedNodes,
                                                std::vector<cvf::Vec3f>* crossSectionVertices,
                                                std::vector<cvf::Vec3f>* cylinderSegmentNormals);
private:
    cvf::cref<cvf::Vec3dArray>  m_originalPipeCenterCoords;
    
    // Based on m_originalPipeCenterCoords, produce list of coords where coords at the same location is removed
    // When a bend is detected, extra bend coordinates are inserted
    std::vector<cvf::Vec3d>     m_filteredPipeCenterCoords;
    
    // Map from generated cylinder segments to pipe result indices
    std::vector<size_t>         m_filteredPipeSegmentToResult;

    size_t                      m_firstVisibleSegmentIndex;

    double                      m_radius;
    double                      m_minimumBendAngle;
    double                      m_bendScalingFactor;
    size_t                      m_crossSectionNodeCount;
    cvf::Color3f                m_pipeColor;
};
