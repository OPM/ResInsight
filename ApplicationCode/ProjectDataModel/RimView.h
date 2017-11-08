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
#include "cafPdmFieldCvfVec3d.h"
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
class RimCellRangeFilterCollection;
class RimIntersectionCollection;
class RimGridCollection;
class RimPropertyFilterCollection;
class RimViewController;
class RimViewLinker;
class RiuViewer;
class RimWellPathCollection;

namespace cvf
{
    class BoundingBox;
    class ModelBasicList;
    class Scene;
    class String;
    class Transform;
    class Part;
}

namespace caf
{
    class DisplayCoordTransform;
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

    caf::PdmField<cvf::Mat4d>               cameraPosition;
    caf::PdmField<cvf::Vec3d>               cameraPointOfInterest;
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


    caf::PdmChildField<RimIntersectionCollection*>      crossSectionCollection;

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
   
    void                                    showGridCells(bool enableGridCells);
    bool                                    isGridVisualizationMode() const;

    void                                    setScaleZAndUpdate(double scaleZ);

    // Animation
    int                                     currentTimeStep() const { return m_currentTimeStep;}
    void                                    setCurrentTimeStep(int frameIdx);
    void                                    setCurrentTimeStepAndUpdate(int frameIdx);

    void                                    updateCurrentTimeStepAndRedraw();

    virtual void                            scheduleGeometryRegen(RivCellSetEnum geometryType) = 0;
    void                                    scheduleCreateDisplayModelAndRedraw();
    void                                    createDisplayModelAndRedraw();
    void                                    createHighlightAndGridBoxDisplayModelWithRedraw();

    RimViewController*                      viewController() const;
    bool                                    isMasterView() const;
    RimViewLinker*                          assosiatedViewLinker() const;

    virtual bool                            isUsingFormationNames() const = 0;
    cvf::ref<cvf::UByteArray>               currentTotalCellVisibility();

    virtual bool                            showActiveCellsOnly();
    virtual void                            axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) = 0;

    void                                    selectOverlayInfoConfig();

    virtual QImage                          snapshotWindowContent() override;

    virtual void                            zoomAll() override;

    cvf::ref<caf::DisplayCoordTransform>    displayCoordTransform() const;

    virtual QWidget*                        viewWidget() override;
    void                                    forceShowWindowOn();

public:
    void                                    updateGridBoxData();
    void                                    updateAnnotationItems();
    virtual RimCase*                        ownerCase() const = 0;

    virtual caf::PdmFieldHandle*            userDescriptionField() override { return &name; }
protected:

    void                                    setDefaultView();

    void                                    addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                                                const cvf::BoundingBox& wellPathClipBoundingBox);

    void                                    addDynamicWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                                                       const cvf::BoundingBox& wellPathClipBoundingBox);

    static void                             removeModelByName(cvf::Scene* scene, const cvf::String& modelName);

    virtual void                            createDisplayModel() = 0;
    
    void                                    createHighlightAndGridBoxDisplayModel();

    virtual void                            createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) = 0;
    
    virtual void                            updateDisplayModelVisibility() = 0;
    virtual void                            clampCurrentTimestep() = 0;

    virtual void                            updateCurrentTimeStep() = 0;
    virtual void                            updateStaticCellColors() = 0;
 
    virtual void                            updateScaleTransform() = 0;
    virtual cvf::Transform*                 scaleTransform() = 0;

    virtual void                            resetLegendsInViewer() = 0;
    virtual void                            calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep) = 0;

    virtual void                            onLoadDataAndUpdate() = 0;

    RimWellPathCollection*                  wellPathCollection();

    QPointer<RiuViewer>                     m_viewer;

    caf::PdmField<int>                                  m_currentTimeStep;
    caf::PdmChildField<Rim3dOverlayInfoConfig*>         m_overlayInfoConfig;

    caf::PdmChildField<RimCellRangeFilterCollection*>   m_rangeFilterCollection;
    caf::PdmChildField<RimCellRangeFilterCollection*>   m_overrideRangeFilterCollection;
    
    caf::PdmChildField<RimGridCollection*>              m_gridCollection;
    
    // Overridden PDM methods:
    virtual void                            setupBeforeSave() override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    virtual QWidget*                        createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                            updateViewWidgetAfterCreation() override; 
    virtual void                            updateMdiWindowTitle() override;
    virtual void                            deleteViewWidget() override;

    cvf::ref<cvf::UByteArray>               m_currentReservoirCellVisibility;
    
    cvf::ref<cvf::ModelBasicList>           m_wellPathPipeVizModel;
    cvf::ref<cvf::ModelBasicList>           m_crossSectionVizModel;
    cvf::ref<cvf::ModelBasicList>           m_highlightVizModel;

private:
    RimViewLinker*                          viewLinkerIfMasterView() const;

    friend class RiuViewer;
    void                                    endAnimation();

private:
    bool                                    m_previousGridModeMeshLinesWasFaults;
    caf::PdmField<bool>                     m_disableLighting;
};



