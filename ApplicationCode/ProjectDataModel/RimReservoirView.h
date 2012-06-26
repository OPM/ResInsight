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
#include "cafPdmObject.h"
#include "cafAppEnum.h"

#include <QPointer>
#include <QString>

#include "RimReservoir.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"

#include "cvfMatrix4.h"
#include "cvfStructGridGeometryGenerator.h"

#include "RivReservoirViewPartMgr.h"
#include "RivReservoirPipesPartMgr.h"

class RIViewer;
class RigGridBase;
class RigGridCellFaceVisibilityFilter;
class RivReservoirViewPartMgr;

namespace cvf
{
    class Transform;
    class ScalarMapperUniformLevels;
    class ModelBasicList;
}


enum ViewState
{
    GEOMETRY_ONLY,
    STATIC_RESULT,
    DYNAMIC_RESULT,
    CELL_FACE_COMBINED_RESULT
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
        NO_MESH
    };

    enum SurfaceModeType
    {
        SURFACE,
        FAULTS,
        NO_SURFACE
    };

    // Public fields:

    caf::PdmField<RimResultSlot*>           cellResult;
    caf::PdmField<RimCellEdgeResultSlot*>   cellEdgeResult;

    caf::PdmField<double>           scaleZ;
    caf::PdmField<bool>             showWindow;
    caf::PdmField<QString>          name;

    // Visibility
    caf::PdmField<bool>             showInvalidCells;
    caf::PdmField<bool>             showInactiveCells;
    caf::PdmField<bool>             showMainGrid;

    caf::PdmField<RimWellCollection*> wellCollection;

    caf::PdmField<RimCellRangeFilterCollection*>    rangeFilterCollection;
    caf::PdmField<RimCellPropertyFilterCollection*> propertyFilterCollection;

    caf::PdmField< caf::AppEnum< MeshModeType > >    meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > > surfaceMode;

    // Animation
    caf::PdmField<int>              maximumFrameRate;
    caf::PdmField<bool>             animationMode;

    int                             currentTimeStep()    { return m_currentTimeStep;}
    void                            setCurrentTimeStep(int frameIdx);
    void                            updateCurrentTimeStepAndRedraw();
    void                            endAnimation();

    // 3D Viewer
    // Cam pos should be a field, but is not yet supported bu caf::Pdm
    cvf::Mat4d                      cameraPosition;
    void                            setDefaultView();

    RIViewer*                       viewer();
    void                            updateViewerWidget();
    void                            updateViewerWidgetWindowTitle();

    // Picking info
    bool                            pickInfo(size_t gridIndex, size_t cellIndex, const cvf::Vec3d& point, QString* pickInfoText) const;
    void                            appendCellResultInfo(size_t gridIndex, size_t cellIndex, QString* resultInfoText) const;

    RigReservoirCellResults*        gridCellResults();
    
    void                            setEclipseCase(RimReservoir* reservoir);
    RimReservoir*                   eclipseCase();

    void                            calculateVisibleWellCellsIncFence(cvf::UByteArray* visibleCells, RigGridBase * grid);


    // Display model generation
public:
    void                            loadDataAndUpdate();
    void                            createDisplayModelAndRedraw();
    void                            scheduleGeometryRegen(unsigned short geometryType);
    void                            schedulePipeGeometryRegen();

    // Overridden PDM methods:
public:
    virtual caf::PdmFieldHandle*    userDescriptionField()  { return &name;}
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
protected:
    virtual void                    initAfterRead();
    virtual void                    setupBeforeSave();


private:
    void                              syncronizeWellsWithResults();

private:
    caf::PdmField<int>              m_currentTimeStep;
    QPointer<RIViewer>              m_viewer;
    caf::PdmPointer<RimReservoir>   m_reservoir;


// Display model generation
private:
    void                            createDisplayModel();
    void                            updateDisplayModelVisibility();
    void                            updateCurrentTimeStep();
    void                            indicesToVisibleGrids(std::vector<size_t>* gridIndices);
    void                            updateScaleTransform();
    void                            updateStaticCellColors();
    void                            updateStaticCellColors(unsigned short geometryType);
    void                            updateLegends();


    cvf::ref<RivReservoirViewPartMgr> m_geometry;
    cvf::ref<RivReservoirPipesPartMgr> m_pipesPartManager;
};

