/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "RicNewEditableWellPathFeature.h"

#include "RiaColorTables.h"

#include "RigEclipseCaseData.h"
#include "Well/RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimFileWellPath.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEditableWellPathFeature, "RicNewEditableWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEditableWellPathFeature::isCommandEnabled() const
{
    {
        const auto targets = caf::SelectionManager::instance()->objectsByType<RimWellPath>();
        if ( !targets.empty() )
        {
            return true;
        }
    }
    {
        const auto wellPathColl = caf::SelectionManager::instance()->objectsByType<RimWellPathCollection>();
        if ( !wellPathColl.empty() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    if ( project && RimProject::current()->activeOilField() )
    {
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            std::vector<RimWellPath*> newWellPaths;
            auto                      newModeledWellPath = new RimModeledWellPath();
            newWellPaths.push_back( newModeledWellPath );

            newModeledWellPath->setUnitSystem( project->commonUnitSystemForAllCases() );

            RimFileWellPath* sourceWellPath           = caf::SelectionManager::instance()->selectedItemOfType<RimFileWellPath>();
            bool             enableTargetPointPicking = !copyWellPathGeometry( sourceWellPath, newModeledWellPath );

            size_t modelledWellpathCount = wellPathCollection->modelledWellPathCount();

            newWellPaths.back()->setName( "Well-" + QString::number( modelledWellpathCount + 1 ) );
            newModeledWellPath->setWellPathColor( RiaColorTables::editableWellPathsPaletteColors().cycledColor3f( modelledWellpathCount ) );

            wellPathCollection->addWellPaths( newWellPaths );
            wellPathCollection->uiCapability()->updateConnectedEditors();

            newModeledWellPath->geometryDefinition()->enableTargetPointPicking( enableTargetPointPicking );

            project->scheduleCreateDisplayModelAndRedrawAllViews();

            Riu3DMainWindowTools::selectAsCurrentItem( newModeledWellPath->geometryDefinition() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEditableWellPathFeature::copyWellPathGeometry( RimFileWellPath* sourceWellPath, RimModeledWellPath* newModeledWellPath )
{
    if ( sourceWellPath && sourceWellPath->wellPathGeometry() && sourceWellPath->wellPathGeometry()->measuredDepths().size() > 2 )
    {
        auto destinationGeometryDef = newModeledWellPath->geometryDefinition();

        const int targetCount = 8;

        const auto wellPathPoints = sourceWellPath->wellPathGeometry()->wellPathPoints();
        const auto sourceMDs      = sourceWellPath->wellPathGeometry()->measuredDepths();

        auto findCaseContainingAnyPoint = []( const std::vector<cvf::Vec3d>& points ) -> RimCase*
        {
            std::vector<RimCase*> cases = RimProject::current()->allGridCases();
            for ( RimCase* c : cases )
            {
                for ( const cvf::Vec3d& p : points )
                {
                    if ( c->activeCellsBoundingBox().contains( p ) )
                    {
                        return c;
                    }
                }
            }
            return nullptr;
        };

        const RimCase* theCase = findCaseContainingAnyPoint( wellPathPoints );
        const double   depth   = theCase ? theCase->activeCellsBoundingBox().max().z() : 0.0;

        auto filterAwayPointsAboveMinimumDepth = []( const std::vector<cvf::Vec3d>& wellPathPoints,
                                                     const std::vector<double>&     md,
                                                     double minimumDepth ) -> std::pair<std::vector<cvf::Vec3d>, std::vector<double>>
        {
            // Loop from the top and exclude until the minimum depth has been reached
            std::vector<cvf::Vec3d> filteredPoints;
            std::vector<double>     filteredMds;

            bool minimumDepthReached = false;
            for ( size_t i = 0; i < wellPathPoints.size(); i++ )
            {
                if ( wellPathPoints[i].z() < minimumDepth || minimumDepthReached )
                {
                    filteredPoints.push_back( wellPathPoints[i] );
                    filteredMds.push_back( md[i] );
                    minimumDepthReached = true;
                }
            }

            return { filteredPoints, filteredMds };
        };

        const auto& [filteredWellPathPoints, filteredMDs] = filterAwayPointsAboveMinimumDepth( wellPathPoints, sourceMDs, depth );
        CAF_ASSERT( filteredWellPathPoints.size() == filteredMDs.size() );
        if ( filteredWellPathPoints.size() < 2 ) return false;

        std::vector<cvf::Vec3d> targetCoordinates;

        const double distanceBetweenTargets = ( filteredMDs.back() - filteredMDs.front() ) / targetCount;

        RigWellPath wellPath( filteredWellPathPoints, filteredMDs );

        for ( int targetIdx = 0; targetIdx <= targetCount; targetIdx++ )
        {
            double measuredDepth = filteredMDs.front() + targetIdx * distanceBetweenTargets;
            auto   sourceCoord   = wellPath.interpolatedPointAlongWellPath( measuredDepth );
            targetCoordinates.push_back( sourceCoord );
        }
        destinationGeometryDef->createAndInsertTargets( targetCoordinates );
        newModeledWellPath->createWellPathGeometry();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEditableWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Editable Well Path" );
    actionToSetup->setIcon( QIcon( ":/EditableWell.png" ) );
}
