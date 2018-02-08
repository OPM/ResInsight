/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RivFemPartGeometryGenerator.h"

namespace cvf
{
    class StructGridInterface;
    class ModelBasicList;
    class Transform;
    class Part;
    class Effect;
}

class RimGeoMechCellColors;
class RigFemPart;


//==================================================================================================
///
/// RivGridGeometry: Class to handle visualization structures that embodies a specific grid at a specific time step.
/// frame on a certain level
/// LGR's have their own instance and the parent grid as well
///
//==================================================================================================

class RivFemPartPartMgr: public cvf::Object
{
public:
    explicit RivFemPartPartMgr(const RigFemPart* femPart);
    ~RivFemPartPartMgr();
    void                        setTransform(cvf::Transform* scaleTransform);
    void                        setCellVisibility(cvf::UByteArray* cellVisibilities );
    cvf::ref<cvf::UByteArray>   cellVisibility() { return  m_cellVisibility;}
                                
    void                        updateCellColor(cvf::Color4f color);
    void                        updateCellResultColor(size_t timeStepIndex, RimGeoMechCellColors* cellResultColors);
                                
    void                        appendPartsToModel(cvf::ModelBasicList* model);

    const RivFemPartGeometryGenerator* surfaceGenerator() const;
                                
private:                        
    void                        generatePartGeometry(RivFemPartGeometryGenerator& geoBuilder);

private:
    int                         m_gridIdx;
    cvf::cref<RigFemPart>       m_grid;

    cvf::ref<cvf::Transform>    m_scaleTransform;
    float                       m_opacityLevel;
    cvf::Color3f                m_defaultColor;

    // Surface visualization
    RivFemPartGeometryGenerator m_surfaceGenerator;

    cvf::ref<cvf::Part>         m_surfaceFaces;
    cvf::ref<cvf::Vec2fArray>   m_surfaceFacesTextureCoords;

    cvf::ref<cvf::Part>         m_surfaceGridLines;

    cvf::ref<cvf::UByteArray>   m_cellVisibility;
};
