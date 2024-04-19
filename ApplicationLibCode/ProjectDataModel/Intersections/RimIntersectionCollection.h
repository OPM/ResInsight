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

#include "cvfArray.h"

#include "RimIntersectionEnums.h"

class Rim3dView;
class RimEclipseView;
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

    void appendIntersectionAndUpdate( RimExtrudedCurveIntersection* intersection, bool allowActiveViewChange = true );
    void appendIntersectionNoUpdate( RimExtrudedCurveIntersection* intersection );

    void appendIntersectionBoxAndUpdate( RimBoxIntersection* intersectionBox );
    void appendIntersectionBoxNoUpdate( RimBoxIntersection* intersectionBox );

    bool hasActiveIntersectionForSimulationWell( const RimSimWellInView* simWell ) const;
    bool hasAnyActiveSeparateResults();

    void updateIntersectionBoxGeometry();

    void synchronize2dIntersectionViews();
    void scheduleCreateDisplayModelAndRedraw2dIntersectionViews();

    bool shouldApplyCellFiltersToIntersections() const;

    // Visualization interface

    void applySingleColorEffect();
    void updateCellResultColor( bool hasGeneralCellResult, int timeStepIndex );
    void appendPartsToModel( Rim3dView& view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendDynamicPartsToModel( cvf::ModelBasicList* model,
                                    cvf::Transform*      scaleTransform,
                                    size_t               timeStepIndex,
                                    cvf::UByteArray*     visibleCells = nullptr );
    void clearGeometry();

    std::vector<RimExtrudedCurveIntersection*> intersections() const;
    std::vector<RimBoxIntersection*>           intersectionBoxes() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    void onChildAdded( caf::PdmFieldHandle* containerForNewObject ) override;

    bool isActive() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;

private:
    RimEclipseView* eclipseView() const;
    void            rebuild3dView() const;

    caf::PdmField<bool> m_isActive;

    caf::PdmChildArrayField<RimExtrudedCurveIntersection*> m_intersections;
    caf::PdmChildArrayField<RimBoxIntersection*>           m_intersectionBoxes;

    caf::PdmField<bool>                                    m_depthThresholdOverridden;
    caf::PdmField<double>                                  m_depthUpperThreshold;
    caf::PdmField<double>                                  m_depthLowerThreshold;
    caf::PdmField<caf::AppEnum<RimIntersectionFilterEnum>> m_depthFilterType;

    caf::PdmField<bool> m_applyCellFilters;

    caf::PdmField<bool>    m_kFilterOverridden;
    caf::PdmField<QString> m_kFilterStr;
};
