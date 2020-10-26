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

#include "RimStreamlineInViewCollection.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigSimWellData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimStreamline.h"

#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include "cvfCollection.h"

#include <qdebug.h>

CAF_PDM_SOURCE_INIT( RimStreamlineInViewCollection, "StreamlineInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::RimStreamlineInViewCollection()
{
    CAF_PDM_InitScriptableObject( "Streamlines", ":/Erase.png", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_collectionName, "Name", "Name", "", "", "" );
    m_collectionName = "Streamlines";

    CAF_PDM_InitField( &m_phase,
                       "Phase",
                       caf::AppEnum<RiaDefines::PhaseType>( RiaDefines::PhaseType::OIL_PHASE ),
                       "Phase",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_streamlines, "Streamlines", "Streamlines", "", "", "" );
    m_streamlines.uiCapability()->setUiTreeHidden( true );

    m_eclipseCase = nullptr;

    // we are a topmost folder, do not delete us
    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStreamlineInViewCollection::~RimStreamlineInViewCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimStreamlineInViewCollection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::setEclipseCase( RimEclipseCase* reservoir )
{
    m_eclipseCase = reservoir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStreamlineInViewCollection::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimStreamlineInViewCollection::phase() const
{
    return m_phase();
}

QString RimStreamlineInViewCollection::gridResultNameFromPhase() const
{
    switch ( phase() )
    {
        case RiaDefines::PhaseType::GAS_PHASE:
            return "FLRGAS";
        case RiaDefines::PhaseType::OIL_PHASE:
            return "FLROIL";
        case RiaDefines::PhaseType::WATER_PHASE:
            return "FLRWAT";
        default:
            break;
    }
    CAF_ASSERT( false );
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<RigTracer>& RimStreamlineInViewCollection::tracers()
{
    m_activeTracers.clear();

    for ( auto streamline : m_streamlines() )
    {
        // TODO - add filter for active simulation wells here?
        m_activeTracers.push_back( streamline->tracer() );
    }
    return m_activeTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStreamlineInViewCollection::goForIt()
{
    // reset generated streamlines
    m_streamlines().clear();

    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    // get current simulation timestep
    int timeIdx = eclView->currentTimeStep();

    // get the simulation wells
    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<RigCell> seedCellsInjector;
    std::vector<RigCell> seedCellsProducer;

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    // TODO - add filter to select subset of simwells here? Depends on speed - use a view filter or a calculation filter?
    for ( auto swdata : simWellData )
    {
        if ( !swdata->hasWellResult( timeIdx ) || !swdata->hasAnyValidCells( timeIdx ) ) continue;

        RigWellResultFrame frame = swdata->wellResultFrame( timeIdx );

        for ( auto branch : frame.m_wellResultBranches )
        {
            for ( const auto& point : branch.m_branchResultPoints )
            {
                if ( point.isValid() && point.m_isOpen )
                {
                    RigCell cell = grids[point.m_gridIndex]->cell( point.m_gridCellIndex );
                    if ( frame.m_productionType == RigWellResultFrame::WellProductionType::PRODUCER )
                    {
                        seedCellsProducer.push_back( cell );
                    }
                    else if ( frame.m_productionType != RigWellResultFrame::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
                    {
                        seedCellsInjector.push_back( cell );
                    }
                }
            }
        }
    }

    qDebug() << "Found " << seedCellsProducer.size() << " producing cells";
    qDebug() << "Found " << seedCellsInjector.size() << " injector cells";

    const int reverseDirection = -1.0;
    const int normalDirection  = 1.0;

    for ( int cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++ )
    {
        for ( auto cell : seedCellsInjector )
        {
            generateTracer( cell, cubeFaceIdx, normalDirection, m_phase() );
        }
        for ( auto cell : seedCellsProducer )
        {
            generateTracer( cell, cubeFaceIdx, reverseDirection, m_phase() );
        }
    }
}

void RimStreamlineInViewCollection::generateTracer( RigCell cell, int faceIdx, double direction, RiaDefines::PhaseType phase )
{
}
