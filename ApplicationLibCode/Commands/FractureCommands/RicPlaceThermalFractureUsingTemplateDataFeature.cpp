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

    if ( !thermalTemplate->placeFractureUsingTemplateData( fracture ) ) return false;

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
