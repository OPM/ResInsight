/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RicCreateWellPathFromSimulationWellFeature.h"

#include "RiaColorTables.h"

#include "RicEclipseWellFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"

#include "Riu3DMainWindowTools.h"

#include "Well/RigSimulationWellCenterLineCalculator.h"

#include "cvfVector3.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateWellPathFromSimulationWellFeature, "RicCreateWellPathFromSimulationWellFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellPathFromSimulationWellFeature::onActionTriggered( bool isChecked )
{
    auto createWellPathForSimulationWell = []( const RimSimWellInView& simulationWell ) -> std::vector<cvf::Vec3d>
    {
        auto simWellCellBranches        = simulationWell.wellBranchesForVisualization();
        const auto& [coords, wellCells] = RigSimulationWellCenterLineCalculator::extractBranchData( simWellCellBranches );

        if ( coords.empty() ) return {};

        // Take only the first branch.
        return coords.front();
    };

    auto filterWellPathPoints = []( const std::vector<cvf::Vec3d>& wellPathPoints, double distance )
    {
        std::vector<cvf::Vec3d> filteredWellPathPoints;
        if ( wellPathPoints.size() > 2 )
        {
            // Always keep first point
            filteredWellPathPoints.push_back( wellPathPoints.front() );

            // Keep only points that are a given distance apart to get
            // reasonable number of them in the well path.
            size_t lastUsedPoint = 0;
            for ( size_t i = 1; i < wellPathPoints.size() - 1; i++ )
            {
                if ( wellPathPoints[lastUsedPoint].pointDistance( wellPathPoints[i] ) > distance )
                {
                    filteredWellPathPoints.push_back( wellPathPoints[i] );
                    lastUsedPoint = i;
                }
            }

            // Always keep last point
            filteredWellPathPoints.push_back( wellPathPoints.back() );
        }

        return filteredWellPathPoints;
    };

    std::vector<RimSimWellInView*> selection = RicEclipseWellFeatureImpl::selectedWells();

    RimProject* project = RimProject::current();
    if ( !project ) return;

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( !wellPathCollection ) return;

    std::vector<RimWellPath*> newWellPaths;

    size_t numAddedWells         = 0;
    size_t modelledWellpathCount = wellPathCollection->modelledWellPathCount();

    RimModeledWellPath* lastAddedWell = nullptr;
    for ( const RimSimWellInView* simulationWell : selection )
    {
        auto wellPathPoints = createWellPathForSimulationWell( *simulationWell );

        std::vector<cvf::Vec3d> filteredWellPathPoints = filterWellPathPoints( wellPathPoints, 100.0 );

        // Need at least two points to add create a well
        if ( filteredWellPathPoints.size() > 2 )
        {
            auto newModeledWellPath = new RimModeledWellPath();
            newModeledWellPath->geometryDefinition()->createAndInsertTargets( filteredWellPathPoints );
            newModeledWellPath->createWellPathGeometry();
            newModeledWellPath->setUnitSystem( project->commonUnitSystemForAllCases() );

            newModeledWellPath->setWellPathColor(
                RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( modelledWellpathCount + numAddedWells ) );

            newModeledWellPath->setName( simulationWell->name );
            newWellPaths.push_back( newModeledWellPath );

            lastAddedWell = newModeledWellPath;
            numAddedWells++;
        }
    }

    wellPathCollection->addWellPaths( newWellPaths );
    wellPathCollection->uiCapability()->updateConnectedEditors();

    project->scheduleCreateDisplayModelAndRedrawAllViews();

    if ( lastAddedWell ) Riu3DMainWindowTools::selectAsCurrentItem( lastAddedWell->geometryDefinition() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellPathFromSimulationWellFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Well Path" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateWellPathFromSimulationWellFeature::isCommandEnabled() const
{
    return RicEclipseWellFeatureImpl::isAnyWellSelected();
}
