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
#include "cvfBase.h"
#include "cvfObject.h"

//#include "RimCrossSection.h"
#include "RivCrossSectionGeometryGenerator.h"
#include "cvfColor4.h"

namespace cvf
{
    class ModelBasicList;
    class Transform;
    class Part;
}

class RimEclipseCellColors;
class RimCellEdgeColors;
class RimCrossSectionCollection;
class RimCrossSection;

//==================================================================================================
///
///
//==================================================================================================

class RivCrossSectionPartMgr : public cvf::Object
{
public:
    RivCrossSectionPartMgr(const RigMainGrid* grid, 
                          const RimCrossSectionCollection* rimCrossSectionCollection, 
                          const RimCrossSection* rimCrossSection);

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors);

    void appendNativeCrossSectionFacesToModel(cvf::ModelBasicList* model);
    void appendMeshLinePartsToModel(cvf::ModelBasicList* model);

private:
    void updatePartEffect();
    void generatePartGeometry();

    cvf::cref<RigMainGrid>      m_grid;
    const RimCrossSection*             m_rimCrossSection;
    const RimCrossSectionCollection*   m_rimCrossSectionCollection;

    cvf::Color3f                m_defaultColor;

    cvf::ref<RivCrossSectionGeometryGenerator>   m_nativeCrossSectionGenerator;
    cvf::ref<cvf::Part>         m_nativeCrossSectionFaces;
    cvf::ref<cvf::Part>         m_nativeCrossSectionGridLines;
    cvf::ref<cvf::Vec2fArray>   m_nativeCrossSectionFacesTextureCoords;

};
