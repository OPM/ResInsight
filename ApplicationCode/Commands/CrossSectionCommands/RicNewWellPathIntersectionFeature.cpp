/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewWellPathIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimWellPath.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellPathIntersectionFeature, "RicNewWellPathIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeature::RicNewWellPathIntersectionFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellPathIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeature::onActionTriggered( bool isChecked )
{
    RimGridView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !activeView ) return;

    std::vector<RimWellPath*> collection;
    caf::SelectionManager::instance()->objectsByType( &collection );
    CVF_ASSERT( collection.size() == 1 );

    RimWellPath* wellPath = collection[0];

    RicNewWellPathIntersectionFeatureCmd* cmd =
        new RicNewWellPathIntersectionFeatureCmd( activeView->intersectionCollection(), wellPath );
    caf::CmdExecCommandManager::instance()->processExecuteCommand( cmd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CrossSection16x16.png" ) );
    actionToSetup->setText( "Create Intersection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeatureCmd::RicNewWellPathIntersectionFeatureCmd( RimIntersectionCollection* intersectionCollection,
                                                                            RimWellPath*               wellPath )
    : CmdExecuteCommand( nullptr )
    , m_intersectionCollection( intersectionCollection )
    , m_wellPath( wellPath )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewWellPathIntersectionFeatureCmd::~RicNewWellPathIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewWellPathIntersectionFeatureCmd::name()
{
    return "Create Intersection From Well Path";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeatureCmd::redo()
{
    CVF_ASSERT( m_intersectionCollection );
    CVF_ASSERT( m_wellPath );

    RimExtrudedCurveIntersection* intersection = new RimExtrudedCurveIntersection();
    intersection->setName( m_wellPath->name() );
    intersection->type     = RimExtrudedCurveIntersection::CS_WELL_PATH;
    intersection->wellPath = m_wellPath;

    m_intersectionCollection->appendIntersectionAndUpdate( intersection, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellPathIntersectionFeatureCmd::undo()
{
}
