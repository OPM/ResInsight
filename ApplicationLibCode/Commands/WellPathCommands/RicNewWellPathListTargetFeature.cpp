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
#include "RicNewWellPathListTargetFeature.h"

CAF_CMD_SOURCE_INIT( RicNewWellPathListTargetFeature, "RicNewWellPathListTargetFeature" );

#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "RiaOffshoreSphericalCoords.h"

#include "cafSelectionManager.h"
#include <QAction>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathListTargetFeature::isCommandEnabled() const
{
    {
        const auto attributes = caf::SelectionManager::instance()->objectsByType<RimWellPathGeometryDef>();
        if ( !attributes.empty() )
        {
            return false;
        }
    }
    {
        const auto selectedTargets = caf::SelectionManager::instance()->objectsByType<RimWellPathTarget>( caf::SelectionManager::FIRST_LEVEL );
        if ( !selectedTargets.empty() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathListTargetFeature::onActionTriggered( bool isChecked )
{
    const auto selectedTargets = caf::SelectionManager::instance()->objectsByType<RimWellPathTarget>( caf::SelectionManager::FIRST_LEVEL );
    if ( !selectedTargets.empty() )
    {
        RimWellPathTarget*      firstTarget = selectedTargets.front();
        RimWellPathGeometryDef* wellGeomDef = firstTarget->firstAncestorOrThisOfTypeAsserted<RimWellPathGeometryDef>();

        auto afterBeforePair = wellGeomDef->findActiveTargetsAroundInsertionPoint( firstTarget );

        cvf::Vec3d newPos = cvf::Vec3d::ZERO;

        bool isSeaLevelTarget = false;

        if ( !afterBeforePair.first && afterBeforePair.second )
        {
            if ( afterBeforePair.second->targetPointXYZ().z() == -wellGeomDef->anchorPointXyz().z() )
            {
                return; // We already have a target at sealevel.
            }

            cvf::Vec3d targetTangent = afterBeforePair.second->tangent();
            double     radius        = afterBeforePair.second->radius1();

            cvf::Vec3d tangentInHorizontalPlane = targetTangent;
            tangentInHorizontalPlane[2]         = 0.0;
            tangentInHorizontalPlane.normalize();

            RiaOffshoreSphericalCoords sphTangent( targetTangent );
            double                     inc                        = sphTangent.inc();
            double                     horizontalLengthFromTarget = radius - radius * cvf::Math::cos( inc );

            newPos     = afterBeforePair.second->targetPointXYZ() - horizontalLengthFromTarget * tangentInHorizontalPlane;
            newPos.z() = -wellGeomDef->anchorPointXyz().z();

            isSeaLevelTarget = true;
        }
        else if ( afterBeforePair.first && afterBeforePair.second )
        {
            newPos = 0.5 * ( afterBeforePair.first->targetPointXYZ() + afterBeforePair.second->targetPointXYZ() );
        }
        else if ( afterBeforePair.first && !afterBeforePair.second )
        {
            std::vector<RimWellPathTarget*> activeTargets = wellGeomDef->activeWellTargets();
            size_t                          targetCount   = activeTargets.size();
            if ( targetCount > 1 )
            {
                newPos                    = activeTargets[targetCount - 1]->targetPointXYZ();
                cvf::Vec3d nextLastToLast = newPos - activeTargets[targetCount - 2]->targetPointXYZ();
                newPos += 0.5 * nextLastToLast;
            }
            else
            {
                newPos = afterBeforePair.first->targetPointXYZ() + cvf::Vec3d( 0, 0, 200 );
            }
        }

        RimWellPathTarget* newTarget = new RimWellPathTarget;
        if ( isSeaLevelTarget )
        {
            newTarget->setAsPointXYZAndTangentTarget( { newPos[0], newPos[1], newPos[2] }, 0, 0 );
        }
        else
        {
            newTarget->setAsPointTargetXYD( { newPos[0], newPos[1], -newPos[2] } );
        }

        wellGeomDef->insertTarget( firstTarget, newTarget );
        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization( false );

        return;
    }

    const auto geomDefs = caf::SelectionManager::instance()->objectsByType<RimWellPathGeometryDef>( caf::SelectionManager::FIRST_LEVEL );
    if ( !geomDefs.empty() )
    {
        RimWellPathGeometryDef*         wellGeomDef   = geomDefs[0];
        std::vector<RimWellPathTarget*> activeTargets = wellGeomDef->activeWellTargets();

        size_t targetCount = activeTargets.size();

        if ( targetCount == 0 )
        {
            wellGeomDef->appendTarget();
        }
        else
        {
            cvf::Vec3d newPos = cvf::Vec3d::ZERO;

            if ( targetCount > 1 )
            {
                newPos                    = activeTargets[targetCount - 1]->targetPointXYZ();
                cvf::Vec3d nextLastToLast = newPos - activeTargets[targetCount - 2]->targetPointXYZ();
                newPos += 0.5 * nextLastToLast;
            }
            else if ( targetCount > 0 )
            {
                newPos = activeTargets[targetCount - 1]->targetPointXYZ() + cvf::Vec3d( 0, 0, 200 );
            }

            RimWellPathTarget* newTarget = new RimWellPathTarget;
            newTarget->setAsPointTargetXYD( { newPos[0], newPos[1], -newPos[2] } );
            wellGeomDef->insertTarget( nullptr, newTarget );
        }

        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathListTargetFeature::setupActionLook( QAction* actionToSetup )
{
    const auto selectedTargets = caf::SelectionManager::instance()->objectsByType<RimWellPathTarget>( caf::SelectionManager::FIRST_LEVEL );
    if ( !selectedTargets.empty() )
    {
        auto                    firstTarget = selectedTargets.front();
        RimWellPathGeometryDef* wellGeomDef = firstTarget->firstAncestorOrThisOfTypeAsserted<RimWellPathGeometryDef>();

        auto afterBeforePair = wellGeomDef->findActiveTargetsAroundInsertionPoint( firstTarget );

        if ( !afterBeforePair.first )
        {
            actionToSetup->setText( "Insert New Target At Sea Level" );
            actionToSetup->setIcon( QIcon( ":/WellTargets.png" ) );
            return;
        }
    }

    actionToSetup->setText( "Insert New Target Above" );
    actionToSetup->setIcon( QIcon( ":/WellTargets.png" ) );
}
