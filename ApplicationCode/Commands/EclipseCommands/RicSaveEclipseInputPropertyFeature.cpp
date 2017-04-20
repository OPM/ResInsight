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

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimExportInputPropertySettings.h"
#include "RimEclipseInputCase.h"

#include "RifEclipseInputFileTools.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QMessageBox>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicSaveEclipseInputPropertyFeature, "RicSaveEclipseInputPropertyFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseInputPropertyFeature::isCommandEnabled()
{
    return selectedInputProperty() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputPropertyFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    RimEclipseInputProperty* inputProperty = selectedInputProperty();
    if (!inputProperty) return;

    {
        bool isResolved = false;
        if (inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED || inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED_NOT_SAVED)
        {
            isResolved = true;
        }

        if (!isResolved)
        {
            QMessageBox::warning(RiuMainWindow::instance(), "Export failure", "Property is not resolved, and then it is not possible to export the property.");

            return;
        }
    }

    RimExportInputSettings exportSettings;
    exportSettings.eclipseKeyword = inputProperty->eclipseKeyword;

    // Find input reservoir for this property
    RimEclipseInputCase* inputReservoir = NULL;
    {
        RimEclipseInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimEclipseInputPropertyCollection*>(inputProperty->parentField()->ownerObject());
        if (!inputPropertyCollection) return;

        inputReservoir = dynamic_cast<RimEclipseInputCase*>(inputPropertyCollection->parentField()->ownerObject());
    }

    if (!inputReservoir) return;

    {
        RiaApplication* app = RiaApplication::instance();
        QString projectFolder = app->currentProjectPath();
        if (projectFolder.isEmpty())
        {   
            projectFolder = inputReservoir->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + inputProperty->eclipseKeyword;

        exportSettings.fileName = outputFileName;
    }

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Eclipse Property to Text File", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        bool isOk = RifEclipseInputFileTools::writePropertyToTextFile(exportSettings.fileName, inputReservoir->eclipseCaseData(), 0, inputProperty->resultName, exportSettings.eclipseKeyword);
        if (isOk)
        {
            inputProperty->fileName = exportSettings.fileName;
            inputProperty->eclipseKeyword = exportSettings.eclipseKeyword;
            inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;

            inputProperty->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputPropertyFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Property To File");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputProperty* RicSaveEclipseInputPropertyFeature::selectedInputProperty() const
{
    std::vector<RimEclipseInputProperty*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection.size() > 0 ? selection[0] : NULL;
}


