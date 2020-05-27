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

#include "cvfArray.h"
#include "cvfObject.h"

#include "cafEffectGenerator.h"
#include "cafPdmPointer.h"

namespace cvf
{
class StructGridInterface;
class ModelBasicList;
class Transform;
class Part;
} // namespace cvf

class RimEclipseCellColors;
class RimCellEdgeColors;
class RimFaultInViewCollection;
class RigGridBase;
class RimFaultInViewCollection;
class RimFaultInView;
class RivFaultGeometryGenerator;
class RivNNCGeometryGenerator;

//==================================================================================================
///
///
//==================================================================================================

class RivFaultPartMgr : public cvf::Object
{
public:
    RivFaultPartMgr( const RigGridBase* grid, const RimFaultInViewCollection* rimFaultCollection, RimFaultInView* rimFault );

    void setCellVisibility( cvf::UByteArray* cellVisibilities );

    void applySingleColorEffect();
    void setOpacityLevel( float opacity ) { m_opacityLevel = opacity; }
    void updateCellResultColor( size_t timeStepIndex, RimEclipseCellColors* cellResultColors );
    void updateCellEdgeResultColor( size_t                timeStepIndex,
                                    RimEclipseCellColors* cellResultColors,
                                    RimCellEdgeColors*    cellEdgeResultColors );

    void appendNativeFaultFacesToModel( cvf::ModelBasicList* model );
    void appendOppositeFaultFacesToModel( cvf::ModelBasicList* model );
    void appendLabelPartsToModel( cvf::ModelBasicList* model );
    void appendMeshLinePartsToModel( cvf::ModelBasicList* model );

    void appendNativeNNCFacesToModel( cvf::ModelBasicList* model );
    void appendCompleteNNCFacesToModel( cvf::ModelBasicList* model );

private:
    void generatePartGeometry();

    void generateNativeNncPartGeometry();
    void generateAllNncPartGeometry();

    void clearFlags();

    void updatePartEffect();

    void updateNNCColors( size_t timeStepIndex, RimEclipseCellColors* cellResultColors );

    caf::FaceCulling faceCullingMode() const;

    void createLabelWithAnchorLine( const cvf::Part* part );

    static cvf::Vec3f findClosestVertex( const cvf::Vec3f& point, const cvf::Vec3fArray* vertices );

private:
    cvf::cref<RigGridBase>          m_grid;
    caf::PdmPointer<RimFaultInView> m_rimFault;
    const RimFaultInViewCollection* m_rimFaultCollection;

    float        m_opacityLevel;
    cvf::Color3f m_defaultColor;

    cvf::ref<cvf::UByteArray> m_cellVisibility;

    bool                                m_isNativeFaultsGenerated;
    cvf::ref<RivFaultGeometryGenerator> m_nativeFaultGenerator;
    cvf::ref<cvf::Part>                 m_nativeFaultFaces;
    cvf::ref<cvf::Part>                 m_nativeFaultGridLines;
    cvf::ref<cvf::Vec2fArray>           m_nativeFaultFacesTextureCoords;

    bool                                m_isOppositeFaultsGenerated;
    cvf::ref<RivFaultGeometryGenerator> m_oppositeFaultGenerator;
    cvf::ref<cvf::Part>                 m_oppositeFaultFaces;
    cvf::ref<cvf::Part>                 m_oppositeFaultGridLines;
    cvf::ref<cvf::Vec2fArray>           m_oppositeFaultFacesTextureCoords;

    bool                              m_isNativeNncsGenerated;
    cvf::ref<RivNNCGeometryGenerator> m_NNCGenerator;
    cvf::ref<cvf::Part>               m_NNCFaces;
    cvf::ref<cvf::Vec2fArray>         m_NNCTextureCoords;

    bool                              m_isAllNncsGenerated;
    cvf::ref<RivNNCGeometryGenerator> m_allanNNCGenerator;
    cvf::ref<cvf::Part>               m_allanNNCFaces;
    cvf::ref<cvf::Vec2fArray>         m_allanNNCTextureCoords;

    cvf::ref<cvf::Part> m_faultLabelPart;
    cvf::ref<cvf::Part> m_faultLabelLinePart;
};
