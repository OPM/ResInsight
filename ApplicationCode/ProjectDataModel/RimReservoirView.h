/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfObject.h"

// Includes to make Pdm work for cvf::Color and cvf:Mat
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"

#include "RivReservoirViewPartMgr.h"

class RigActiveCellInfo;
class RigCaseCellResultsData;
class RigGridBase;
class RigGridCellFaceVisibilityFilter;
class Rim3dOverlayInfoConfig;
class RimCase;
class RimCellEdgeResultSlot;
class RimCellPropertyFilter;
class RimCellPropertyFilterCollection;
class RimCellRangeFilter;
class RimCellRangeFilterCollection;
class RimFaultCollection;
class RimFaultResultSlot;
class RimReservoirCellResultsStorage;
class RimReservoirCellResultsStorage;
class RimResultSlot;
class RimWellCollection;
class RiuViewer;
class RivReservoirPipesPartMgr;

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
class RimReservoirView : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimReservoirView(void);
    virtual ~RimReservoirView(void);

    enum MeshModeType
    {
        FULL_MESH,
        FAULTS_MESH,
        NO_MESH
    };

    enum SurfaceModeType
    {
        SURFACE,
        FAULTS,
        NO_SURFACE
    };

    // Fields containing child objects :

    caf::PdmField<RimResultSlot*>                       cellResult;
    caf::PdmField<RimCellEdgeResultSlot*>               cellEdgeResult;
    caf::PdmField<RimFaultResultSlot*>                  faultResultSettings;

    caf::PdmField<RimCellRangeFilterCollection*>        rangeFilterCollection;
    caf::PdmField<RimCellPropertyFilterCollection*>     propertyFilterCollection;

    caf::PdmField<RimWellCollection*>                   wellCollection;

    caf::PdmField<RimFaultCollection*>                  faultCollection;

    caf::PdmField<Rim3dOverlayInfoConfig*>              overlayInfoConfig;

    // Visualization setup fields

    caf::PdmField<QString>                              name;
    caf::PdmField<double>                               scaleZ;
    caf::PdmField<bool>                                 showWindow;

    caf::PdmField<bool>                                 showInvalidCells;
    caf::PdmField<bool>                                 showInactiveCells;
    caf::PdmField<bool>                                 showMainGrid;

    caf::PdmField< caf::AppEnum< MeshModeType > >       meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > >    surfaceMode;

    caf::PdmField< cvf::Color3f >                       backgroundColor;

    caf::PdmField<cvf::Mat4d>                           cameraPosition;

    caf::PdmField<int>                                  maximumFrameRate;
    caf::PdmField<bool>                                 animationMode;


    // Access internal objects
    RimReservoirCellResultsStorage*         currentGridCellResults();
    RigActiveCellInfo*                      currentActiveCellInfo();

    void                                    setEclipseCase(RimCase* reservoir);
    RimCase*                                eclipseCase();

    // Animation
    int                                     currentTimeStep()    { return m_currentTimeStep;}
    void                                    setCurrentTimeStep(int frameIdx);
    void                                    updateCurrentTimeStepAndRedraw();
    void                                    endAnimation();

    // 3D Viewer
    RiuViewer*                              viewer();
    void                                    updateViewerWidget();
    void                                    updateViewerWidgetWindowTitle();
    void                                    setDefaultView();

    void                                    setMeshOnlyDrawstyle();
    void                                    setMeshSurfDrawstyle();
    void                                    setSurfOnlyDrawstyle();
    void                                    setFaultMeshSurfDrawstyle();

    void                                    setShowFaultsOnly(bool showFaults);
    bool                                    isGridVisualizationMode() const;


    // Picking info
    bool                                    pickInfo(size_t gridIndex, size_t cellIndex, cvf::StructGridInterface::FaceType face, const cvf::Vec3d& point, QString itemSeparator, QString* pickInfoText) const;
    void                                    appendCellResultInfo(size_t gridIndex, size_t cellIndex, cvf::StructGridInterface::FaceType face, QString* resultInfoText) ;
    void                                    appendNNCResultInfo(size_t nncIndex, QString* resultInfo);
    static void                             appendTextFromResultSlot(RigCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimResultSlot* resultSlot, QString* resultInfoText);

    // Does this belong here, really ?
    void                                    calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);

    // Display model generation
public:
    void                                    loadDataAndUpdate();
    void                                    createDisplayModelAndRedraw();
    void                                    scheduleCreateDisplayModelAndRedraw();
    bool                                    isTimeStepDependentDataVisible() const;

    void                                    scheduleGeometryRegen(unsigned short geometryType);
    void                                    scheduleReservoirGridGeometryRegen();
    void                                    schedulePipeGeometryRegen();
    void                                    updateDisplayModelForWellResults();

    const std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType>&
                                            visibleGridParts() const { return m_visibleGridParts;}
    cvf::cref<RivReservoirViewPartMgr>      reservoirGridPartManager() const { return m_reservoirGridPartManager.p(); }

    // Display model generation
private:
    void                                    appendTextFromFault(RigGridBase* grid, size_t cellIndex, cvf::StructGridInterface::FaceType face, QString* textString);

    void                                    createDisplayModel();
    void                                    updateDisplayModelVisibility();
    void                                    updateCurrentTimeStep();
    void                                    indicesToVisibleGrids(std::vector<size_t>* gridIndices);
    void                                    updateScaleTransform();
    void                                    updateStaticCellColors();
    void                                    updateStaticCellColors(unsigned short geometryType);
    void                                    updateLegends();
    void                                    updateMinMaxValuesAndAddLegendToView(QString legendLabel, RimResultSlot* resultSlot, RigCaseCellResultsData* cellResultsData);

    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> visibleFaultGeometryTypes() const;
    void                                    updateFaultForcedVisibility();
    void                                    updateFaultColors();

    cvf::ref<RivReservoirViewPartMgr>       m_reservoirGridPartManager;
    cvf::ref<RivReservoirPipesPartMgr>      m_pipesPartManager;

    // Overridden PDM methods:
public:
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &name; }
    virtual caf::PdmFieldHandle*            objectToggleField();
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
protected:
    virtual void                            initAfterRead();
    virtual void                            setupBeforeSave();
    virtual void                            defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    // Really private
private:
    void                                    syncronizeWellsWithResults();
    void                                    clampCurrentTimestep();

private:
    caf::PdmField<int>                      m_currentTimeStep;
    QPointer<RiuViewer>                     m_viewer;
    caf::PdmPointer<RimCase>                m_reservoir;

    bool                                    m_previousGridModeMeshLinesWasFaults;

    std::vector<RivReservoirViewPartMgr::ReservoirGeometryCacheType> m_visibleGridParts;
};

