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

#include "RiuViewerToViewInterface.h"
#include "RimViewWindow.h"

#include "RivCellSetEnum.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfObject.h"

#include <QPointer>

class RimCase;
class RimLegendConfig;
class RimWellPathCollection;
class RiuViewer;
class RivWellPathsPartMgr; 

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


enum PartRenderMaskEnum
{
    surfaceBit      = 1,
    meshSurfaceBit  = 2,
    faultBit        = 4,
    meshFaultBit    = 8,
    intersectionCellFaceBit    = 16,
    intersectionCellMeshBit    = 32,
    intersectionFaultMeshBit   = 64
};


//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dView : public RimViewWindow, public RiuViewerToViewInterface
{
    CAF_PDM_HEADER_INIT;
public:
    Rim3dView(void);
    virtual ~Rim3dView(void);

    // Public fields: 

    caf::PdmField<QString>                  name;
    caf::PdmField<double>                   scaleZ;
    caf::PdmField<bool>                     isPerspectiveView;
    caf::PdmField<int>                      maximumFrameRate;
    caf::PdmField<bool>                     hasUserRequestedAnimation;

    // Draw style 

    enum MeshModeType    { FULL_MESH, FAULTS_MESH, NO_MESH    };
    enum SurfaceModeType { SURFACE,   FAULTS,      NO_SURFACE };

    caf::PdmField< caf::AppEnum< MeshModeType > >    meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > > surfaceMode;

    RiuViewer*                              viewer();

    void                                    setMeshOnlyDrawstyle();
    void                                    setMeshSurfDrawstyle();
    void                                    setSurfOnlyDrawstyle();
    void                                    setFaultMeshSurfDrawstyle();
    void                                    setSurfaceDrawstyle();

    void                                    disableLighting(bool disable);
    bool                                    isLightingDisabled() const;

    virtual bool                            isGridVisualizationMode() const = 0;

    void                                    setScaleZAndUpdate(double scaleZ);
    virtual bool                            showActiveCellsOnly();
    virtual bool                            isUsingFormationNames() const = 0;

    virtual QImage                          snapshotWindowContent() override;
    virtual void                            zoomAll() override;
    void                                    forceShowWindowOn();

    // Animation
    int                                     currentTimeStep() const { return m_currentTimeStep;}
    void                                    setCurrentTimeStep(int frameIdx);
    void                                    setCurrentTimeStepAndUpdate(int frameIdx) override;
    virtual bool                            isTimeStepDependentDataVisible() const = 0;

    // Updating 
    void                                    updateCurrentTimeStepAndRedraw() override;
    virtual void                            scheduleGeometryRegen(RivCellSetEnum geometryType) = 0;
    void                                    scheduleCreateDisplayModelAndRedraw();
    void                                    createDisplayModelAndRedraw();
    void                                    createHighlightAndGridBoxDisplayModelWithRedraw();
    void                                    updateGridBoxData();
    void                                    updateAnnotationItems();   

    bool                                    isMasterView() const;

    cvf::ref<caf::DisplayCoordTransform>    displayCoordTransform() const override;

    virtual RimCase*                        ownerCase() const = 0;
    virtual std::vector<RimLegendConfig*>   legendConfigs() const = 0;

    cvf::Mat4d                              cameraPosition() const;
    cvf::Vec3d                              cameraPointOfInterest() const;

protected:
    static void                             removeModelByName(cvf::Scene* scene, const cvf::String& modelName);

    virtual void                            setDefaultView();
    void                                    disableGridBoxField();
    void                                    disablePerspectiveProjectionField();

    RimWellPathCollection*                  wellPathCollection() const;
    void                                    addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                                                const cvf::BoundingBox& wellPathClipBoundingBox);

    void                                    addDynamicWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                                                       const cvf::BoundingBox& wellPathClipBoundingBox);

    void                                    createHighlightAndGridBoxDisplayModel();

    // Implementation of RiuViewerToViewInterface
    virtual cvf::Color3f                    backgroundColor() const override { return m_backgroundColor(); }
    void                                    applyBackgroundColor();

    // Abstract methods to implement in subclasses

    virtual void                            axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) = 0;

    virtual void                            createDisplayModel() = 0;
    virtual void                            createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) = 0;
    
    virtual void                            updateDisplayModelVisibility();
    virtual void                            clampCurrentTimestep() = 0;

    virtual void                            updateCurrentTimeStep() = 0;
    virtual void                            onTimeStepChanged() = 0;
    virtual void                            updateStaticCellColors() = 0;
 
    virtual void                            updateScaleTransform() = 0;
    virtual cvf::Transform*                 scaleTransform() = 0;

    virtual void                            resetLegendsInViewer() = 0;

protected: // Fields
    caf::PdmField<int>                      m_currentTimeStep;

protected: 
    QPointer<RiuViewer>                     m_viewer;

    cvf::ref<cvf::ModelBasicList>           m_wellPathPipeVizModel;
    cvf::ref<cvf::ModelBasicList>           m_crossSectionVizModel;
    cvf::ref<cvf::ModelBasicList>           m_highlightVizModel;

    cvf::ref<RivWellPathsPartMgr>           m_wellPathsPartManager; 

private:
    // Overridden PdmObject methods:

    virtual caf::PdmFieldHandle*            userDescriptionField() override { return &name; }
    virtual void                            setupBeforeSave() override;
protected:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    // Overridden ViewWindow methods:

    virtual QWidget*                        createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                            updateViewWidgetAfterCreation() override; 
    virtual void                            updateMdiWindowTitle() override;
    virtual void                            deleteViewWidget() override;
    virtual QWidget*                        viewWidget() override;

    // Implementation of RiuViewerToViewInterface

    virtual void                            setCameraPosition(const cvf::Mat4d& cameraPosition) override               { m_cameraPosition = cameraPosition; }
    virtual void                            setCameraPointOfInterest(const cvf::Vec3d& cameraPointOfInterest) override { m_cameraPointOfInterest = cameraPointOfInterest;}
    virtual QString                         timeStepName(int frameIdx) const override;
    virtual void                            endAnimation() override;
    virtual caf::PdmObjectHandle*           implementingPdmObject() override  { return this; }
    virtual void                            handleMdiWindowClosed() override;
    virtual void                            setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry) override;

private:
    caf::PdmField<bool>                     m_disableLighting;
    caf::PdmField<cvf::Mat4d>               m_cameraPosition;
    caf::PdmField<cvf::Vec3d>               m_cameraPointOfInterest;
    caf::PdmField< cvf::Color3f >           m_backgroundColor;
    caf::PdmField<bool>                     m_showGridBox;

};
