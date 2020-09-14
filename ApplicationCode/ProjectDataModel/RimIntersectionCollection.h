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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class Rim3dView;
class RimExtrudedCurveIntersection;
class RimBoxIntersection;
class RimEclipseCellColors;
class RimSimWellInView;
class RivTernaryScalarMapper;

namespace cvf
{
class ModelBasicList;
class Transform;
class ScalarMapper;
} // namespace cvf

//==================================================================================================
//
//
//
//==================================================================================================
class RimIntersectionCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersectionCollection();
    ~RimIntersectionCollection() override;

    caf::PdmField<bool> isActive;

    void appendIntersectionAndUpdate( RimExtrudedCurveIntersection* intersection, bool allowActiveViewChange = true );
    void appendIntersectionNoUpdate( RimExtrudedCurveIntersection* intersection );

    void appendIntersectionBoxAndUpdate( RimBoxIntersection* intersectionBox );
    void appendIntersectionBoxNoUpdate( RimBoxIntersection* intersectionBox );

    bool hasActiveIntersectionForSimulationWell( const RimSimWellInView* simWell ) const;
    bool hasAnyActiveSeparateResults();

    void updateIntersectionBoxGeometry();

    void syncronize2dIntersectionViews();
    void scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
    void recomputeSimWellBranchData();

    // Visualization interface

    void applySingleColorEffect();
    void updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex );
    void appendPartsToModel( Rim3dView& view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void rebuildGeometry();

    std::vector<RimExtrudedCurveIntersection*> intersections() const;
    std::vector<RimBoxIntersection*>           intersectionBoxes() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;

private:
    caf::PdmChildArrayField<RimExtrudedCurveIntersection*> m_intersections;
    caf::PdmChildArrayField<RimBoxIntersection*>           m_intersectionBoxes;
};
