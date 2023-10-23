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

#include "RiaColorTables.h"

#include "RimAnnotationLineAppearance.h"
#include "RimEnsembleSurface.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateSurfaceIntersectionBandFeature, "RicCreateSurfaceIntersectionBandFeature" );

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
            auto surfColl = RimTools::surfaceCollection();
            auto surfaces = surfColl->ensembleSurfaces();
            if ( !surfaces.empty() ) firstEnsembleSurface = surfaces.front();
        }

        RimSurfaceIntersectionBand* objectToSelect = nullptr;

        const double defaultOpacity = 0.6;
        const auto   colors         = RiaColorTables::structuralUncertaintyColors();

        if ( firstEnsembleSurface )
        {
            // Create min/max band
            {
                auto surf1 = firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::MIN );
                auto surf2 = firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::MAX );

                if ( surf2 && surf1 )
                {
                    auto band = intersection->addIntersectionBand();
                    band->setSurfaces( surf1, surf2 );

                    auto color = colors.cycledColor3f( 0 );
                    band->setBandColor( color );
                    band->setBandOpacity( defaultOpacity );
                    band->setPolygonOffsetUnit( 0.08 );

                    band->lineAppearance()->setColor( color );

                    objectToSelect = band;
                }
            }

            // Create p10/p90 band
            {
                auto surf1 = firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::P10 );
                auto surf2 = firstEnsembleSurface->findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType::P90 );

                if ( surf2 && surf1 )
                {
                    auto band = intersection->addIntersectionBand();
                    band->setSurfaces( surf1, surf2 );

                    auto color = colors.cycledColor3f( 1 );
                    band->setBandColor( color );
                    band->setBandOpacity( defaultOpacity );
                    band->setPolygonOffsetUnit( 0.1 );

                    band->lineAppearance()->setColor( color );
                }
            }
        }
        else
        {
            auto band = intersection->addIntersectionBand();

            auto surfColl = RimTools::surfaceCollection();
            auto surfaces = surfColl->surfaces();

            if ( surfaces.size() > 1 )
            {
                band->setSurfaces( surfaces[0], surfaces[1] );
            }

            auto color = colors.cycledColor3f( 1 );
            band->setBandColor( color );
            band->setBandOpacity( defaultOpacity );
            band->setPolygonOffsetUnit( 0.1 );

            band->lineAppearance()->setColor( color );

            objectToSelect = band;
        }

        intersection->rebuildGeometryAndScheduleCreateDisplayModel();
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
