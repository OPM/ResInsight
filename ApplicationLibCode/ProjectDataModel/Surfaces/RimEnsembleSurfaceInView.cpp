/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleSurfaceInView.h"

#include "RimEnsembleSurface.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivSurfacePartMgr.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimEnsembleSurfaceInView, "EnsembleSurfaceInView" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurfaceInView::RimEnsembleSurfaceInView()
{
    CAF_PDM_InitObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleSurfaceName, "EnsembleSurfaceName", "EnsembleSurfaceName", "", "", "" );
    m_ensembleSurfaceName.registerGetMethod( this, &RimEnsembleSurfaceInView::name );
    m_ensembleSurfaceName.uiCapability()->setUiReadOnly( true );
    m_ensembleSurfaceName.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_surfacesInView, "SurfacesInViewField", "SurfacesInViewField", "", "", "" );
    m_surfacesInView.uiCapability()->setUiTreeHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleSurface, "EnsembleSurface", "EnsembleSurface", "", "", "" );
    m_ensembleSurface.uiCapability()->setUiTreeHidden( true );

    nameField()->uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurfaceInView::~RimEnsembleSurfaceInView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleSurfaceInView::userDescriptionField()
{
    return &m_ensembleSurfaceName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleSurfaceInView::name() const
{
    if ( m_ensembleSurface ) return m_ensembleSurface->name();

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurface* RimEnsembleSurfaceInView::ensembleSurface() const
{
    return m_ensembleSurface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::setEnsembleSurface( RimEnsembleSurface* surfcoll )
{
    m_ensembleSurface = surfcoll;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::updateAllViewItems()
{
    syncSurfacesWithView();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::syncSurfacesWithView()
{
    // Delete surfaceInView without any real Surface connection

    std::vector<RimSurfaceInView*> surfsInView = m_surfacesInView.childObjects();

    for ( auto surf : surfsInView )
    {
        if ( !surf->surface() )
        {
            m_surfacesInView.removeChildObject( surf );
            delete surf;
        }
    }

    // Create new surfade entries and reorder
    std::vector<RimSurfaceInView*> orderedSurfs;

    if ( m_ensembleSurface )
    {
        // pick up the surfaces and the order from the surface collection
        std::vector<RimSurface*> surfs = m_ensembleSurface->surfaces();
        for ( auto surf : surfs )
        {
            // check if this is a surface we need to create
            RimSurfaceInView* viewSurf = this->getSurfaceInViewForSurface( surf );
            if ( viewSurf == nullptr )
            {
                RimSurfaceInView* newSurfInView = new RimSurfaceInView();
                newSurfInView->setSurface( surf );
                orderedSurfs.push_back( newSurfInView );
            }
            else
            {
                orderedSurfs.push_back( viewSurf );
            }
        }

        // make sure our view surfaces have the same order as the source surface collection
        m_surfacesInView.clear();
        for ( auto viewSurf : orderedSurfs )
        {
            m_surfacesInView.push_back( viewSurf );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::loadData()
{
    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::clearGeometry()
{
    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->clearGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::appendPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( !isChecked() ) return;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->surfacePartMgr()->appendIntersectionGeometryPartsToModel( model, scaleTransform );
        }
    }

    model->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    this->updateUiIconFromToggleField();

    if ( changedField == &m_isChecked )
    {
        RimGridView* ownerView;
        this->firstAncestorOrThisOfTypeAsserted( ownerView );
        ownerView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::initAfterRead()
{
    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInView* RimEnsembleSurfaceInView::getSurfaceInViewForSurface( const RimSurface* surf ) const
{
    for ( auto surfInView : m_surfacesInView )
    {
        if ( surfInView->surface() == surf )
        {
            return surfInView;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex )
{
    if ( !this->isChecked() ) return;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            bool useNativeSurfaceColors = false;

            if ( surf->isNativeSurfaceResultsActive() ) useNativeSurfaceColors = true;

            if ( !useNativeSurfaceColors )
            {
                bool showResults = surf->activeSeparateResultDefinition()
                                       ? surf->activeSeparateResultDefinition()->hasResult()
                                       : hasGeneralCellResult;

                if ( showResults )
                {
                    surf->surfacePartMgr()->updateCellResultColor( timeStepIndex );
                }
                else
                {
                    useNativeSurfaceColors = true;
                }
            }

            if ( useNativeSurfaceColors )
            {
                surf->surfacePartMgr()->updateNativeSurfaceColors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::applySingleColorEffect()
{
    if ( !this->isChecked() ) return;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() )
        {
            surf->surfacePartMgr()->updateNativeSurfaceColors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleSurfaceInView::hasAnyActiveSeparateResults()
{
    if ( !this->isChecked() ) return false;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() && surf->activeSeparateResultDefinition() &&
             surf->activeSeparateResultDefinition()->hasResult() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurfaceInView::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
                                                                    bool       isUsingOverrideViewer )
{
    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->updateLegendRangesTextAndVisibility( nativeOrOverrideViewer, isUsingOverrideViewer );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRegularLegendConfig*> RimEnsembleSurfaceInView::legendConfigs()
{
    std::vector<RimRegularLegendConfig*> configs;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->isActive() && surf->surfaceResultDefinition() )
        {
            configs.push_back( surf->surfaceResultDefinition()->legendConfig() );
        }
    }

    return configs;
}
