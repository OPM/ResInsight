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

#include "Rim3dOverlayInfoConfig.h"
#include "RimLegendConfig.h"
#include "RimRegularLegendConfig.h"
#include "RimSeismicData.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "RimSurfaceInViewCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuViewer.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfString.h"
#include "cvfTransform.h"

CAF_PDM_SOURCE_INIT( RimSeismicView, "RimSeismicView", "SeismicView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicView::RimSeismicView()
{
    CAF_PDM_InitObject( "Seismic View", ":/Seismic16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_seismicData, "SeismicData", "Seismic Data" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceCollection, "SurfaceInViewCollection", "Surface Collection Field" );
    m_surfaceCollection.uiCapability()->setUiTreeHidden( true );
    m_surfaceCollection = new RimSurfaceInViewCollection();

    CAF_PDM_InitFieldNoDefault( &m_seismicSectionCollection, "SeismicSectionCollection", "Seismic Collection Field" );
    m_seismicSectionCollection.uiCapability()->setUiTreeHidden( true );
    m_seismicSectionCollection = new RimSeismicSectionCollection();

    CAF_PDM_InitFieldNoDefault( &m_overlayInfoConfig, "OverlayInfoConfig", "Info Box" );
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    // m_overlayInfoConfig->setReservoirView( this );
    m_overlayInfoConfig.uiCapability()->setUiTreeHidden( true );

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
void RimSeismicView::initAfterRead()
{
    Rim3dView::initAfterRead();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::setSeismicData( RimSeismicData* data )
{
    m_seismicData = data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::addSlice( RiaDefines::SeismicSliceDirection sliceType )
{
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

    for ( auto legendConfig : surfaceInViewCollection()->legendConfigs() )
    {
        legends.push_back( legendConfig );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicView::onCreateDisplayModel()
{
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
    // updateSurfacesInViewTreeItems();

    onUpdateScaleTransform();

    updateMdiWindowVisibility();

    m_surfaceCollection->loadData();

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
