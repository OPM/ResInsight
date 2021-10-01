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

#include "RicAppendSeparateIntersectionResultFeature.h"

#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendSeparateIntersectionResultFeature, "RicAppendSeparateIntersectionResultFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendSeparateIntersectionResultFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSeparateIntersectionResultFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmObjectHandle*> collection;
    caf::SelectionManager::instance()->objectsByType( &collection );
    CVF_ASSERT( collection.size() == 1 );

    RimIntersectionResultsDefinitionCollection* intersectionResCollection = nullptr;
    collection[0]->firstAncestorOrThisOfType( intersectionResCollection );

    CVF_ASSERT( intersectionResCollection );

    RicAppendSeparateIntersectionResultFeatureCmd* cmd =
        new RicAppendSeparateIntersectionResultFeatureCmd( intersectionResCollection );
    caf::CmdExecCommandManager::instance()->processExecuteCommand( cmd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSeparateIntersectionResultFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Separate Result" );
    actionToSetup->setIcon( QIcon( ":/CellResult.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicAppendSeparateIntersectionResultFeatureCmd::RicAppendSeparateIntersectionResultFeatureCmd(
    RimIntersectionResultsDefinitionCollection* intersectionCollection )
    : CmdExecuteCommand( nullptr )
    , m_intersectionCollection( intersectionCollection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicAppendSeparateIntersectionResultFeatureCmd::~RicAppendSeparateIntersectionResultFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicAppendSeparateIntersectionResultFeatureCmd::name()
{
    return "New Intersection";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSeparateIntersectionResultFeatureCmd::redo()
{
    CVF_ASSERT( m_intersectionCollection );

    RimIntersectionResultDefinition* intersectionResDef = new RimIntersectionResultDefinition();
    m_intersectionCollection->appendIntersectionResultDefinition( intersectionResDef );

    m_intersectionCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendSeparateIntersectionResultFeatureCmd::undo()
{
}
