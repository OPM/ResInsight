/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cvfDrawableGeo.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf
{
class Part;
}

class RigEclipseCaseData;
class RigGridBase;
class RimGeoMechCase;

//==================================================================================================
///
///
//==================================================================================================
class RivSingleCellPartGenerator
{
public:
    RivSingleCellPartGenerator( RigEclipseCaseData* rigCaseData, size_t gridIndex, size_t cellIndex, const cvf::Vec3d& displayModelOffset );
    RivSingleCellPartGenerator( RimGeoMechCase* rimGeoMechCase, size_t gridIndex, size_t cellIndex, const cvf::Vec3d& displayModelOffset );

    void setShowLgrMeshLines( bool enable );
    void setDisplacementData( double scaleFactor, const std::vector<cvf::Vec3f>& displacements );

    cvf::ref<cvf::Part>               createPart( const cvf::Color3f color );
    static cvf::ref<cvf::DrawableGeo> createMeshLinesOfParentGridCells( RigGridBase const*   grid,
                                                                        std::vector<size_t>& localGridCellIndices,
                                                                        const cvf::Vec3d&    displayModelOffset );

private:
    cvf::ref<cvf::DrawableGeo> createMeshDrawable();
    cvf::ref<cvf::DrawableGeo> createMeshDrawableFromLgrGridCells();

private:
    RigEclipseCaseData*     m_rigCaseData;
    RimGeoMechCase*         m_geoMechCase;
    size_t                  m_gridIndex;
    size_t                  m_cellIndex;
    cvf::Vec3d              m_displayModelOffset;
    bool                    m_showLgrMeshLines{ false };
    double                  m_displacementScaleFactor{ 1.0 };
    std::vector<cvf::Vec3f> m_displacements;
};
