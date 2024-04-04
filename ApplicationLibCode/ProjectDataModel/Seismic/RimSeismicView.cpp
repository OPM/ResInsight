/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSeismicView.h"

#include "RigPolyLinesData.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimLegendConfig.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicDataInterface.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInViewCollection.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"

#include "RivPolylinePartMgr.h"

#include "cafPdmUiTreeOrdering.h"

#include "cafDisplayCoordTransform.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfString.h"
#include "cvfTransform.h"

CAF_PDM_SOURCE_INIT( RimSeismicView, "RimSeismicView", "SeismicView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicView::RimSeismicView()
{
    CAF_PDM_InitObject( "Seismic View", ":/SeismicView24x24.png" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );

    CAF_PDM_InitFieldNoDefault( &m_seismicSectionCollection, "SeismicSectionCollection", "Seismic Collection Field" );
    m_seismicSectionCollection = new RimSeismicSectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_annotationCollection, "AnnotationCollection", "Annotations" );
    m_annotationCollection = new RimAnnotationInViewCollection;

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView( this );

    m_scaleTransform = new cvf::Transform();

    m_surfaceVizModel = new cvf::ModelBasicList;
    m_surfaceVizModel->setName( "SurfaceModel" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicView::~RimSeismicView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::setSeismicData( RimSeismicDataInterface* data )
{
    m_seismicData = data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicDataInterface* RimSeismicView::seismicData() const
{
    return m_seismicData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::addSlice( RiaDefines::SeismicSectionType sectionType )
{
    auto section = m_seismicSectionCollection->addNewSection( sectionType );
    section->setSeismicData( m_seismicData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection* RimSeismicView::surfaceInViewCollection() const
{
    return m_surfaceCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicSectionCollection* RimSeismicView::seismicSectionCollection() const
{
    return m_seismicSectionCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimSeismicView::ownerCase() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::View3dContent RimSeismicView::viewContent() const
{
    return RiaDefines::View3dContent::SEISMIC;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicView::isGridVisualizationMode() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicView::isUsingFormationNames() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimLegendConfig*> RimSeismicView::legendConfigs() const
{
    std::vector<RimLegendConfig*> legends;

    if ( m_surfaceCollection )
    {
        for ( auto legendConfig : m_surfaceCollection->legendConfigs() )
        {
            legends.push_back( legendConfig );
        }
    }

    for ( auto section : seismicSectionCollection()->seismicSections() )
    {
        legends.push_back( section->legendConfig() );
    }

    legends.erase( std::remove( legends.begin(), legends.end(), nullptr ), legends.end() );

    return legends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::scheduleGeometryRegen( RivCellSetEnum geometryType )
{
    // no need do do anything here
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimSeismicView::domainBoundingBox()
{
    cvf::BoundingBox bb;

    if ( m_seismicData )
    {
        bb.add( *m_seismicData->boundingBox() );
    }
    return bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_seismicData )
    {
        updateGridBoxData();
        scheduleCreateDisplayModelAndRedraw();
    }
    else
    {
        Rim3dView::fieldChangedByUi( changedField, oldValue, newValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );

    genGrp->add( userDescriptionField() );
    genGrp->add( &m_seismicData );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    uiTreeOrdering.add( m_overlayInfoConfig() );
    uiTreeOrdering.add( seismicSectionCollection() );
    if ( surfaceInViewCollection() ) uiTreeOrdering.add( surfaceInViewCollection() );
    uiTreeOrdering.add( annotationCollection() );

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onCreateDisplayModel()
{
    if ( nativeOrOverrideViewer() == nullptr ) return;

    if ( !m_seismicData ) return;

    // Remove all existing frames from the viewer.
    nativeOrOverrideViewer()->removeAllFrames( isUsingOverrideViewer() );

    // Set the Main scene in the viewer.
    cvf::ref<cvf::Scene> mainScene = new cvf::Scene;

    // Seismic sections

    cvf::ref<caf::DisplayCoordTransform> transform = displayCoordTransform();
    m_seismicVizModel->removeAllParts();

    if ( m_polylinePartMgr.isNull() ) m_polylinePartMgr = new RivPolylinePartMgr( this, this, this );
    m_polylinePartMgr->appendDynamicGeometryPartsToModel( m_seismicVizModel.p(), transform.p(), domainBoundingBox() );

    m_seismicSectionCollection->appendPartsToModel( this, m_seismicVizModel.p(), transform.p(), domainBoundingBox() );
    mainScene->addModel( m_seismicVizModel.p() );
    nativeOrOverrideViewer()->setMainScene( mainScene.p(), isUsingOverrideViewer() );

    // Well path model

    m_wellPathPipeVizModel->removeAllParts();
    addWellPathsToModel( m_wellPathPipeVizModel.p(), domainBoundingBox(), m_seismicData->inlineSpacing() );
    nativeOrOverrideViewer()->addStaticModelOnce( m_wellPathPipeVizModel.p(), isUsingOverrideViewer() );

    // Surfaces

    m_surfaceVizModel->removeAllParts();
    if ( m_surfaceCollection )
    {
        bool nativeOnly = true;
        m_surfaceCollection->appendPartsToModel( m_surfaceVizModel.p(), scaleTransform(), nativeOnly );
        nativeOrOverrideViewer()->addStaticModelOnce( m_surfaceVizModel.p(), isUsingOverrideViewer() );
    }

    // Annotations

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;
    model->setName( "Annotations" );
    addAnnotationsToModel( model.p() );
    mainScene->addModel( model.p() );

    onUpdateLegends();
    if ( m_surfaceCollection )
    {
        m_surfaceCollection->applySingleColorEffect();
    }

    if ( m_seismicData ) nativeOrOverrideViewer()->setPointOfInterest( m_seismicData->boundingBox()->center() );

    m_overlayInfoConfig()->update3DInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onUpdateDisplayModelForCurrentTimeStep()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onClampCurrentTimestep()
{
    m_currentTimeStep = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSeismicView::onTimeStepCountRequested()
{
    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSeismicView::isTimeStepDependentDataVisible() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel )
{
    *xLabel = "E(x)";
    *yLabel = "N(y)";
    *zLabel = "Z";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts )
{
    // no action needed, might be needed if we want to hilite something later
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onUpdateStaticCellColors()
{
    // no action needed
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onUpdateLegends()
{
    if ( nativeOrOverrideViewer() )
    {
        if ( !isUsingOverrideViewer() )
        {
            nativeOrOverrideViewer()->removeAllColorLegends();
        }
        else
        {
            std::vector<RimLegendConfig*> legendConfs = this->legendConfigs();

            for ( auto legendConf : legendConfs )
            {
                nativeOrOverrideViewer()->removeColorLegend( legendConf->titledOverlayFrame() );
            }
        }
    }

    if ( !nativeOrOverrideViewer() )
    {
        return;
    }

    if ( m_surfaceCollection && m_surfaceCollection->isChecked() )
    {
        m_surfaceCollection->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
    }

    if ( m_seismicSectionCollection->isChecked() )
    {
        m_seismicSectionCollection->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer(), isUsingOverrideViewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onLoadDataAndUpdate()
{
    updateViewTreeItems( RiaDefines::ItemIn3dView::ALL );
    synchronizeLocalAnnotationsFromGlobal();
    onUpdateScaleTransform();

    updateMdiWindowVisibility();

    if ( m_surfaceCollection ) m_surfaceCollection->loadData();

    scheduleCreateDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::selectOverlayInfoConfig()
{
    Riu3DMainWindowTools::selectAsCurrentItem( m_overlayInfoConfig );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Transform* RimSeismicView::scaleTransform()
{
    return m_scaleTransform.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSeismicView::createAutoName() const
{
    if ( m_seismicData != nullptr )
    {
        return QString::fromStdString( m_seismicData->userDescription() );
    }

    return "Seismic View";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::updateGridBoxData()
{
    if ( viewer() )
    {
        viewer()->updateGridBoxData( m_scaleZ(), cvf::Vec3d::ZERO, backgroundColor(), domainBoundingBox(), fontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::updateViewTreeItems( RiaDefines::ItemIn3dView itemType )
{
    auto bitmaskEnum = BitmaskEnum( itemType );

    if ( bitmaskEnum.AnyOf( RiaDefines::ItemIn3dView::SURFACE ) )
    {
        RimSurfaceCollection* surfColl = RimTools::surfaceCollection();

        if ( surfColl && surfColl->containsSurface() )
        {
            if ( !m_surfaceCollection() )
            {
                m_surfaceCollection = new RimSurfaceInViewCollection();
            }

            m_surfaceCollection->setSurfaceCollection( surfColl );
            m_surfaceCollection->updateFromSurfaceCollection();
        }
        else
        {
            delete m_surfaceCollection;
        }
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigPolyLinesData> RimSeismicView::polyLinesData() const
{
    cvf::ref<RigPolyLinesData> pld = new RigPolyLinesData;

    if ( m_seismicData != nullptr )
    {
        m_seismicData->addSeismicOutline( pld.p() );
        pld->setLineAppearance( 1, { 255, 255, 255 }, false );
        pld->setZPlaneLock( false, 0.0 );
        pld->setVisibility( true, false );
    }
    else
    {
        pld->setVisibility( false, false );
    }

    return pld;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::setDefaultView()
{
    if ( viewer() )
    {
        viewer()->setDefaultView( cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSeismicView::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_seismicData )
    {
        RimTools::seismicDataOptionItems( &options, domainBoundingBox() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimSeismicView::characteristicCellSize() const
{
    if ( m_seismicData != nullptr )
    {
        return m_seismicData->inlineSpacing();
    }

    return Rim3dView::characteristicCellSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData RimSeismicView::histogramData()
{
    RigHistogramData histData;

    if ( m_seismicData )
    {
        auto xvals = m_seismicData->histogramXvalues();
        auto yvals = m_seismicData->histogramYvalues();
        if ( xvals.empty() || yvals.empty() ) return histData;

        histData.min  = xvals.front();
        histData.max  = xvals.back();
        histData.mean = 0.0;
        histData.sum  = 0.0;

        histData.histogram.resize( yvals.size() );

        int i = 0;
        for ( auto val : yvals )
        {
            histData.histogram[i++] = (size_t)val;
        }
    }
    return histData;
}
