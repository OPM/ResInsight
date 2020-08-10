/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimSurfaceInViewCollection.h"

#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceInView.h"
#include "RimSurfaceResultDefinition.h"

#include "RivSurfacePartMgr.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimSurfaceInViewCollection, "SurfaceInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::RimSurfaceInViewCollection()
{
    CAF_PDM_InitObject( "Surfaces", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_surfacesInView, "SurfacesInViewField", "SurfacesInViewField", "", "", "" );
    m_surfacesInView.uiCapability()->setUiTreeHidden( true );

    setName( "Surfaces" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceInViewCollection::~RimSurfaceInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateFromSurfaceCollection()
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

    // Create new entries

    RimProject*           proj     = RimProject::current();
    RimSurfaceCollection* surfColl = proj->activeOilField()->surfaceCollection();

    if ( surfColl )
    {
        std::vector<RimSurface*> surfs = surfColl->surfaces();

        for ( auto surf : surfs )
        {
            if ( !this->hasSurfaceInViewForSurface( surf ) )
            {
                RimSurfaceInView* newSurfInView = new RimSurfaceInView();
                newSurfInView->setSurface( surf );
                m_surfacesInView.push_back( newSurfInView );
            }
        }
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::loadData()
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
void RimSurfaceInViewCollection::clearGeometry()
{
    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        surf->clearGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::appendPartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
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
void RimSurfaceInViewCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
void RimSurfaceInViewCollection::initAfterRead()
{
    this->updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSurfaceInViewCollection::hasSurfaceInViewForSurface( const RimSurface* surf ) const
{
    for ( auto surfInView : m_surfacesInView )
    {
        if ( surfInView->surface() == surf )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceInViewCollection::updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex )
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
void RimSurfaceInViewCollection::applySingleColorEffect()
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
bool RimSurfaceInViewCollection::hasAnyActiveSeparateResults()
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
void RimSurfaceInViewCollection::updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer,
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
std::vector<RimRegularLegendConfig*> RimSurfaceInViewCollection::legendConfigs()
{
    std::vector<RimRegularLegendConfig*> configs;

    for ( RimSurfaceInView* surf : m_surfacesInView )
    {
        if ( surf->surfaceResultDefinition() )
        {
            configs.push_back( surf->surfaceResultDefinition()->legendConfig() );
        }
    }

    return configs;
}
