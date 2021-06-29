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

#include "RicLinkWellPathFeature.h"

#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "Riu3dSelectionManager.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicLinkWellPathFeature, "RicLinkWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkWellPathFeature::isCommandEnabled()
{
    return ( wellPathGeometryDef() != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkWellPathFeature::onActionTriggered( bool isChecked )
{
    if ( auto geoDef = wellPathGeometryDef() )
    {
        geoDef->enableLinkOfReferencePointUpdates( isChecked );
        geoDef->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicLinkWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    QString text = "Link Reference Point";
    actionToSetup->setText( text );
    actionToSetup->setCheckable( true );
    actionToSetup->setChecked( isCommandChecked() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicLinkWellPathFeature::isCommandChecked()
{
    if ( auto geoDef = wellPathGeometryDef() )
    {
        return geoDef->isReferencePointUpdatesLinked();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef* RicLinkWellPathFeature::wellPathGeometryDef()
{
    auto wellPathSelectionItem = RiuWellPathSelectionItem::wellPathSelectionItem();
    if ( wellPathSelectionItem && wellPathSelectionItem->m_wellpath )
    {
        auto modeledWellPath = dynamic_cast<RimModeledWellPath*>( wellPathSelectionItem->m_wellpath );
        return modeledWellPath->geometryDefinition();
    }
}
