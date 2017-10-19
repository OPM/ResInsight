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

#include "RiaDefines.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"

namespace cvf
{
    class ScalarMapper;
    class DrawableGeo;
}

class RigNNCData;
class RigGridBase;

//==================================================================================================
///
///
//==================================================================================================

class RivNNCGeometryGenerator : public cvf::Object
{
public:
    RivNNCGeometryGenerator(const RigNNCData* nncData, const cvf::Vec3d& offset, const cvf::Array<size_t>* nncIndexes );
    ~RivNNCGeometryGenerator();

    void setCellVisibility( const cvf::UByteArray* cellVisibilities, const RigGridBase * grid);
    
    void textureCoordinates(cvf::Vec2fArray* textureCoords,  
                            const cvf::ScalarMapper* mapper,
                            RiaDefines::ResultCatType resultType,
                            size_t scalarResultIndex,
                            size_t nativeTimeStepIndex) const;

    // Mapping between cells and geometry
    cvf::ref<cvf::Array<size_t> >   triangleToNNCIndex() const;

    // Generated geometry
    cvf::ref<cvf::DrawableGeo>    generateSurface();

private:
    void computeArrays();

private:
    // Input
    cvf::cref<RigNNCData>               m_nncData;
    cvf::cref<cvf::Array<size_t> >      m_nncIndexes;
    cvf::cref<cvf::UByteArray>          m_cellVisibility;
    cvf::cref<RigGridBase>              m_grid;
    cvf::Vec3d                          m_offset;
    // Triangles
    cvf::ref<cvf::Vec3fArray>           m_vertices;

    // Mappings
    cvf::ref<cvf::Array<size_t> >       m_triangleIndexToNNCIndex;
};
