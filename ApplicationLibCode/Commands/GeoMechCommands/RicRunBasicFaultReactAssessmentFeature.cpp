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
#include "RiaPreferences.h"
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
    caf::ProgressInfo runProgress( 2, "Running Basic Fault RA processing, please wait..." );

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

        // run the java macris program in prepare mode
        QString     command    = RiaPreferences::current()->geomechFRAMacrisCommand();
        QStringList parameters = fraSettings->basicMacrisParameters( faultID );

        RimProcess process;
        process.setCommand( command );
        process.setParameters( parameters );
        process.execute();

        runProgress.incrementProgress();
    }
    {
        runProgress.setProgressDescription( "Generating surface results." );

        QString outErrorText;
        if ( !RifFaultRAJSonWriter::writeToPostprocFile( faultID, fraSettings, outErrorText ) )
        {
            QMessageBox::warning( nullptr,
                                  "Fault Reactivation Assessment Processing",
                                  "Unable to write postproc parameter file! " + outErrorText );
            return;
        }

        QString     command    = RiaPreferences::current()->geomechFRAPostprocCommand();
        QStringList parameters = fraSettings->postprocParameters( faultID );

        RimProcess process;
        process.setCommand( command );
        process.setParameters( parameters );
        process.execute();

        runProgress.incrementProgress();
    }
    {
        runProgress.setProgressDescription( "Importing surface results." );
    }
    // todo - delete parameter files!
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRunBasicFaultReactAssessmentFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/fault_react_24x24.png" ) );
    actionToSetup->setText( "Run Basic Processing" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultInViewCollection* RicRunBasicFaultReactAssessmentFeature::faultCollection()
{
    RimFaultInViewCollection* faultColl = nullptr;

    RimFaultInView* selObj = dynamic_cast<RimFaultInView*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        if ( !selObj->name().startsWith( RiaResultNames::faultReactAssessmentPrefix() ) ) return nullptr;
        selObj->firstAncestorOrThisOfType( faultColl );
    }

    return faultColl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicRunBasicFaultReactAssessmentFeature::selectedFaultID()
{
    int             retval = -1;
    RimFaultInView* selObj = dynamic_cast<RimFaultInView*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        QString lookFor = RiaResultNames::faultReactAssessmentPrefix();
        QString name    = selObj->name();
        if ( !name.startsWith( lookFor ) ) return retval;

        name = name.mid( lookFor.length() );
        if ( name.size() == 0 ) return retval;

        bool bOK;
        retval = name.toInt( &bOK );
        if ( !bOK ) retval = -1;
    }

    return retval;
}
