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
class RimEclipseWell;
class RimEclipseWellCollection;
class RimFaultCollection;
class RimReservoirCellResultsStorage;
class RimReservoirCellResultsStorage;
class RimStimPlanColors;
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
    caf::PdmChildField<RimStimPlanColors*>                  stimPlanColors;

    caf::PdmChildField<RimEclipseWellCollection*>            wellCollection;
    caf::PdmChildField<RimFaultCollection*>                  faultCollection;

    // Fields

    caf::PdmField<bool>                             showInvalidCells;
    caf::PdmField<bool>                             showInactiveCells;
    caf::PdmField<bool>                             showMainGrid;

    // Access internal objects
    virtual const RimPropertyFilterCollection*      propertyFilterCollection() const;

    RimEclipsePropertyFilterCollection*             eclipsePropertyFilterCollection();
    const RimEclipsePropertyFilterCollection*       eclipsePropertyFilterCollection() const;
    void                                            setOverridePropertyFilterCollection(RimEclipsePropertyFilterCollection* pfc);

    RimReservoirCellResultsStorage*                 currentGridCellResults();
    RigActiveCellInfo*                              currentActiveCellInfo();
    RimEclipseCellColors*                           currentFaultResultColors();

    void                                            setEclipseCase(RimEclipseCase* reservoir);
    RimEclipseCase*                                 eclipseCase() const;
    virtual RimCase*                                ownerCase() const override;

    RigMainGrid*                                    mainGrid() const;

    // Display model generation

    virtual void                                    loadDataAndUpdate();
    bool                                            isTimeStepDependentDataVisible() const;

    virtual void                                    scheduleGeometryRegen(RivCellSetEnum geometryType);
    void                                            scheduleReservoirGridGeometryRegen();
    void                                            scheduleSimWellGeometryRegen();
    void                                            updateDisplayModelForWellResults();

    const std::vector<RivCellSetEnum>&              visibleGridParts() const;
    const RivReservoirViewPartMgr*                  reservoirGridPartManager() const;

    // Does this belong here, really ?
    void                                            calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);

    // Overridden PDM methods:
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    void                                            updateIconStateForFilterCollections();

    virtual void                                    axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel);

    virtual bool                                    isUsingFormationNames() const override;

protected:
    virtual void                                    initAfterRead();
    virtual void                                    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    virtual void                                    defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");

    virtual void                                    createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts);
    virtual bool                                    showActiveCellsOnly();

private:
    void                                            createDisplayModel();
    void                                            updateDisplayModelVisibility();
    virtual void                                    updateCurrentTimeStep();

    void                                            indicesToVisibleGrids(std::vector<size_t>* gridIndices);
    virtual void                                    updateScaleTransform();
    virtual cvf::Transform*                         scaleTransform();

    virtual void                                    updateStaticCellColors();
    void                                            updateStaticCellColors(RivCellSetEnum geometryType);
    void                                            updateLegends();
    void                                            updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimEclipseCellColors* resultColors, RigCaseCellResultsData* cellResultsData);
    virtual void                                    resetLegendsInViewer();

    std::set<RivCellSetEnum>                        allVisibleFaultGeometryTypes() const;
    void                                            updateFaultColors();

    void                                            syncronizeWellsWithResults();

    void                                            clampCurrentTimestep();

    virtual void calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility);

    caf::PdmChildField<RimEclipsePropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimEclipsePropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPointer<RimEclipseCase>                 m_eclipseCase;

    cvf::ref<RivReservoirViewPartMgr>               m_reservoirGridPartManager;
    cvf::ref<RivReservoirSimWellsPartMgr>           m_simWellsPartManager;
	
    std::vector<RivCellSetEnum>                     m_visibleGridParts;
};

