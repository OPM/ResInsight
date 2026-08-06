/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicNewIjkIntersection3dviewFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimIjkIntersection.h"
#include "RimIntersectionCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewIjkIntersection3dviewFeature, "RicNewIjkIntersection3dviewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewIjkIntersection3dviewFeature::isCommandEnabled() const
{
    // I/J/K intersections rely on a structured Eclipse grid
    return dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeMainOrComparisonGridView() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIjkIntersection3dviewFeature::onActionTriggered( bool isChecked )
{
    QVariant userData = this->userData();
    if ( userData.isNull() || userData.type() != QVariant::List ) return;

    // Axis and 0-based fixed index computed from the picked cell
    QVariantList list = userData.toList();
    CAF_ASSERT( list.size() == 2 );

    auto axis       = static_cast<RimIjkIntersection::GridAxis>( list[0].toInt() );
    int  fixedIndex = list[1].toInt();

    auto* eclipseView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeMainOrComparisonGridView() );
    if ( !eclipseView ) return;

    RimIntersectionCollection* coll = eclipseView->intersectionCollection();
    if ( !coll ) return;

    RimIjkIntersection* intersection = new RimIjkIntersection();
    intersection->setName( "Intersection I/J/K" );

    // The default values are computed from the grid, which is resolved through the parent
    // view, so the intersection must be added to the collection first
    coll->appendIjkIntersectionNoUpdate( intersection );
    intersection->setToDefaultValues();
    intersection->setAxis( axis );
    intersection->setFixedIndex( fixedIndex );

    coll->updateConnectedEditors();
    Riu3DMainWindowTools::selectAsCurrentItem( intersection );

    eclipseView->scheduleCreateDisplayModelAndRedraw();
    eclipseView->showGridCells( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIjkIntersection3dviewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
}
