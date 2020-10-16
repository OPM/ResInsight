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

    CAF_PDM_InitField( &m_isActive, "isActive", false, "Active", "", "", "" );
    m_isActive.uiCapability()->setUiHidden( true );

    // CAF_PDM_InitScriptableFieldNoDefault( &m_streamlines, "Streamlines", "Streamlines", "", "", "" );
    // m_streamlines.uiCapability()->setUiTreeHidden( true );

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
void RimStreamlineInViewCollection::goForIt()
{
    // get the view
    RimEclipseView* eclView = nullptr;
    this->firstAncestorOrThisOfType( eclView );
    if ( !eclView ) return;

    int timeIdx = eclView->currentTimeStep();

    const cvf::Collection<RigSimWellData>& simWellData = eclipseCase()->eclipseCaseData()->wellResults();

    std::vector<RigCell> seedCells;

    std::vector<const RigGridBase*> grids;
    eclipseCase()->eclipseCaseData()->allGrids( &grids );

    for ( auto swdata : simWellData )
    {
        if ( !swdata->hasWellResult( timeIdx ) || !swdata->hasAnyValidCells( timeIdx ) ) continue;

        RigWellResultFrame frame = swdata->wellResultFrame( timeIdx );
        if ( frame.m_productionType != RigWellResultFrame::WellProductionType::PRODUCER ) continue;

        for ( auto branch : frame.m_wellResultBranches )
        {
            for ( const auto& point : branch.m_branchResultPoints )
            {
                if ( point.isValid() && point.m_isOpen )
                {
                    RigCell cell = grids[point.m_gridIndex]->cell( point.m_gridCellIndex );
                    seedCells.push_back( cell );
                }
            }
        }
    }

    qDebug() << "Found " << seedCells.size() << " producing cells";
}
