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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmFieldCvfMat4d.h"
#include "cafAppEnum.h"

class RiuViewer;
class Rim3dOverlayInfoConfig;
class RimCase;
class RimCellRangeFilterCollection;

namespace cvf
{
    class BoundingBox;
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

    caf::PdmField<RimCellRangeFilterCollection*>    rangeFilterCollection;

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
   
    void                                    setShowFaultsOnly(bool showFaults);
    bool                                    isGridVisualizationMode() const;

    // Animation
    int                                     currentTimeStep()    { return m_currentTimeStep;}
    void                                    setCurrentTimeStep(int frameIdx);
    void                                    updateCurrentTimeStepAndRedraw();
    void                                    endAnimation();

    virtual void                            scheduleGeometryRegen(unsigned short geometryType) = 0;
    void                                    scheduleCreateDisplayModelAndRedraw();
    void                                    createDisplayModelAndRedraw();

public:
    virtual void                            loadDataAndUpdate() = 0;
    virtual RimCase*                        ownerCase() = 0;

    virtual caf::PdmFieldHandle*            objectToggleField()     { return &showWindow; }
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &name; }
protected:

    void                                    setDefaultView();
	void									addWellPathsToScene(cvf::Scene* scene, const cvf::Vec3d& displayModelOffset, double characteristicCellSize, const cvf::BoundingBox& boundingBox, cvf::Transform* scaleTransform);

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
 
    QPointer<RiuViewer>                     m_viewer;

    caf::PdmField<int>                      m_currentTimeStep;
    caf::PdmField<Rim3dOverlayInfoConfig*>  overlayInfoConfig;

    // Overridden PDM methods:
    virtual void                            setupBeforeSave();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

private:
    bool                                    m_previousGridModeMeshLinesWasFaults;

};



