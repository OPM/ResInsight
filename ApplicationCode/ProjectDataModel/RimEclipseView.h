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

#include "cvfArray.h"
#include "cvfBase.h"
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
class RivIntersectionPartMgr;
class RivReservoirViewPartMgr;

namespace cvf
{
    class Transform;
    class ScalarMapperUniformLevels;
    class ModelBasicList;
    class OverlayItem;
}

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

    RimEclipseCellColors*                           cellResult() const;
    RimCellEdgeColors*                              cellEdgeResult() const;
    RimEclipseFaultColors*                          faultResultSettings() const;
    RimStimPlanColors*                              fractureColors() const;
    RimSimWellInViewCollection*                     wellCollection() const;
    RimFaultInViewCollection*                       faultCollection() const;
    RimVirtualPerforationResults*                   virtualPerforationResult() const;

    bool                                            showInvalidCells() const;
    bool                                            showInactiveCells() const;

    // Access internal objects
    const RimPropertyFilterCollection*              propertyFilterCollection() const override;

    RimEclipsePropertyFilterCollection*             eclipsePropertyFilterCollection();
    const RimEclipsePropertyFilterCollection*       eclipsePropertyFilterCollection() const;
    void                                            setOverridePropertyFilterCollection(RimEclipsePropertyFilterCollection* pfc);

    RigCaseCellResultsData*                         currentGridCellResults();
    const RigActiveCellInfo*                        currentActiveCellInfo() const;
    RimEclipseCellColors*                           currentFaultResultColors();

    void                                            setEclipseCase(RimEclipseCase* reservoir);
    RimEclipseCase*                                 eclipseCase() const;
    RimCase*                                        ownerCase() const override;

    RigMainGrid*                                    mainGrid() const;

    // Display model generation

    bool                                            isTimeStepDependentDataVisible() const override;

    void                                            scheduleGeometryRegen(RivCellSetEnum geometryType) override;
    void                                            scheduleReservoirGridGeometryRegen();
    void                                            scheduleSimWellGeometryRegen();
    void                                            updateDisplayModelForWellResults();
    
    void                                            calculateCompletionTypeAndRedrawIfRequired();

    bool                                            isVirtualConnectionFactorGeometryVisible() const;
    bool                                            isMainGridVisible() const;


    const std::vector<RivCellSetEnum>&              visibleGridParts() const;
    const RivReservoirViewPartMgr*                  reservoirGridPartManager() const;
    RivReservoirViewPartMgr*                        reservoirGridPartManager();

    // Does this belong here, really ?
    void                                            calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);

    // Overridden PDM methods:
    void                                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                            updateIconStateForFilterCollections();

    void                                            axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) override;

    bool                                            isUsingFormationNames() const override;

    virtual void                                    calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep) override;
    
    virtual std::vector<RimLegendConfig*>           legendConfigs() const override;
    cvf::Color4f                                    colorFromCellCategory(RivCellSetEnum geometryType) const;

protected:
    void                                    initAfterRead() override;
    virtual void                            defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                            onLoadDataAndUpdate() override;

    void                                    createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) override;
    bool                                    showActiveCellsOnly() override;
    virtual void                            updateCurrentTimeStep() override;
    void                                    updateVisibleGeometriesAndCellColors();
    void                                    appendWellsAndFracturesToModel();

    virtual void                            createDisplayModel() override;
    RimPropertyFilterCollection*            nativePropertyFilterCollection();
    virtual std::set<RivCellSetEnum>        allVisibleFaultGeometryTypes() const;

private:
    void                                            updateDisplayModelVisibility() override;

    std::vector<size_t>                             indicesToVisibleGrids() const;
    void                                            updateScaleTransform() override;
    cvf::Transform*                                 scaleTransform() override;

    void                                            updateStaticCellColors() override;
    void                                            updateStaticCellColors(RivCellSetEnum geometryType);

    virtual void                                    updateLegends() override;
    void                                            updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimEclipseCellColors* resultColors, RigCaseCellResultsData* cellResultsData);
    void                                            resetLegendsInViewer() override;
    void                                            updateVirtualConnectionLegendRanges();

    void                                            updateFaultColors();

    void                                            syncronizeWellsWithResults();

    void                                            clampCurrentTimestep() override;
    void                                            setVisibleGridParts(const std::vector<RivCellSetEnum>& cellSets);
    void                                            setVisibleGridPartsWatertight();

private:
    caf::PdmField<bool>                             m_showInvalidCells;
    caf::PdmField<bool>                             m_showInactiveCells;
    caf::PdmField<bool>                             m_showMainGrid_OBSOLETE;

    caf::PdmChildField<RimEclipseCellColors*>       m_cellResult;
    caf::PdmChildField<RimCellEdgeColors*>          m_cellEdgeResult;
    caf::PdmChildField<RimEclipseFaultColors*>      m_faultResultSettings;
    caf::PdmChildField<RimStimPlanColors*>          m_fractureColors;
    caf::PdmChildField<RimVirtualPerforationResults*> m_virtualPerforationResult;

    caf::PdmChildField<RimSimWellInViewCollection*> m_wellCollection;
    caf::PdmChildField<RimFaultInViewCollection*>   m_faultCollection;

    caf::PdmChildField<RimEclipsePropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimEclipsePropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPointer<RimEclipseCase>                 m_eclipseCase;

    cvf::ref<RivReservoirViewPartMgr>               m_reservoirGridPartManager;
    cvf::ref<RivReservoirSimWellsPartMgr>           m_simWellsPartManager;
    
    std::vector<RivCellSetEnum>                     m_visibleGridParts;
};
