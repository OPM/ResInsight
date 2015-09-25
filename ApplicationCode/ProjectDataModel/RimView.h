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

#include "cvfArray.h"
#include "cvfBase.h"
#include "cvfObject.h"


#include <QPointer>

class Rim3dOverlayInfoConfig;
class RimCase;
class RimCellRangeFilterCollection;
class RiuViewer;
class RimViewLinker;
class RimViewController;

namespace cvf
{
    class BoundingBox;
    class ModelBasicList;
    class Scene;
    class Transform;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimView : public caf::PdmObject
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
    caf::PdmField< cvf::Color3f >           backgroundColor;

    caf::PdmField<int>                      maximumFrameRate;
    caf::PdmField<bool>                     hasUserRequestedAnimation;

    RimCellRangeFilterCollection*           rangeFilterCollection();
    const RimCellRangeFilterCollection*     rangeFilterCollection() const;
    void                                    setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc);

    caf::PdmField< std::vector<int> >       windowGeometry;


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

    void                                    setMeshOnlyDrawstyle();
    void                                    setMeshSurfDrawstyle();
    void                                    setSurfOnlyDrawstyle();
    void                                    setFaultMeshSurfDrawstyle();
    void                                    setSurfaceDrawstyle();

    void                                    disableLighting(bool disable);
    bool                                    isLightingDisabled() const;
   
    void                                    setShowFaultsOnly(bool showFaults);
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

    RimViewController*                      viewController() const;
    bool                                    isMasterView() const;
    RimViewLinker*                          assosiatedViewLinker() const;

    cvf::ref<cvf::UByteArray>               currentTotalCellVisibility();

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

    virtual void                            createDisplayModel() = 0;
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

    QPointer<RiuViewer>                     m_viewer;

    caf::PdmField<int>                                  m_currentTimeStep;
    caf::PdmChildField<Rim3dOverlayInfoConfig*>         m_overlayInfoConfig;

    caf::PdmChildField<RimCellRangeFilterCollection*>   m_rangeFilterCollection;
    caf::PdmPointer<RimCellRangeFilterCollection>       m_overrideRangeFilterCollection;

    // Overridden PDM methods:
    virtual void                            setupBeforeSave();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    cvf::ref<cvf::UByteArray>               m_currentReservoirCellVisibility;

private:
    RimViewLinker*                          viewLinkerIfMasterView() const;

private:
    bool                                    m_previousGridModeMeshLinesWasFaults;
    caf::PdmField<bool>                     m_disableLighting;
};



