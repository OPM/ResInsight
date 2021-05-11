/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RicRunBasicFaultReactAssessmentFeature.h"

#include "RiaApplication.h"
#include "RiaEclipseFileNameTools.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaPreferencesGeoMech.h"
#include "RiaResultNames.h"

#include "RifFaultRAJsonWriter.h"
#include "RifFaultRAXmlWriter.h"

#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFaultRAPreprocSettings.h"
#include "RimFaultRASettings.h"
#include "RimGeoMechCase.h"
#include "RimProcess.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicRunBasicFaultReactAssessmentFeature, "RicRunBasicFaultReactAssessmentFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRunBasicFaultReactAssessmentFeature::isCommandEnabled()
{
    RimFaultInViewCollection* faultColl = faultCollection();
    if ( faultColl )
    {
        return ( faultColl->faultRAEnabled() );
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunBasicFaultReactAssessmentFeature::onActionTriggered( bool isChecked )
{
    RimFaultInViewCollection* coll = faultCollection();
    if ( coll == nullptr ) return;

    RimFaultRASettings* fraSettings = coll->faultRASettings();
    if ( fraSettings == nullptr ) return;

    int               faultID = selectedFaultID();
    caf::ProgressInfo runProgress( 3, "Running Basic Fault RA processing, please wait..." );

    {
        runProgress.setProgressDescription( "Macris calculate command." );
        QString paramfilename = fraSettings->basicParameterXMLFilename( faultID );

        RifFaultRAXmlWriter xmlwriter( fraSettings );
        QString             outErrorText;
        if ( !xmlwriter.writeCalculateFile( paramfilename, faultID, outErrorText ) )
        {
            QMessageBox::warning( nullptr,
                                  "Fault Reactivation Assessment Processing",
                                  "Unable to write parameter file! " + outErrorText );
            return;
        }

        addParameterFileForCleanUp( paramfilename );

        // run the java macris program in calculate mode
        QString     command    = RiaPreferencesGeoMech::current()->geomechFRAMacrisCommand();
        QStringList parameters = fraSettings->basicMacrisParameters( faultID );

        RimProcess process;
        process.setCommand( command );
        process.setParameters( parameters );
        process.execute();

        runProgress.incrementProgress();
    }

    runProgress.setProgressDescription( "Generating surface results." );

    runPostProcessing( faultID, fraSettings );

    runProgress.incrementProgress();

    runProgress.setProgressDescription( "Importing surface results." );

    // reload output surfaces
    reloadSurfaces( fraSettings );

    // delete parameter files
    cleanUpParameterFiles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunBasicFaultReactAssessmentFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Run Basic Processing" );
}
