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

#include "RiaDefines.h"

#include "RimNameConfig.h"
#include "RimViewWindow.h"

#include "RiuViewerToViewInterface.h"

#include "RivAnnotationTools.h"
#include "RivCellSetEnum.h"
#include "RivNamedVisualizationModels.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include "cafFontTools.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfCollection.h"
#include "cvfObject.h"

#include <QPointer>
#include <QTimer>

class RimCase;
class RimLegendConfig;
class RimWellPathCollection;
class RimAnnotationInViewCollection;
class RiuViewer;
class RivAnnotationsPartMgr;
class RivMeasurementPartMgr;
class RivWellPathsPartMgr;
class RiuMainWindowBase;
class RimViewNameConfig;

namespace cvf
{
class BoundingBox;
class ModelBasicList;
class Scene;
class String;
class Transform;
class Part;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

enum PartRenderMaskEnum
{
    surfaceBit               = 1,
    meshSurfaceBit           = 2,
    faultBit                 = 4,
    meshFaultBit             = 8,
    intersectionCellFaceBit  = 16,
    intersectionCellMeshBit  = 32,
    intersectionFaultMeshBit = 64
};

//==================================================================================================
///
///
//==================================================================================================
class Rim3dView : public RimViewWindow, public RiuViewerToViewInterface, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dView();
    ~Rim3dView() override;

    int id() const final;

    // Names for models in m_vizModels
    static const char* wellPathPipeModelName() { return "WellPathPipeModel"; }
    static const char* seismicSectionModelName() { return "SeismicSectionModel"; }

    // Public fields:

    caf::PdmField<bool> isPerspectiveView;
    caf::PdmField<int>  maximumFrameRate;

    // Draw style

    enum SurfaceModeType
    {
        SURFACE,
        FAULTS,
        NO_SURFACE
    };

    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>> meshMode;
    caf::PdmField<caf::AppEnum<SurfaceModeType>>          surfaceMode;

    // Default: no owner case. Grid views (RimGridView) re-abstract this.
    virtual RimCase* ownerCase() const { return nullptr; }
    RiuViewer*       viewer() const;

    void               setName( const QString& name );
    QString            name() const;
    QString            autoName() const;
    RimViewNameConfig* nameConfig() const;

    virtual RiaDefines::View3dContent viewContent() const = 0;

    void           setMeshOnlyDrawstyle();
    void           setMeshSurfDrawstyle();
    void           setSurfOnlyDrawstyle();
    void           setFaultMeshSurfDrawstyle();
    void           setSurfaceDrawstyle();
    void           setShowGridBox( bool showGridBox );
    virtual bool   isShowingActiveCellsOnly();
    virtual bool   isGridVisualizationMode() const = 0;
    virtual double characteristicCellSize() const;

    void         setBackgroundColor( const cvf::Color3f& newBackgroundColor );
    cvf::Color3f backgroundColor() const override; // Implementation of RiuViewerToViewInterface

    int  fontSize() const override;
    void updateFonts() override;
    void applyFontChanges();

    void disableLighting( bool disable );
    bool isLightingDisabled() const;

    virtual bool                          isUsingFormationNames() const { return false; }
    cvf::ref<caf::DisplayCoordTransform>  displayCoordTransform() const override;
    virtual std::vector<RimLegendConfig*> legendConfigs() const = 0;

    QImage captureSnapshot( int width, int height ) override;
    QImage snapshotWindowContent() override;
    void   zoomAll() override;
    void   forceShowWindowOn();

    // Timestep control
    int                 currentTimeStep() const;
    void                setCurrentTimeStep( int frameIdx );
    void                setCurrentTimeStepAndUpdate( int frameIdx ) override;
    bool                isTimeStepDependentDataVisibleInThisOrComparisonView() const;
    size_t              timeStepCount();
    QString             timeStepName( int frameIdx ) const override;
    virtual QStringList timeStepStrings() const;

    // Animation control
    caf::Signal<> updateAnimations;
    void          requestAnimationTimer();
    void          releaseAnimationTimer();

    // Updating
    void         scheduleCreateDisplayModelAndRedraw();
    virtual void scheduleGeometryRegen( RivCellSetEnum geometryType ) = 0;

    void createDisplayModelAndRedraw();
    void updateDisplayModelForCurrentTimeStepAndRedraw();
    void createHighlightAndGridBoxDisplayModelAndRedraw();
    void createMeasurementDisplayModelAndRedraw();
    void updateAnnotationItems();
    void resetLegends();

    virtual void             updateGridBoxData();
    virtual cvf::BoundingBox domainBoundingBox();

    void         setScaleZ( double scaleZ );
    void         setScaleZAndUpdate( double scaleZ );
    void         updateScaling();
    void         updateZScaleLabel();
    virtual bool isScaleZEditable() const;
    double       scaleZ() const;

    virtual QString activeFiltersDisplayText() const;
    void            updateFilterLabel();

    bool                    isMasterView() const;
    Rim3dView*              activeComparisonView() const;
    void                    setComparisonView( Rim3dView* compView );
    std::set<Rim3dView*>    viewsUsingThisAsComparisonView() const;
    void                    updateWindowTitle() override;
    std::vector<Rim3dView*> validComparisonViews() const;

    RimViewLinker*     assosiatedViewLinker() const override;
    RimViewController* viewController() const override;

    virtual void updateViewTreeItems( RiaDefines::ItemIn3dView itemType );

    RimAnnotationInViewCollection* annotationCollection() const;
    void                           synchronizeLocalAnnotationsFromGlobal();

    void dockInMainWindow();
    void dockInPlotWindow();

protected:
    static void removeModelByName( cvf::Scene* scene, const cvf::String& modelName );

