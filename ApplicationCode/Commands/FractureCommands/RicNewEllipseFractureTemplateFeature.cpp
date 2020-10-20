/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
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

#include "RicNewEllipseFractureTemplateFeature.h"

#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewEllipseFractureTemplateFeature, "RicNewEllipseFractureTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::createNewTemplateForFractureAndUpdate( RimFracture* fracture )
{
    RimEllipseFractureTemplate* fractureTemplate = createNewTemplate();
    fracture->setFractureTemplate( fractureTemplate );
    selectFractureTemplateAndUpdate( fractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate )
{
    fractureTemplate->loadDataAndUpdate();

    RimFractureTemplateCollection* templateCollection = nullptr;
    fractureTemplate->firstAncestorOrThisOfTypeAsserted( templateCollection );
    templateCollection->updateConnectedEditors();

    RimProject* project = RimProject::current();

    project->scheduleCreateDisplayModelAndRedrawAllViews();

    Riu3DMainWindowTools::selectAsCurrentItem( fractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate* RicNewEllipseFractureTemplateFeature::createNewTemplate()
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( oilfield == nullptr ) return nullptr;

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();

    if ( fracDefColl )
    {
        return fracDefColl->addDefaultEllipseTemplate();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::onActionTriggered( bool isChecked )
{
    RimEllipseFractureTemplate* ellipseFractureTemplate = createNewTemplate();
    selectFractureTemplateAndUpdate( ellipseFractureTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewEllipseFractureTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New Ellipse Fracture Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewEllipseFractureTemplateFeature::isCommandEnabled()
{
    return true;
}
