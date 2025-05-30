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

#include "RicNewSimWellIntersectionFeature.h"

#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimSimWellInView.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSimWellIntersectionFeature, "RicNewSimWellIntersectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionFeature::onActionTriggered( bool isChecked )
{
    for ( auto simWell : caf::SelectionManager::instance()->objectsByType<RimSimWellInView>() )
    {
        RimEclipseView* eclView = simWell->firstAncestorOrThisOfTypeAsserted<RimEclipseView>();

        auto* cmd = new RicNewSimWellIntersectionCmd( eclView->intersectionCollection(), simWell );
        caf::CmdExecCommandManager::instance()->processExecuteCommand( cmd );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CrossSection16x16.png" ) );
    actionToSetup->setText( "New Intersection" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewSimWellIntersectionCmd::RicNewSimWellIntersectionCmd( RimIntersectionCollection* intersectionCollection, RimSimWellInView* simWell )
    : CmdExecuteCommand( nullptr )
    , m_intersectionCollection( intersectionCollection )
    , m_simWell( simWell )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewSimWellIntersectionCmd::~RicNewSimWellIntersectionCmd()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewSimWellIntersectionCmd::name()
{
    return "Create Intersection From Well";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionCmd::redo()
{
    CVF_ASSERT( m_intersectionCollection );
    CVF_ASSERT( m_simWell );

    auto* intersection = new RimExtrudedCurveIntersection();
    intersection->setName( m_simWell->name );
    intersection->configureForSimulationWell( m_simWell );

    m_intersectionCollection->appendIntersectionAndUpdate( intersection, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellIntersectionCmd::undo()
{
}
