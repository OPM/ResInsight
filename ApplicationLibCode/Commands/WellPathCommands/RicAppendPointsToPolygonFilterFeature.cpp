/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicAppendPointsToPolygonFilterFeature.h"

CAF_CMD_SOURCE_INIT( RicAppendPointsToPolygonFilterFeature, "RicAppendPointsToPolygonFilterFeature" );

#include "RimPolygonFilter.h"
#include "RimPolylineTarget.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QClipboard>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendPointsToPolygonFilterFeature::isCommandEnabled()
{
    caf::PdmObject* selectedObject = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selectedObject ) return false;

    RimPolygonFilter* polygonFilter = nullptr;
    selectedObject->firstAncestorOrThisOfType( polygonFilter );

    return ( polygonFilter != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendPointsToPolygonFilterFeature::onActionTriggered( bool isChecked )
{
    caf::PdmObject* selectedObject = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selectedObject ) return;

    RimPolygonFilter* polygonFilter = nullptr;
    selectedObject->firstAncestorOrThisOfType( polygonFilter );
    if ( !polygonFilter ) return;

    QStringList listOfThreeDoubles;

    QClipboard* clipboard = QApplication::clipboard();
    if ( clipboard )
    {
        QString content    = clipboard->text();
        listOfThreeDoubles = content.split( "\n", QString::SkipEmptyParts );
    }

    std::vector<cvf::Vec3d> points;
    caf::PdmValueFieldSpecialization<std::vector<cvf::Vec3d>>::setFromVariant( listOfThreeDoubles, points );

    for ( const auto& p : points )
    {
        auto newTarget = new RimPolylineTarget;
        newTarget->setAsPointTargetXYD( p );
        polygonFilter->insertTarget( nullptr, newTarget );
    }

    polygonFilter->updateEditorsAndVisualization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendPointsToPolygonFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Append Points from Clipboard" );

    RicPasteFeatureImpl::setIconAndShortcuts( actionToSetup );
}
