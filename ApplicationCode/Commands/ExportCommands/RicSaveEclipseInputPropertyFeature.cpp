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

#include "RicSaveEclipseInputPropertyFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicEclipseCellResultToFileImpl.h"
#include "RicExportFeatureImpl.h"

#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimExportInputPropertySettings.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicSaveEclipseInputPropertyFeature, "RicSaveEclipseInputPropertyFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseInputPropertyFeature::isCommandEnabled()
{
    return selectedInputProperty() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputPropertyFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RimEclipseInputProperty* inputProperty = selectedInputProperty();
    if ( !inputProperty ) return;

    {
        bool isResolved = false;
        if ( inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED ||
             inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED_NOT_SAVED )
        {
            isResolved = true;
        }

        if ( !isResolved )
        {
            QMessageBox::warning( Riu3DMainWindowTools::mainWindowWidget(),
                                  "Export failure",
                                  "Property is not resolved, and then it is not possible to export the property." );

            return;
        }
    }

    RimExportInputSettings exportSettings;
    exportSettings.eclipseKeyword = inputProperty->eclipseKeyword;

    // Find input reservoir for this property
    RimEclipseCase* eclipseCase = nullptr;
    {
        RimEclipseInputPropertyCollection* inputPropertyCollection =
            dynamic_cast<RimEclipseInputPropertyCollection*>( inputProperty->parentField()->ownerObject() );
        if ( !inputPropertyCollection ) return;

        eclipseCase = dynamic_cast<RimEclipseCase*>( inputPropertyCollection->parentField()->ownerObject() );
    }

    if ( !eclipseCase ) return;

    {
        RiaApplication* app           = RiaApplication::instance();
        QString         projectFolder = app->currentProjectPath();
        if ( projectFolder.isEmpty() )
        {
            projectFolder = eclipseCase->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + inputProperty->eclipseKeyword;

        exportSettings.fileName = outputFileName;
    }

    caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                                 &exportSettings,
                                                 "Export Eclipse Property to Text File",
                                                 "" );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        const double undefinedValue = 0.0;

        QString errorMsg;
        bool    isOk = RicEclipseCellResultToFileImpl::writePropertyToTextFile( exportSettings.fileName,
                                                                             eclipseCase->eclipseCaseData(),
                                                                             0,
                                                                             inputProperty->resultName,
                                                                             exportSettings.eclipseKeyword,
                                                                             undefinedValue,
                                                                             &errorMsg );
        if ( isOk )
        {
            inputProperty->fileName       = exportSettings.fileName();
            inputProperty->eclipseKeyword = exportSettings.eclipseKeyword;
            inputProperty->resolvedState  = RimEclipseInputProperty::RESOLVED;

            inputProperty->updateConnectedEditors();
        }
        else
        {
            RiaLogging::error( errorMsg );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputPropertyFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Property To File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputProperty* RicSaveEclipseInputPropertyFeature::selectedInputProperty() const
{
    std::vector<RimEclipseInputProperty*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    return selection.size() > 0 ? selection[0] : nullptr;
}
