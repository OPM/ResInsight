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

#include "RicCreateTextAnnotationFeature.h"

#include "RiaApplication.h"

#include "RimAnnotationCollection.h"
#include "RimAnnotationGroupCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimOilField.h"
#include "RimPolylinesAnnotation.h"
#include "RimProject.h"
#include "RimReachCircleAnnotation.h"
#include "RimTextAnnotation.h"

#include "RiuMainWindow.h"

#include <cafSelectionManagerTools.h>

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateTextAnnotationFeature, "RicCreateTextAnnotationFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTextAnnotationFeature::isCommandEnabled()
{
    auto selObjsGlobal  = caf::selectedObjectsByTypeStrict<RimAnnotationCollection*>();
    auto selObjs2InView = caf::selectedObjectsByTypeStrict<RimAnnotationInViewCollection*>();
    auto selGroupColl   = caf::selectedObjectsByTypeStrict<RimAnnotationGroupCollection*>();

    return selObjsGlobal.size() == 1 || selObjs2InView.size() == 1 ||
           ( selGroupColl.size() == 1 &&
             selGroupColl.front()->uiCapability()->uiName() == RimAnnotationGroupCollection::TEXT_ANNOTATION_UI_NAME );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationFeature::onActionTriggered( bool isChecked )

{
    auto coll = annotationCollectionBase();
    if ( coll )
    {
        auto newAnnotation = new RimTextAnnotation();
        newAnnotation->enablePicking( true );
        coll->addAnnotation( newAnnotation );
        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( newAnnotation );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTextAnnotationFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/TextAnnotation16x16.png" ) );
    actionToSetup->setText( "Create Text Annotation" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase* RicCreateTextAnnotationFeature::annotationCollectionBase() const
{
    auto selColls = caf::selectedObjectsByTypeStrict<RimAnnotationCollectionBase*>();
    if ( selColls.size() == 1 ) return selColls.front();

    RimAnnotationCollectionBase* coll         = nullptr;
    auto                         selGroupColl = caf::selectedObjectsByTypeStrict<RimAnnotationGroupCollection*>();
    if ( selGroupColl.size() == 1 )
    {
        selGroupColl.front()->firstAncestorOrThisOfType( coll );
    }

    return coll;
}
