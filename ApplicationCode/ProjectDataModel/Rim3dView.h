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

#include "RivCellSetEnum.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfCollection.h"
#include "cvfObject.h"

#include <QPointer>

class RimCase;
class RimLegendConfig;
class RimWellPathCollection;
class RiuViewer;
class RivAnnotationsPartMgr;
class RivMeasurementPartMgr;
class RivWellPathsPartMgr;
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
    Rim3dView( void );
    ~Rim3dView( void ) override;

    int id() const final;

    // Public fields:

    caf::PdmField<double> scaleZ;
    caf::PdmField<bool>   isPerspectiveView;
    caf::PdmField<int>    maximumFrameRate;
    caf::PdmField<bool>   hasUserRequestedAnimation;

    // Draw style

    enum SurfaceModeType
    {
        SURFACE,
        FAULTS,
        NO_SURFACE
    };

    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>> meshMode;
    caf::PdmField<caf::AppEnum<SurfaceModeType>>          surfaceMode;

    virtual RimCase* ownerCase() const = 0;
    RiuViewer*       viewer() const;

    void    setName( const QString& name );
    QString name() const;
    QString autoName() const;

    void         setMeshOnlyDrawstyle();
    void         setMeshSurfDrawstyle();
    void         setSurfOnlyDrawstyle();
    void         setFaultMeshSurfDrawstyle();
    void         setSurfaceDrawstyle();
    void         setShowGridBox( bool showGridBox );
    virtual bool isShowingActiveCellsOnly();
    virtual bool isGridVisualizationMode() const = 0;

    void         setBackgroundColor( const cvf::Color3f& newBackgroundColor );
    cvf::Color3f backgroundColor() const override; // Implementation of RiuViewerToViewInterface
    void         applyBackgroundColorAndFontChanges();

    int  fontSize() const override;
    void updateFonts() override;

    void disableLighting( bool disable );
    bool isLightingDisabled() const;

    virtual bool                          isUsingFormationNames() const = 0;
    cvf::ref<caf::DisplayCoordTransform>  displayCoordTransform() const override;
    virtual std::vector<RimLegendConfig*> legendConfigs() const = 0;

    QImage snapshotWindowContent() override;
    void   zoomAll() override;
    void   forceShowWindowOn();

    // Animation
    int     currentTimeStep() const;
    void    setCurrentTimeStep( int frameIdx );
    void    setCurrentTimeStepAndUpdate( int frameIdx ) override;
    bool    isTimeStepDependentDataVisibleInThisOrComparisonView() const;
    size_t  timeStepCount();
    QString timeStepName( int frameIdx ) const override;

    // Updating
    void         scheduleCreateDisplayModelAndRedraw();
    virtual void scheduleGeometryRegen( RivCellSetEnum geometryType ) = 0;

    void createDisplayModelAndRedraw();
    void updateDisplayModelForCurrentTimeStepAndRedraw();
    void createHighlightAndGridBoxDisplayModelAndRedraw();
    void createMeasurementDisplayModelAndRedraw();
    void updateGridBoxData();
    void updateAnnotationItems();
    void resetLegends();

    void setScaleZAndUpdate( double scaleZ );
    void updateScaling();
    void updateZScaleLabel();
    bool isScaleZEditable();

    bool                 isMasterView() const;
    Rim3dView*           activeComparisonView() const;
    void                 setComparisonView( Rim3dView* compView );
    std::set<Rim3dView*> viewsUsingThisAsComparisonView();

