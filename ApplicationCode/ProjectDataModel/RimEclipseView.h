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

#include "RimView.h"

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
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
class RimStimPlanColors;
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
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

enum PartRenderMaskEnum
{
    surfaceBit      = 0x00000001,
    meshSurfaceBit  = 0x00000002,
    faultBit        = 0x00000004,
    meshFaultBit    = 0x00000008,
};

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseView : public RimView
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseView(void);
    virtual ~RimEclipseView(void);

    // Fields containing child objects :

    caf::PdmChildField<RimEclipseCellColors*>               cellResult;
    caf::PdmChildField<RimCellEdgeColors*>                  cellEdgeResult;
    caf::PdmChildField<RimEclipseFaultColors*>              faultResultSettings;
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    caf::PdmChildField<RimStimPlanColors*>                  stimPlanColors;
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    caf::PdmChildField<RimSimWellInViewCollection*>         wellCollection;
    caf::PdmChildField<RimFaultInViewCollection*>           faultCollection;

    // Fields

    caf::PdmField<bool>                             showInvalidCells;
    caf::PdmField<bool>                             showInactiveCells;
    caf::PdmField<bool>                             showMainGrid;

    // Access internal objects
    virtual const RimPropertyFilterCollection*      propertyFilterCollection() const override;

    RimEclipsePropertyFilterCollection*             eclipsePropertyFilterCollection();
    const RimEclipsePropertyFilterCollection*       eclipsePropertyFilterCollection() const;
    void                                            setOverridePropertyFilterCollection(RimEclipsePropertyFilterCollection* pfc);

    RigCaseCellResultsData*                         currentGridCellResults();
    RigActiveCellInfo*                              currentActiveCellInfo();
    RimEclipseCellColors*                           currentFaultResultColors();

    void                                            setEclipseCase(RimEclipseCase* reservoir);
    RimEclipseCase*                                 eclipseCase() const;
    virtual RimCase*                                ownerCase() const override;

    RigMainGrid*                                    mainGrid() const;

    // Display model generation

    bool                                            isTimeStepDependentDataVisible() const;

    virtual void                                    scheduleGeometryRegen(RivCellSetEnum geometryType) override;
    void                                            scheduleReservoirGridGeometryRegen();
    void                                            scheduleSimWellGeometryRegen();
    void                                            updateDisplayModelForWellResults();

    const std::vector<RivCellSetEnum>&              visibleGridParts() const;
    const RivReservoirViewPartMgr*                  reservoirGridPartManager() const;
    RivReservoirViewPartMgr*                        reservoirGridPartManager();

    // Does this belong here, really ?
    void                                            calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);

    // Overridden PDM methods:
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                            updateIconStateForFilterCollections();

    virtual void                                    axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) override;

    virtual bool                                    isUsingFormationNames() const override;

    virtual void                                    calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep) override;

protected:
    virtual void                                    initAfterRead() override;
    virtual void                                    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                                    onLoadDataAndUpdate() override;

    virtual void                                    createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) override;
    virtual bool                                    showActiveCellsOnly() override;

private:
    void                                            createDisplayModel() override;
    void                                            updateDisplayModelVisibility() override;
    virtual void                                    updateCurrentTimeStep() override;

    void                                            indicesToVisibleGrids(std::vector<size_t>* gridIndices);
    virtual void                                    updateScaleTransform() override;
    virtual cvf::Transform*                         scaleTransform() override;

    virtual void                                    updateStaticCellColors() override;
    void                                            updateStaticCellColors(RivCellSetEnum geometryType);
    void                                            updateLegends();
    void                                            updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimEclipseCellColors* resultColors, RigCaseCellResultsData* cellResultsData);
    virtual void                                    resetLegendsInViewer() override;

    std::set<RivCellSetEnum>                        allVisibleFaultGeometryTypes() const;
    void                                            updateFaultColors();

    void                                            syncronizeWellsWithResults();

    void                                            clampCurrentTimestep() override;
    void                                            setVisibleGridParts(const std::vector<RivCellSetEnum>& cellSets);

private:
    caf::PdmChildField<RimEclipsePropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimEclipsePropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPointer<RimEclipseCase>                 m_eclipseCase;

    cvf::ref<RivReservoirViewPartMgr>               m_reservoirGridPartManager;
    cvf::ref<RivReservoirSimWellsPartMgr>           m_simWellsPartManager;
	
    std::vector<RivCellSetEnum>                     m_visibleGridParts;
};

