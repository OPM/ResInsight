/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include "cvfArray.h"

#include "cvfObject.h"

// Includes to make Pdm work for cvf::Color and cvf:Mat
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"

#include "RimGridView.h"

class RigActiveCellInfo;
class RigCaseCellResultsData;
class RigGridBase;
class RigGridCellFaceVisibilityFilter;
class RigMainGrid;
class Rim3dOverlayInfoConfig;
class RimCellEdgeColors;
class RimCellRangeFilter;
class RimCellRangeFilterCollection;
class RimEclipseCase;
class RimEclipseCellColors;
class RimEclipseFaultColors;
class RimEclipsePropertyFilter;
class RimEclipsePropertyFilterCollection;
class RimFaultInViewCollection;
class RimReservoirCellResultsStorage;
class RimReservoirCellResultsStorage;
class RimSimWellInViewCollection;
class RimStimPlanColors;
class RimVirtualPerforationResults;
class RiuViewer;
class RivReservoirSimWellsPartMgr;
class RivExtrudedCurveIntersectionPartMgr;
class RivReservoirViewPartMgr;
class RivStreamlinesPartMgr;
class RimRegularLegendConfig;
class RimTernaryLegendConfig;
class RimEclipseResultDefinition;
class RimElementVectorResult;
class RimStreamlineInViewCollection;
class RimMultipleEclipseResults;
class RigEclipseResultAddress;
class RimFaultReactivationModelCollection;

namespace cvf
{
class Transform;
class ScalarMapperUniformLevels;
class ModelBasicList;
class OverlayItem;
} // namespace cvf

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseView : public RimGridView
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseView();
    ~RimEclipseView() override;

    RiaDefines::View3dContent viewContent() const override;

    RimEclipseCellColors*                cellResult() const;
    RimCellEdgeColors*                   cellEdgeResult() const;
    RimElementVectorResult*              elementVectorResult() const;
    RimEclipseFaultColors*               faultResultSettings() const;
    RimStimPlanColors*                   fractureColors() const;
    RimSimWellInViewCollection*          wellCollection() const;
    RimFaultInViewCollection*            faultCollection() const;
    RimVirtualPerforationResults*        virtualPerforationResult() const;
    RimStreamlineInViewCollection*       streamlineCollection() const;
    RimFaultReactivationModelCollection* faultReactivationModelCollection() const;

    bool showInvalidCells() const;
    bool showInactiveCells() const;

    // Access internal objects
    const RimPropertyFilterCollection* propertyFilterCollection() const override;

    RimEclipsePropertyFilterCollection*       eclipsePropertyFilterCollection();
    const RimEclipsePropertyFilterCollection* eclipsePropertyFilterCollection() const;
    void                                      setOverridePropertyFilterCollection( RimEclipsePropertyFilterCollection* pfc );

    RigCaseCellResultsData*  currentGridCellResults() const;
    const RigActiveCellInfo* currentActiveCellInfo() const;
    RimEclipseCellColors*    currentFaultResultColors();

    std::vector<double> currentCellResultData() const;
    void                setCurrentCellResultData( const std::vector<double>& values );

    void            setEclipseCase( RimEclipseCase* reservoir );
    RimEclipseCase* eclipseCase() const;
    RimCase*        ownerCase() const override;

    RigMainGrid* mainGrid() const;

    // Display model generation

    bool isTimeStepDependentDataVisible() const override;

    void scheduleGeometryRegen( RivCellSetEnum geometryType ) override;
    void scheduleReservoirGridGeometryRegen();
    void scheduleSimWellGeometryRegen();
    void updateDisplayModelForWellResults();

    void calculateCompletionTypeAndRedrawIfRequired();

    bool isVirtualConnectionFactorGeometryVisible() const;
    bool isMainGridVisible() const;

    const std::vector<RivCellSetEnum>& visibleGridParts() const;
    const RivReservoirViewPartMgr*     reservoirGridPartManager() const;
    RivReservoirViewPartMgr*           reservoirGridPartManager();

    // Does this belong here, really ?
    void calculateVisibleWellCellsIncFence( cvf::UByteArray* visibleCells, RigGridBase* grid );

    // Overridden PDM methods:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void updateIconStateForFilterCollections();

    void defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) override;

    bool isUsingFormationNames() const override;

    void calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int timeStep ) override;

    void calculateCellVisibility( cvf::UByteArray* visibility, std::vector<RivCellSetEnum> geomTypes, int timeStep = 0 ) override;

    std::vector<RimLegendConfig*> legendConfigs() const override;
    cvf::Color4f                  colorFromCellCategory( RivCellSetEnum geometryType ) const;

    std::vector<RigEclipseResultAddress> additionalResultsForResultInfo() const;