protected:
    static void removeModelByName( cvf::Scene* scene, const cvf::String& modelName );

    virtual void       setDefaultView();
    cvf::Mat4d         cameraPosition() const;
    cvf::Vec3d         cameraPointOfInterest() const;
    RimViewNameConfig* nameConfig() const;

    void disableGridBoxField();
    void disablePerspectiveProjectionField();
    void updateDisplayModelVisibility();

    bool hasVisibleTimeStepDependent3dWellLogCurves() const;

    RimWellPathCollection* wellPathCollection() const;

    void addWellPathsToModel( cvf::ModelBasicList* wellPathModelBasicList, const cvf::BoundingBox& wellPathClipBoundingBox );
    void addDynamicWellPathsToModel( cvf::ModelBasicList*    wellPathModelBasicList,
                                     const cvf::BoundingBox& wellPathClipBoundingBox );
    void addAnnotationsToModel( cvf::ModelBasicList* annotationsModel );
    void addMeasurementToModel( cvf::ModelBasicList* measureModel );
    void addPolylineSelectionToModel( cvf::ModelBasicList* model );

    // Override viewer

    RiuViewer* nativeOrOverrideViewer() const;
    bool       isUsingOverrideViewer() const;

    void hideComparisonViewField();

    // Abstract methods to implement in subclasses

    virtual void   onCreateDisplayModel()                   = 0;
    virtual void   onUpdateDisplayModelForCurrentTimeStep() = 0;
    virtual void   onUpdateDisplayModelVisibility(){};
    virtual void   onClampCurrentTimestep()   = 0;
    virtual size_t onTimeStepCountRequested() = 0;

    virtual void onClearReservoirCellVisibilitiesIfNecessary(){};
    virtual bool isTimeStepDependentDataVisible() const                                            = 0;
    virtual void defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) = 0;
    virtual void onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts )          = 0;
    virtual void onUpdateStaticCellColors()                                                        = 0;
    virtual void onResetLegendsInViewer()                                                          = 0;
    virtual void onUpdateLegends()                                                                 = 0;

    virtual void            onUpdateScaleTransform() = 0;
    virtual cvf::Transform* scaleTransform()         = 0;

protected:
    // Overridden PdmObject methods:

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* backgroundColorField();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

    void initAfterRead() override;
    void setupBeforeSave() override;

    // Overridden ViewWindow methods:
    void     updateViewWidgetAfterCreation() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

protected:
    // Timestep Field. Children clamps this differently
    caf::PdmField<int> m_currentTimeStep;

    // 3D display model data
    cvf::ref<cvf::ModelBasicList> m_wellPathPipeVizModel;
    cvf::ref<cvf::ModelBasicList> m_intersectionVizModel;
    cvf::ref<RivWellPathsPartMgr> m_wellPathsPartManager;

private:
    friend class RimProject;

    void setId( int id );
    void assignIdIfNecessary() final;

    void     updateMdiWindowTitle() override;
    void     deleteViewWidget() override;
    QWidget* viewWidget() override;

    // Implementation of RimNameConfigHolderInterface
    void performAutoNameUpdate() override final;

    // Implementation of RiuViewerToViewInterface

    void setCameraPosition( const cvf::Mat4d& cameraPosition ) override;
    void setCameraPointOfInterest( const cvf::Vec3d& cameraPointOfInterest ) override;

    void endAnimation() override;

    caf::PdmObjectHandle* implementingPdmObject() override;

    void handleMdiWindowClosed() override;
    void setMdiWindowGeometry( const RimMdiWindowGeometry& windowGeometry ) override;

    // Pure private methods

    void createHighlightAndGridBoxDisplayModel();
    void appendAnnotationsToModel();
    void appendMeasurementToModel();
    void appendPolylineSelectionToModel();

    // Pure private methods : Override viewer and comparison view

    void       setOverrideViewer( RiuViewer* overrideViewer );
    Rim3dView* prepareComparisonView();
    void       restoreComparisonView();

private:
    QPointer<RiuViewer> m_viewer;
    QPointer<RiuViewer> m_overrideViewer;
    bool m_isCallingUpdateDisplayModelForCurrentTimestepAndRedraw; // To avoid infinite recursion if comparison views
                                                                   // are pointing to each other.

    // Fields
    caf::PdmField<int>                     m_id;
    caf::PdmField<QString>                 m_name_OBSOLETE;
    caf::PdmChildField<RimViewNameConfig*> m_nameConfig;
    caf::PdmField<bool>                    m_disableLighting;
    caf::PdmField<cvf::Mat4d>              m_cameraPosition;
    caf::PdmField<cvf::Vec3d>              m_cameraPointOfInterest;
    caf::PdmField<cvf::Color3f>            m_backgroundColor;
    caf::PdmField<bool>                    m_showGridBox;
    caf::PdmField<bool>                    m_showZScaleLabel;
    caf::PdmPtrField<Rim3dView*>           m_comparisonView;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_fontSize;

    // 3D display model data
    cvf::ref<cvf::ModelBasicList>   m_highlightVizModel;
    cvf::ref<RivAnnotationsPartMgr> m_annotationsPartManager;
    cvf::ref<RivMeasurementPartMgr> m_measurementPartManager;
};
