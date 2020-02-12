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

#include "RicAppendIntersectionFeature.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimTensorResults.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicAppendIntersectionFeature, "RicAppendIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicAppendIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmObjectHandle*> collection;
    caf::SelectionManager::instance()->objectsByType( &collection );
    CVF_ASSERT( collection.size() == 1 );

    RimIntersectionCollection* intersectionCollection = nullptr;
    collection[0]->firstAncestorOrThisOfType( intersectionCollection );

    CVF_ASSERT( intersectionCollection );

    RicAppendIntersectionFeatureCmd* cmd = new RicAppendIntersectionFeatureCmd( intersectionCollection );
    caf::CmdExecCommandManager::instance()->processExecuteCommand( cmd );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CrossSection16x16.png" ) );
    actionToSetup->setText( "New Intersection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicAppendIntersectionFeatureCmd::RicAppendIntersectionFeatureCmd( RimIntersectionCollection* intersectionCollection )
    : CmdExecuteCommand( nullptr )
    , m_intersectionCollection( intersectionCollection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicAppendIntersectionFeatureCmd::~RicAppendIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicAppendIntersectionFeatureCmd::name()
{
    return "New Intersection";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeatureCmd::redo()
{
    CVF_ASSERT( m_intersectionCollection );

    RimExtrudedCurveIntersection* intersection = new RimExtrudedCurveIntersection();
    intersection->setName( "Intersection" );
    m_intersectionCollection->appendIntersectionAndUpdate( intersection );

    RimGridView* view = nullptr;
    m_intersectionCollection->firstAncestorOrThisOfTypeAsserted( view );

    RimGeoMechView* geoMechView = nullptr;
    geoMechView                 = dynamic_cast<RimGeoMechView*>( view );
    if ( geoMechView )
    {
        geoMechView->tensorResults()->setShowTensors( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicAppendIntersectionFeatureCmd::undo()
{
}
