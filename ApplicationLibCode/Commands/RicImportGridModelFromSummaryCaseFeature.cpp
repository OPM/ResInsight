/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RicImportGridModelFromSummaryCaseFeature.h"

#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportGridModelFromSummaryCaseFeature, "RicImportGridModelFromSummaryCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGridModelFromSummaryCaseFeature::openOrImportGridModelFromSummaryCase( const RimFileSummaryCase* summaryCase )
{
    if ( !summaryCase ) return false;

    if ( findAndActivateFirstView( summaryCase ) ) return true;

    QString                 summaryFileName = summaryCase->summaryHeaderFilename();
    RiaEclipseFileNameTools fileHelper( summaryFileName );
    auto                    candidateGridFileName = fileHelper.findRelatedGridFile();

    if ( QFileInfo::exists( candidateGridFileName ) )
    {
        bool createView = true;
        auto id         = RiaImportEclipseCaseTools::openEclipseCaseFromFile( candidateGridFileName, createView );
        if ( id > -1 )
        {
            RiaLogging::info( QString( "Imported %1" ).arg( candidateGridFileName ) );

            return true;
        }
    }

    RiaLogging::info( QString( "No grid case found based on summary file %1" ).arg( summaryFileName ) );

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGridModelFromSummaryCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCaseFeature::onActionTriggered( bool isChecked )
{
    RimFileSummaryCase* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();

    openOrImportGridModelFromSummaryCase( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGridModelFromSummaryCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );

    RimFileSummaryCase* summaryCase = caf::SelectionManager::instance()->selectedItemOfType<RimFileSummaryCase>();

    QString summaryCaseName;
    if ( summaryCase ) summaryCaseName = summaryCase->caseName();

    QString txt;
    auto    gridCase = gridModelFromSummaryCase( summaryCase );
    if ( gridCase )
    {
        txt = "Open Grid Model View";
    }
    else
    {
        txt = "Import Grid Model";
    }

    if ( !summaryCaseName.isEmpty() )
    {
        txt += QString( " for '%1'" ).arg( summaryCaseName );
    }

    actionToSetup->setText( txt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGridModelFromSummaryCaseFeature::findAndActivateFirstView( const RimFileSummaryCase* summaryCase )
{
    auto gridCase = gridModelFromSummaryCase( summaryCase );
    if ( gridCase )
    {
        if ( !gridCase->gridViews().empty() )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( gridCase->gridViews().front() );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicImportGridModelFromSummaryCaseFeature::gridModelFromSummaryCase( const RimFileSummaryCase* summaryCase )
{
    if ( summaryCase )
    {
        QString                 summaryFileName = summaryCase->summaryHeaderFilename();
        RiaEclipseFileNameTools fileHelper( summaryFileName );
        auto                    candidateGridFileName = fileHelper.findRelatedGridFile();

        RimProject* project  = RimProject::current();
        auto        gridCase = project->eclipseCaseFromGridFileName( candidateGridFileName );

        return gridCase;
    }

    return nullptr;
}
