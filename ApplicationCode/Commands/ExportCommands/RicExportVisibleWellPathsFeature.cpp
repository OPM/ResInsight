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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CompletionExportCommands/RicExportCompletionsForVisibleWellPathsFeature.h"
#include "RicExportSelectedWellPathsFeature.h"

#include "RigWellPath.h"

#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellPath.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <memory>

CAF_CMD_SOURCE_INIT( RicExportVisibleWellPathsFeature, "RicExportVisibleWellPathsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportVisibleWellPathsFeature::exportWellPath( const RimWellPath* wellPath, double mdStepSize, const QString& folder )
{
    auto   geom   = wellPath->wellPathGeometry();
    double currMd = geom->measureDepths().front() - mdStepSize;
    double endMd  = geom->measureDepths().back();

    auto        fileName = wellPath->name() + ".dev";
    auto        filePtr  = RicExportSelectedWellPathsFeature::openFileForExport( folder, fileName );
    QTextStream stream( filePtr.get() );
    stream.setRealNumberNotation( QTextStream::FixedNotation );

    stream << "WELLNAME: " << wellPath->name() << endl;

    while ( currMd < endMd )
    {
        currMd += mdStepSize;
        if ( currMd > endMd ) currMd = endMd;

        auto   pt  = geom->interpolatedPointAlongWellPath( currMd );
        double tvd = -pt.z();

        // Write to file
        stream << pt.x() << " " << pt.y() << " " << tvd << " " << currMd << endl;
    }

    filePtr->close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportVisibleWellPathsFeature::isCommandEnabled()
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

    RicExportSelectedWellPathsFeature::handleAction( wellPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportVisibleWellPathsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Visible Well Paths" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
