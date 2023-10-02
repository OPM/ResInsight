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

#include "RicWellLogFileCloseFeature.h"

#include "RimViewWindow.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogPlot.h"
#include "RimWellPath.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiObjectEditorHandle.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicWellLogFileCloseFeature, "RicWellLogFileCloseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogFileCloseFeature::isCommandEnabled() const
{
    std::vector<RimWellLogLasFile*> objects = caf::selectedObjectsByType<RimWellLogLasFile*>();
    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogFileCloseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellLogLasFile*> objects = caf::selectedObjectsByType<RimWellLogLasFile*>();

    if ( objects.empty() ) return;

    for ( const auto& wellLogFile : objects )
    {
        RimWellPath* parentWellPath = wellLogFile->firstAncestorOrThisOfType<RimWellPath>();
        if ( parentWellPath )
        {
            std::set<RimViewWindow*> referringPlots = referringWellLogPlots( wellLogFile );
            parentWellPath->deleteWellLogFile( wellLogFile );

            for ( RimViewWindow* plot : referringPlots )
            {
                plot->loadDataAndUpdate();
            }

            parentWellPath->updateConnectedEditors();
        }
    }

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogFileCloseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Close Well Log File(s)" );
    actionToSetup->setIcon( QIcon( ":/Close.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimViewWindow*> RicWellLogFileCloseFeature::referringWellLogPlots( const RimWellLogLasFile* wellLogFile )
{
    // Remove all curves displaying data from the specified wellLogFile
    std::vector<caf::PdmObjectHandle*> referringObjects = wellLogFile->objectsWithReferringPtrFields();

    std::set<RimViewWindow*> plots;
    for ( const auto& obj : referringObjects )
    {
        RimWellAllocationPlot* allocationPlot = obj->firstAncestorOrThisOfType<RimWellAllocationPlot>();
        RimWellPltPlot*        pltPlot        = obj->firstAncestorOrThisOfType<RimWellPltPlot>();
        RimWellRftPlot*        rftPlot        = obj->firstAncestorOrThisOfType<RimWellRftPlot>();
        RimWellLogPlot*        wellLogPlot    = obj->firstAncestorOrThisOfType<RimWellLogPlot>();

        RimViewWindow* plot = allocationPlot ? dynamic_cast<RimViewWindow*>( allocationPlot )
                              : pltPlot      ? dynamic_cast<RimViewWindow*>( pltPlot )
                              : rftPlot      ? dynamic_cast<RimViewWindow*>( rftPlot )
                              : wellLogPlot  ? dynamic_cast<RimViewWindow*>( wellLogPlot )
                                             : nullptr;

        if ( plot != nullptr )
        {
            plots.insert( plot );
        }
    }
    return plots;
}
