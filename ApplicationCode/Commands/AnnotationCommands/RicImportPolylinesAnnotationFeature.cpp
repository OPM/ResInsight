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

#include "RicImportPolylinesAnnotationFeature.h"

#include "RiaApplication.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimOilField.h"
#include "RimPolylinesFromFileAnnotation.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QFileDialog>

#include <cafSelectionManagerTools.h>

CAF_CMD_SOURCE_INIT( RicImportPolylinesAnnotationFeature, "RicImportPolylinesAnnotationFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportPolylinesAnnotationFeature::isCommandEnabled()
{
    auto selObjs      = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
    auto selGroupColl = caf::selectedObjectsByTypeStrict<RimAnnotationGroupCollection*>();

    return selObjs.size() == 1 ||
           ( selGroupColl.size() == 1 && selGroupColl.front()->uiCapability()->uiName() ==
                                             RimAnnotationGroupCollection::POLYLINE_FROM_FILE_ANNOTATION_UI_NAME );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPolylinesAnnotationFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );
    QStringList     fileNames  = QFileDialog::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                           "Import Poly Lines Annotation",
                                                           defaultDir,
                                                           "Text Files (*.txt);;Polylines (*.dat);;All Files (*.*)" );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileNames.last() ).absolutePath() );

    // Find or create the AnnotationsCollection

    RimProject*              proj      = RiaApplication::instance()->project();
    RimAnnotationCollection* annotColl = proj->activeOilField()->annotationCollection();

    if ( !annotColl )
    {
        annotColl                                    = new RimAnnotationCollection;
        proj->activeOilField()->annotationCollection = annotColl;
    }

    // For each file,

    RimPolylinesFromFileAnnotation* lastCreatedOrUpdated = annotColl->importOrUpdatePolylinesFromFile( fileNames );

    proj->updateConnectedEditors();

    if ( lastCreatedOrUpdated )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastCreatedOrUpdated );
    }

    annotColl->scheduleRedrawOfRelevantViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportPolylinesAnnotationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/PolylinesFromFile16x16.png" ) );
    actionToSetup->setText( "Import Poly Lines Annotation" );
}
