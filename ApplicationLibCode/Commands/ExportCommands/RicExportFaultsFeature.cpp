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

#include "RicExportFaultsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigMainGrid.h"

#include "RifEclipseInputFileTools.h"

#include "RimEclipseCase.h"
#include "RimFaultInView.h"

#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicExportFaultsFeature, "RicExportFaultsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportFaultsFeature::isCommandEnabled() const
{
    return ( !caf::SelectionManager::instance()->objectsByType<RimFaultInView>().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFaultsFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    const auto selectedFaults = caf::SelectionManager::instance()->objectsByType<RimFaultInView>();
    if ( selectedFaults.empty() ) return;

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "FAULTS" );

    QString selectedDir = RiuFileDialogTools::getExistingDirectory( nullptr, tr( "Select Directory" ), defaultDir );

    if ( selectedDir.isNull() )
    {
        // Stop if folder selection was cancelled.
        return;
    }

    for ( RimFaultInView* rimFault : selectedFaults )
    {
        RimEclipseCase* eclCase = rimFault->firstAncestorOrThisOfType<RimEclipseCase>();
        if ( eclCase )
        {
            QString caseName = eclCase->caseUserDescription();

            QString faultName = rimFault->name();
            if ( faultName == RiaResultNames::undefinedGridFaultName() ) faultName = "UNDEF";
            if ( faultName == RiaResultNames::undefinedGridFaultWithInactiveName() ) faultName = "UNDEF_IA";

            QString baseFilename = "Fault_" + faultName + "_" + caseName;
            baseFilename         = caf::Utils::makeValidFileBasename( baseFilename );

            QString completeFilename = selectedDir + "/" + baseFilename + ".grdecl";

            RifEclipseInputFileTools::saveFault( completeFilename,
                                                 eclCase->eclipseCaseData()->mainGrid(),
                                                 rimFault->faultGeometry()->faultFaces(),
                                                 faultName );
        }
    }

    // Remember the path to next time
    RiaApplication::instance()->setLastUsedDialogDirectory( "FAULTS", selectedDir );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFaultsFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Faults ..." );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}
