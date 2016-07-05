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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmObject.h"

#include "RivCellSetEnum.h"

#include "RimViewWindow.h"

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfObject.h"

#include <QPointer>


class Rim3dOverlayInfoConfig;
class RimCase;
class RimCellRangeFilter;
class RimGridCollection;
class RimCellRangeFilterCollection;
class RimCrossSectionCollection;
class RimPropertyFilterCollection;
class RimViewController;
class RimViewLinker;
class RiuViewer;

namespace cvf
{
    class BoundingBox;
    class ModelBasicList;
    class Scene;
    class String;
    class Transform;
    class Part;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimView : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;
public:
    RimView(void);
    virtual ~RimView(void);

    // 3D Viewer
    RiuViewer*                              viewer();

    caf::PdmField<QString>                  name;
    caf::PdmField<double>                   scaleZ;

    caf::PdmField<bool>                     showWindow;
    caf::PdmField<cvf::Mat4d>               cameraPosition;
    caf::PdmField<bool>                     isPerspectiveView;
    caf::PdmField< cvf::Color3f >           backgroundColor;

    caf::PdmField<int>                      maximumFrameRate;
    caf::PdmField<bool>                     hasUserRequestedAnimation;

    virtual const RimPropertyFilterCollection* propertyFilterCollection() const = 0;
    RimCellRangeFilterCollection*           rangeFilterCollection();
    const RimCellRangeFilterCollection*     rangeFilterCollection() const;

    bool                                    hasOverridenRangeFilterCollection();
    void                                    setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc);
    void                                    replaceRangeFilterCollectionWithOverride();


    caf::PdmChildField<RimCrossSectionCollection*>      crossSectionCollection;

    // Draw style 

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

    caf::PdmField< caf::AppEnum< MeshModeType > >       meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > >    surfaceMode;
    caf::PdmField<bool>                     showGridBox;

    void                                    setMeshOnlyDrawstyle();
    void                                    setMeshSurfDrawstyle();
    void                                    setSurfOnlyDrawstyle();
    void                                    setFaultMeshSurfDrawstyle();
    void                                    setSurfaceDrawstyle();

    void                                    disableLighting(bool disable);
    bool                                    isLightingDisabled() const;
   
    void                                    showGridCells(bool enableHideGridCells);
    bool                                    isGridVisualizationMode() const;

    void                                    setScaleZAndUpdate(double scaleZ);

    // Animation
    int                                     currentTimeStep()    { return m_currentTimeStep;}
    void                                    setCurrentTimeStep(int frameIdx);
    void                                    updateCurrentTimeStepAndRedraw();
    void                                    endAnimation();

    virtual void                            scheduleGeometryRegen(RivCellSetEnum geometryType) = 0;
    void                                    scheduleCreateDisplayModelAndRedraw();
    void                                    createDisplayModelAndRedraw();
    void                                    createHighlightAndGridBoxDisplayModelWithRedraw();

    RimViewController*                      viewController() const;
    bool                                    isMasterView() const;
    RimViewLinker*                          assosiatedViewLinker() const;

    cvf::ref<cvf::UByteArray>               currentTotalCellVisibility();

    virtual bool                            showActiveCellsOnly();
    virtual void                            axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) = 0;

    void                                    selectOverlayInfoConfig();


    virtual void                            zoomAll() override;

public:
    virtual void                            loadDataAndUpdate() = 0;
    virtual RimCase*                        ownerCase() = 0;

    virtual caf::PdmFieldHandle*            objectToggleField()     { return &showWindow; }
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &name; }
protected:

    void                                    setDefaultView();

    void                                    addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                                                const cvf::Vec3d& displayModelOffset,  
                                                                double characteristicCellSize, 
                                                                const cvf::BoundingBox& wellPathClipBoundingBox, 
                                                                cvf::Transform* scaleTransform);

    static void                             removeModelByName(cvf::Scene* scene, const cvf::String& modelName);

    virtual void                            createDisplayModel() = 0;
    
    void                                    createHighlightAndGridBoxDisplayModel();
    void                                    updateGridBoxData();

    virtual void                            createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) = 0;
    
    virtual void                            updateDisplayModelVisibility() = 0;
    virtual void                            clampCurrentTimestep() = 0;

    virtual void                            updateCurrentTimeStep() = 0;
    virtual void                            updateStaticCellColors() = 0;
 
    virtual void                            updateScaleTransform() = 0;
    virtual cvf::Transform*                 scaleTransform() = 0;

    void                                    updateViewerWidget();
    virtual void                            updateViewerWidgetWindowTitle() = 0;

    virtual void                            resetLegendsInViewer() = 0;
    virtual void                            calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility) = 0;

    virtual QImage                          snapshotWindowContent() override;


    QPointer<RiuViewer>                     m_viewer;

    caf::PdmField<int>                                  m_currentTimeStep;
    caf::PdmChildField<Rim3dOverlayInfoConfig*>         m_overlayInfoConfig;

    caf::PdmChildField<RimCellRangeFilterCollection*>   m_rangeFilterCollection;
    caf::PdmChildField<RimCellRangeFilterCollection*>   m_overrideRangeFilterCollection;
    
    caf::PdmChildField<RimGridCollection*>              m_gridCollection;
    
    // Overridden PDM methods:
    virtual void                            setupBeforeSave();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    cvf::ref<cvf::UByteArray>               m_currentReservoirCellVisibility;
    
    cvf::ref<cvf::ModelBasicList>           m_wellPathPipeVizModel;
    cvf::ref<cvf::ModelBasicList>           m_crossSectionVizModel;
    cvf::ref<cvf::ModelBasicList>           m_highlightVizModel;

private:
    RimViewLinker*                          viewLinkerIfMasterView() const;
private:
    bool                                    m_previousGridModeMeshLinesWasFaults;
    caf::PdmField<bool>                     m_disableLighting;
};



