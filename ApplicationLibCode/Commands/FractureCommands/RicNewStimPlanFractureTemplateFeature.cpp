/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2018 Statoil ASA
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

#include "RicNewStimPlanFractureTemplateFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QFileInfo>
#include <vector>

CAF_CMD_SOURCE_INIT( RicNewStimPlanFractureTemplateFeature, "RicNewStimPlanFractureTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::createNewTemplateForFractureAndUpdate( RimFracture* fracture )
{
    std::vector<RimStimPlanFractureTemplate*> newTemplates = createNewTemplates();
    if ( !newTemplates.empty() )
    {
        RimStimPlanFractureTemplate* lastTemplateCreated = newTemplates.back();
        fracture->setFractureTemplate( lastTemplateCreated );

        selectFractureTemplateAndUpdate( lastTemplateCreated );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::selectFractureTemplateAndUpdate( RimFractureTemplate* fractureTemplate )
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
std::vector<RimStimPlanFractureTemplate*> RicNewStimPlanFractureTemplateFeature::createNewTemplates()
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectoryWithFallbackToProjectFolder( "STIMPLAN_XML_DIR" );
    QStringList     fileNames  = RiuFileDialogTools::getOpenFileNames( nullptr,
                                                                  "Open StimPlan XML File",
                                                                  defaultDir,
                                                                  "StimPlan XML File (*.xml);;All files(*.*)" );

    auto templates = createNewTemplatesFromFiles( fileNames.toVector().toStdVector() );

    if ( !fileNames.isEmpty() )
    {
        app->setLastUsedDialogDirectory( "STIMPLAN_XML_DIR", QFileInfo( fileNames.last() ).absolutePath() );
    }

    return templates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimStimPlanFractureTemplate*>
    RicNewStimPlanFractureTemplateFeature::createNewTemplatesFromFiles( const std::vector<QString>& fileNames )
{
    if ( fileNames.empty() ) return std::vector<RimStimPlanFractureTemplate*>();

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimOilField* oilfield = project->activeOilField();
    if ( oilfield == nullptr ) return std::vector<RimStimPlanFractureTemplate*>();

    RimFractureTemplateCollection* fracDefColl = oilfield->fractureDefinitionCollection();
    if ( !fracDefColl ) return std::vector<RimStimPlanFractureTemplate*>();

    std::vector<RimStimPlanFractureTemplate*> newFractures;
    for ( auto fileName : fileNames )
    {
        if ( fileName.isEmpty() ) continue;

        RimStimPlanFractureTemplate* fractureDef = new RimStimPlanFractureTemplate();
        fracDefColl->addFractureTemplate( fractureDef );

        QFileInfo stimplanfileFileInfo( fileName );
        QString   name = stimplanfileFileInfo.baseName();
        if ( name.isEmpty() )
        {
            name = "StimPlan Fracture Template";
        }

        fractureDef->setName( name );

        fractureDef->setFileName( fileName );
        fractureDef->loadDataAndUpdate();
        fractureDef->setDefaultsBasedOnXMLfile();
        fractureDef->setDefaultWellDiameterFromUnit();
        newFractures.push_back( fractureDef );
    }

    return newFractures;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimStimPlanFractureTemplate*> newFractures = createNewTemplates();
    if ( !newFractures.empty() )
    {
        selectFractureTemplateAndUpdate( newFractures.back() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanFractureTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "New StimPlan Fracture Template" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanFractureTemplateFeature::isCommandEnabled()
{
    return true;
}
