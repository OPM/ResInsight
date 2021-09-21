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

#include "RicCreateSurfaceIntersectionBandFeature.h"

#include "RimEnsembleSurface.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceIntersectionBand.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSurfaceIntersectionBandFeature, "RicCreateSurfaceIntersectionBandFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection* RicCreateSurfaceIntersectionBandFeature::surfaceCollection()
{
    RimProject* proj = RimProject::current();
    return proj->activeOilField()->surfaceCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateSurfaceIntersectionBandFeature::isCommandEnabled()
{
    auto* surfColl = RicCreateSurfaceIntersectionBandFeature::surfaceCollection();
    auto  surfaces = surfColl->ensembleSurfaces();

    return !surfaces.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSurfaceIntersectionBandFeature::onActionTriggered( bool isChecked )
{
    auto* intersection = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimExtrudedCurveIntersection>();
    if ( intersection )
    {
        RimEnsembleSurface* firstEnsembleSurface = nullptr;
        {
            auto surfColl = RicCreateSurfaceIntersectionBandFeature::surfaceCollection();
            auto surfaces = surfColl->ensembleSurfaces();
            if ( !surfaces.empty() ) firstEnsembleSurface = surfaces.front();
        }

        RimSurfaceIntersectionBand* objectToSelect = nullptr;

        if ( firstEnsembleSurface )
        {
            const double defaultOpacity = 0.6;

            // Create min/max band
            {
                auto surf1 =
                    firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::MIN );
                auto surf2 =
                    firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::MAX );

                if ( surf2 && surf1 )
                {
                    auto band = intersection->addIntersectionBand();
                    band->setSurfaces( surf1, surf2 );
                    band->setBandColor( surf2->color() );
                    band->setBandOpacity( defaultOpacity * 0.7 );

                    objectToSelect = band;
                }
            }

            // Create p10/p90 band
            {
                auto surf1 =
                    firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::P10 );
                auto surf2 =
                    firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::P90 );

                if ( surf2 && surf1 )
                {
                    auto band = intersection->addIntersectionBand();
                    band->setSurfaces( surf1, surf2 );
                    band->setBandColor( surf2->color() );
                    band->setBandOpacity( defaultOpacity );
                    band->setPolygonOffsetUnit( 100 );
                }
            }
        }

        intersection->updateAllRequiredEditors();

        Riu3DMainWindowTools::selectAsCurrentItem( objectToSelect );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateSurfaceIntersectionBandFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Surface Intersection Band" );
    actionToSetup->setIcon( QIcon( ":/ReservoirSurface16x16.png" ) );
}