    virtual void setDefaultView();
    cvf::Mat4d   cameraPosition() const;
    cvf::Vec3d   cameraPointOfInterest() const;
    bool         isCameraOriented( const cvf::Vec3d& viewDirection, const cvf::Vec3d& upDirection ) const;

    void disableGridBoxField();
    void disablePerspectiveProjectionField();
    void updateDisplayModelVisibility();

    bool hasVisibleTimeStepDependent3dWellLogCurves() const;

    RimWellPathCollection* wellPathCollection() const;

    void addWellPathsToModel( cvf::ModelBasicList*    wellPathModelBasicList,
                              const cvf::BoundingBox& wellPathClipBoundingBox,
                              double                  characteristicCellSize );
    void addDynamicWellPathsToModel( cvf::ModelBasicList*    wellPathModelBasicList,
                                     const cvf::BoundingBox& wellPathClipBoundingBox,
                                     double                  characteristicCellSize );
    void addAnnotationsToModel( cvf::ModelBasicList* annotationsModel );
    void addMeasurementToModel( cvf::ModelBasicList* measureModel );

    // Override viewer

    RiuViewer* nativeOrOverrideViewer() const;
    bool       isUsingOverrideViewer() const;

    void hideComparisonViewField();

    // Abstract methods to implement in subclasses

    virtual void onUpdateDisplayModelVisibility() {};
    virtual void onClearReservoirCellVisibilitiesIfNecessary() {};
    virtual void onResetLegendsInViewer();
    virtual void onUpdateScaleTransform();

    virtual void onCreateDisplayModel() = 0;

    // Time step control. Default: no time step (single static frame). Grid views re-abstract these.
    virtual void   onUpdateDisplayModelForCurrentTimeStep() {}
    virtual void   onClampCurrentTimestep() { m_currentTimeStep = 0; }
    virtual size_t onTimeStepCountRequested() { return 1; }
    virtual bool   isTimeStepDependentDataVisible() const { return false; }

    virtual void defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) = 0;

    // Default: no-op. Grid views (per-cell data) re-abstract these.
    virtual void onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts ) {}
    virtual void onUpdateStaticCellColors() {}

    virtual void onUpdateLegends() = 0;

    virtual cvf::Transform* scaleTransform() = 0;

    void onViewNavigationChanged() override;

    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* backgroundColorField();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void setupBeforeSave() override;

    void     updateViewWidgetAfterCreation() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

    void setCameraPosition( const cvf::Mat4d& cameraPosition ) override;
    void setCameraPointOfInterest( const cvf::Vec3d& cameraPointOfInterest ) override;

protected:
    // Timestep Field. Children clamps this differently
    caf::PdmField<int> m_currentTimeStep;

    // Named viz models (well path pipes, surfaces, intersections, etc.) - see RivNamedVisualizationModels.
    RivNamedVisualizationModels   m_vizModels;
    cvf::ref<RivWellPathsPartMgr> m_wellPathsPartManager;

    caf::PdmField<double> m_scaleZ;

    caf::PdmChildField<RimAnnotationInViewCollection*> m_annotationCollection;

private:
    friend class RimProject;

    void setId( int id );
    void assignIdIfNecessary() final;

    void     deleteViewWidget() override;
    QWidget* viewWidget() override;

    // Implementation of RimNameConfigHolderInterface
    void performAutoNameUpdate() final;

    void endAnimation() override;

    caf::PdmObjectHandle* implementingPdmObject() override;

    // Pure private methods

    void createHighlightAndGridBoxDisplayModel();
    void appendMeasurementToModel();
    void appendAnnotationsToModel();
    void updateScreenSpaceModel();

    // Pure private methods : Override viewer and comparison view

    void       setOverrideViewer( RiuViewer* overrideViewer );
    Rim3dView* prepareComparisonView();
    void       restoreComparisonView();

    RimViewLinker* viewLinkerIfMasterView() const;

private:
    QPointer<RiuViewer> m_viewer;
    QPointer<RiuViewer> m_overrideViewer;
    bool                m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw; // To avoid infinite recursion if comparison views
    // are pointing to each other.

    // Fields
    caf::PdmField<int>                     m_id;
    caf::PdmChildField<RimViewNameConfig*> m_nameConfig;
    caf::PdmField<bool>                    m_disableLighting;
    caf::PdmField<cvf::Color3f>            m_backgroundColor;
    caf::PdmField<bool>                    m_showGridBox;
    caf::PdmField<bool>                    m_showZScaleLabel;
    caf::PdmPtrField<Rim3dView*>           m_comparisonView;

    // Camera position and point of interest. The member variables are mutable to allow for setting them from const methods.
    // The camera position and point of interest can change rapidly as the user interacts with the 3D view. Only update the Pdm field values
    // when the application requests the camera position or point of interest.
    mutable caf::PdmField<cvf::Mat4d>   m_cameraPosition;
    mutable caf::PdmField<cvf::Vec3d>   m_cameraPointOfInterest;
    caf::PdmProxyValueField<cvf::Vec3d> m_cameraPointOfInterestProxy;
    caf::PdmProxyValueField<cvf::Mat4d> m_cameraPositionProxy;

    caf::PdmField<bool>                                                    m_useCustomAnnotationStrategy;
    caf::PdmField<caf::AppEnum<RivAnnotationTools::LabelPositionStrategy>> m_annotationStrategy;
    caf::PdmField<int>                                                     m_annotationCountHint;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_fontSize;

    // 3D display model data
    cvf::ref<RivAnnotationsPartMgr> m_annotationsPartManager;
    cvf::ref<RivMeasurementPartMgr> m_measurementPartManager;

    // Timer for animations
    std::unique_ptr<QTimer> m_animationTimer;
    const int               m_animationIntervalMillisec;
    int                     m_animationTimerUsers;
};
