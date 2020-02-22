/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewValveTemplateFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimValveTemplateCollection.h"
#include "RimWellPathValve.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewValveTemplateFeature, "RicNewValveTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveTemplateFeature::selectValveTemplateAndUpdate( RimValveTemplate* valveTemplate )
{
    valveTemplate->loadDataAndUpdate();

    RimValveTemplateCollection* templateCollection = nullptr;
    valveTemplate->firstAncestorOrThisOfType( templateCollection );
    if ( templateCollection )
    {
        templateCollection->updateConnectedEditors();
    }

    RimProject* project = RiaApplication::instance()->project();

    std::vector<Rim3dView*> views;
    project->allVisibleViews( views );

    for ( Rim3dView* view : views )
    {
        if ( dynamic_cast<RimEclipseView*>( view ) )
        {
            view->updateConnectedEditors();
        }
    }

    Riu3DMainWindowTools::selectAsCurrentItem( valveTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveTemplateFeature::createNewValveTemplateForValveAndUpdate( RimWellPathValve* valve )
{
    RimValveTemplate* valveTemplate = createNewValveTemplate();
    valve->setValveTemplate( valveTemplate );
    selectValveTemplateAndUpdate( valveTemplate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate* RicNewValveTemplateFeature::createNewValveTemplate()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( oilfield == nullptr ) return nullptr;

    RimValveTemplateCollection* valveTemplateColl = oilfield->valveTemplateCollection();

    if ( valveTemplateColl )
    {
        RimValveTemplate* valveTemplate = new RimValveTemplate();
        QString userLabel = QString( "Valve Template #%1" ).arg( valveTemplateColl->valveTemplates().size() + 1 );
        valveTemplate->setUserLabel( userLabel );
        valveTemplateColl->addValveTemplate( valveTemplate );
        valveTemplate->setUnitSystem( valveTemplateColl->defaultUnitSystemType() );
        valveTemplate->setDefaultValuesFromUnits();
        return valveTemplate;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveTemplateFeature::onActionTriggered( bool isChecked )
{
    RimValveTemplate* valveTemplate = createNewValveTemplate();
    if ( valveTemplate )
    {
        selectValveTemplateAndUpdate( valveTemplate );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewValveTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ICDValve16x16.png" ) );
    actionToSetup->setText( "New Valve Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewValveTemplateFeature::isCommandEnabled()
{
    return true;
}
