/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RicPlaceThermalFractureUsingTemplateDataFeature.h"

#include "RiaLogging.h"

#include "RigWellPath.h"
#include "RimProject.h"
#include "RimThermalFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <cmath>

CAF_CMD_SOURCE_INIT( RicPlaceThermalFractureUsingTemplateDataFeature, "RicPlaceThermalFractureUsingTemplateDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPlaceThermalFractureUsingTemplateDataFeature::onActionTriggered( bool isChecked )
{
    RimWellPathFracture* fracture = RicPlaceThermalFractureUsingTemplateDataFeature::selectedThermalFracture();
    if ( !fracture ) return;

    if ( !fracture->fractureTemplate() ) return;

    placeUsingTemplateData( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPlaceThermalFractureUsingTemplateDataFeature::placeUsingTemplateData( RimWellPathFracture* fracture )
{
    RimThermalFractureTemplate* thermalTemplate = dynamic_cast<RimThermalFractureTemplate*>( fracture->fractureTemplate() );
    if ( !thermalTemplate ) return false;

    RimWellPath* wellPath = nullptr;
    fracture->firstAncestorOrThisOfTypeAsserted( wellPath );

    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return false;

    auto [centerPosition, rotation] = thermalTemplate->computePositionAndRotation();

    // TODO: y conversion is workaround for strange test data
    centerPosition.y() = std::fabs( centerPosition.y() );
    centerPosition.z() *= -1.0;

    double md = wellPathGeometry->closestMeasuredDepth( centerPosition );

    RiaLogging::info( QString( "Placing thermal fracture. Posotion: [%1 %2 %3]" )
                          .arg( centerPosition.x() )
                          .arg( centerPosition.y() )
                          .arg( centerPosition.z() ) );
    RiaLogging::info( QString( "Computed MD: %1" ).arg( md ) );

    fracture->setMeasuredDepth( md );

    fracture->setAzimuth( rotation.x() );
    fracture->setDip( rotation.y() );
    fracture->setTilt( rotation.z() );

    fracture->updateConnectedEditors();
    RimProject* project = RimProject::current();
    project->reloadCompletionTypeResultsInAllViews();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPlaceThermalFractureUsingTemplateDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Place Using Template Data" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPlaceThermalFractureUsingTemplateDataFeature::isCommandEnabled()
{
    return selectedThermalFracture() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFracture* RicPlaceThermalFractureUsingTemplateDataFeature::selectedThermalFracture()
{
    auto fracture = caf::SelectionManager::instance()->selectedItemOfType<RimWellPathFracture>();
    if ( !fracture->fractureTemplate() ) return nullptr;

    RimThermalFractureTemplate* thermalTemplate = dynamic_cast<RimThermalFractureTemplate*>( fracture->fractureTemplate() );
    if ( !thermalTemplate ) return nullptr;

    return fracture;
}
