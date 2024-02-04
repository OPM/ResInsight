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

#include "RiaTextStringTools.h"

#include "RimPolygonFilter.h"
#include "RimPolylineTarget.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>

CAF_CMD_SOURCE_INIT( RicAppendPointsToPolygonFilterFeature, "RicAppendPointsToPolygonFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendPointsToPolygonFilterFeature::isCommandEnabled() const
{
    auto obj = caf::firstAncestorOfTypeFromSelectedObject<RimPolygonFilter>();
    return obj != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendPointsToPolygonFilterFeature::onActionTriggered( bool isChecked )
{
    auto polygonFilter = caf::firstAncestorOfTypeFromSelectedObject<RimPolygonFilter>();
    if ( !polygonFilter ) return;

    QStringList listOfThreeDoubles;

    QClipboard* clipboard = QApplication::clipboard();
    if ( clipboard )
    {
        QString content    = clipboard->text();
        listOfThreeDoubles = RiaTextStringTools::splitSkipEmptyParts( content, "\n" );
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
