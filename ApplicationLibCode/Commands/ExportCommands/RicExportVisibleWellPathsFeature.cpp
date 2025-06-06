/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicExportVisibleWellPathsFeature.h"

#include "RiaLogging.h"

#include "CompletionExportCommands/RicExportCompletionsForVisibleWellPathsFeature.h"
#include "RicExportSelectedWellPathsFeature.h"

#include "Well/RigWellPath.h"

#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellPath.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileInfo>

#include <memory>

CAF_CMD_SOURCE_INIT( RicExportVisibleWellPathsFeature, "RicExportVisibleWellPathsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportVisibleWellPathsFeature::isCommandEnabled() const
{
    std::vector<RimWellPath*> selectedWellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();
    std::vector<RimWellPath*> visibleWellPaths  = RicExportCompletionsForVisibleWellPathsFeature::visibleWellPaths();

    return !selectedWellPaths.empty() && !visibleWellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportVisibleWellPathsFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths = RicExportCompletionsForVisibleWellPathsFeature::visibleWellPaths();

    RicExportSelectedWellPathsFeature::exportWellPathsToFile( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportVisibleWellPathsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Visible Well Paths" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
