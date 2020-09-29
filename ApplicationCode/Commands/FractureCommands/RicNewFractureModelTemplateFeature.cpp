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

#include "RicNewFractureModelTemplateFeature.h"

#include "RimCompletionTemplateCollection.h"
#include "RimFractureModelTemplate.h"
#include "RimFractureModelTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RicFractureNameGenerator.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewFractureModelTemplateFeature, "RicNewFractureModelTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelTemplateFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    RimFractureModelTemplateCollection* fracModColl =
        oilfield->completionTemplateCollection->fractureModelTemplateCollection();
    if ( !fracModColl ) return;

    RimFractureModelTemplate* fractureModelTemplate = new RimFractureModelTemplate;
    fractureModelTemplate->setName( RicFractureNameGenerator::nameForNewFractureModelTemplate() );

    fracModColl->addFractureModelTemplate( fractureModelTemplate );
    fracModColl->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( fractureModelTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New Fracture Model Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFractureModelTemplateFeature::isCommandEnabled()
{
    return true;
}
