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

#include "RicNewStimPlanModelTemplateFeature.h"

#include "RimCompletionTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelTemplateCollection.h"

#include "RicFractureNameGenerator.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStimPlanModelTemplateFeature, "RicNewStimPlanModelTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelTemplateFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( !oilfield ) return;

    RimStimPlanModelTemplateCollection* fracModColl =
        oilfield->completionTemplateCollection->stimPlanModelTemplateCollection();
    if ( !fracModColl ) return;

    RimStimPlanModelTemplate* stimPlanModelTemplate = new RimStimPlanModelTemplate;
    stimPlanModelTemplate->setName( RicFractureNameGenerator::nameForNewStimPlanModelTemplate() );

    fracModColl->addStimPlanModelTemplate( stimPlanModelTemplate );
    fracModColl->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( stimPlanModelTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New StimPlan Model Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanModelTemplateFeature::isCommandEnabled()
{
    return true;
}