protected:
    void                 initAfterRead() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void                 onLoadDataAndUpdate() override;
    caf::PdmFieldHandle* userDescriptionField() override;

    bool isShowingActiveCellsOnly() override;
    void onUpdateDisplayModelForCurrentTimeStep() override;
    void updateVisibleCellColors();
    void updateVisibleGeometries();

    void appendWellsAndFracturesToModel();
    void appendElementVectorResultToModel();
    void appendStreamlinesToModel();

    void                             onCreateDisplayModel() override;
    RimPropertyFilterCollection*     nativePropertyFilterCollection();
    virtual std::set<RivCellSetEnum> allVisibleFaultGeometryTypes() const;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    QString createAutoName() const override;

    void onUpdateDisplayModelVisibility() override;

    std::vector<size_t> indicesToVisibleGrids() const;
    void                onUpdateScaleTransform() override;
    cvf::Transform*     scaleTransform() override;

    void onUpdateStaticCellColors() override;
    void updateStaticCellColors( RivCellSetEnum geometryType );

    void onUpdateLegends() override;
    void updateLegendRangesTextAndVisibility( RimRegularLegendConfig*     legendConfig,
                                              RimTernaryLegendConfig*     ternaryLegendConfig,
                                              QString                     legendLabel,
                                              RimEclipseResultDefinition* eclResDef,
                                              int                         timeStepIndex );

    void updateVirtualConnectionLegendRanges();

    void updateFaultColors();

    void synchronizeWellsWithResults();

    void   onClampCurrentTimestep() override;
    size_t onTimeStepCountRequested() override;

    void onAnimationsUpdate( const caf::SignalEmitter* emitter );

    void setVisibleGridParts( const std::vector<RivCellSetEnum>& cellSets );
    void setVisibleGridPartsWatertight();

    void propagateEclipseCaseToChildObjects();

protected:
    cvf::ref<cvf::ModelBasicList>     m_faultReactVizModel;
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;

private:
    caf::PdmField<bool> m_showInvalidCells;
    caf::PdmField<bool> m_showInactiveCells;

    caf::PdmChildField<RimEclipseCellColors*>         m_cellResult;
    caf::PdmChildField<RimCellEdgeColors*>            m_cellEdgeResult;
    caf::PdmChildField<RimElementVectorResult*>       m_elementVectorResult;
    caf::PdmChildField<RimEclipseFaultColors*>        m_faultResultSettings;
    caf::PdmChildField<RimStimPlanColors*>            m_fractureColors;
    caf::PdmChildField<RimVirtualPerforationResults*> m_virtualPerforationResult;

    caf::PdmProxyValueField<std::vector<double>> m_cellResultData;

    caf::PdmChildField<RimSimWellInViewCollection*>          m_wellCollection;
    caf::PdmChildField<RimFaultInViewCollection*>            m_faultCollection;
    caf::PdmChildField<RimFaultReactivationModelCollection*> m_faultReactivationModelCollection;
    caf::PdmChildField<RimStreamlineInViewCollection*>       m_streamlineCollection;

    caf::PdmChildField<RimEclipsePropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimEclipsePropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPtrField<RimEclipseCase*> m_customEclipseCase_OBSOLETE;

    cvf::ref<RivReservoirViewPartMgr>     m_reservoirGridPartManager;
    cvf::ref<RivReservoirSimWellsPartMgr> m_simWellsPartManager;
    cvf::ref<RivStreamlinesPartMgr>       m_streamlinesPartManager;

    std::vector<RivCellSetEnum> m_visibleGridParts;

    caf::PdmChildField<RimMultipleEclipseResults*> m_additionalResultsForResultInfo;
};
