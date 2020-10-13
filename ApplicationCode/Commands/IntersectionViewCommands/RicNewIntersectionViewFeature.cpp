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

#include "RicNewIntersectionViewFeature.h"

#include "RiaLogging.h"
#include "Rim2dIntersectionView.h"
#include "RimCase.h"
#include "RimExtrudedCurveIntersection.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RiuViewer.h"

#include "cafSelectionManagerTools.h"

#include "cvfCamera.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewIntersectionViewFeature, "RicNewIntersectionViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewIntersectionViewFeature::isCommandEnabled()
{
    std::set<RimExtrudedCurveIntersection*> objects = selectedIntersections();

    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionViewFeature::onActionTriggered( bool isChecked )
{
    std::set<RimExtrudedCurveIntersection*> intersections = selectedIntersections();

    Rim2dIntersectionView* objectToSelect = nullptr;

    for ( auto intersection : intersections )
    {
        if ( !intersection ) continue;

        RimCase* rimCase = nullptr;
        intersection->firstAncestorOrThisOfType( rimCase );
        if ( rimCase )
        {
            if ( intersection->direction() != RimExtrudedCurveIntersection::CS_VERTICAL )
            {
                QString text = QString( "The intersection view only supports vertical intersections.\n"
                                        "The intersection '%1' is not vertical but a converted version will be shown "
                                        "in the view ." )
                                   .arg( intersection->name() );

                RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "New Intersection View", text );
            }

            Rim2dIntersectionView* intersectionView = intersection->correspondingIntersectionView();
            intersectionView->setVisible( true );
            intersectionView->loadDataAndUpdate();

            intersectionView->updateConnectedEditors();

            objectToSelect = intersectionView;
        }
    }

    if ( objectToSelect )
    {
        // RiuMainWindow::instance()->selectAsCurrentItem(objectToSelect);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show 2D Intersection View" );
    // actionToSetup->setIcon(QIcon(":/chain.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimExtrudedCurveIntersection*> RicNewIntersectionViewFeature::selectedIntersections()
{
    std::set<RimExtrudedCurveIntersection*> objects;

    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>( selItem );
    if ( generalSelectionItem )
    {
        RimExtrudedCurveIntersection* intersection =
            dynamic_cast<RimExtrudedCurveIntersection*>( generalSelectionItem->m_object );
        if ( intersection )
        {
            objects.insert( intersection );

            // Return only the intersection the user clicked on

            return objects;
        }
    }

    {
        std::vector<RimExtrudedCurveIntersection*> selectedObjects =
            caf::selectedObjectsByType<RimExtrudedCurveIntersection*>();
        for ( auto obj : selectedObjects )
        {
            objects.insert( obj );
        }
    }

    return objects;
}
