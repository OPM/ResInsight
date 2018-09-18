/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicHoloLensExportToFolderFeature.h"
#include "RicHoloLensExportImpl.h"
#include "RicHoloLensExportToFolderUi.h"

#include "RiaApplication.h"
#include "RimCase.h"
#include "RimDialogData.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "cafPdmUiPropertyViewDialog.h"

#include "cvfCollection.h"
#include "cvfPart.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

CAF_CMD_SOURCE_INIT(RicHoloLensExportToFolderFeature, "RicHoloLensExportToFolderFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportToFolderFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToFolderFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    RicHoloLensExportToFolderUi* featureUi = RiaApplication::instance()->project()->dialogData()->holoLensExportToFolderData();
    featureUi->setViewForExport(activeView);

    caf::PdmUiPropertyViewDialog propertyDialog(
        nullptr, featureUi, "HoloLens - Export Data Folder", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(400, 330));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        cvf::Collection<cvf::Part> allPartsColl;

        RimGridView* viewForExport = featureUi->viewForExport();
        if (!viewForExport) return;

        QString caseName("Unnamed Case");
        {
            RimCase* rimCase = nullptr;
            viewForExport->firstAncestorOrThisOfType(rimCase);
            if (rimCase)
            {
                caseName = rimCase->caseUserDescription();
            }
        }
        RicHoloLensExportImpl::partsForExport(viewForExport, &allPartsColl);

        QDir dir(featureUi->exportFolder());

        for (size_t i = 0; i < allPartsColl.size(); i++)
        {
            cvf::Part* part = allPartsColl.at(i);

            if (part)
            {
                QString nameOfObject = RicHoloLensExportImpl::nameFromPart(part);
                // caseName is relevant to combine with name of object

                // bool isGrid = RicHoloLensExportImpl::isGrid(part);

                QString absolutePath = dir.absoluteFilePath(nameOfObject);
                QFile   outputFile(absolutePath);
                if (outputFile.open(QIODevice::WriteOnly))
                {
                    QTextStream stream(&outputFile);

                    stream << nameOfObject;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToFolderFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Save.png"));
    actionToSetup->setText("HoloLens : Export to Folder");
}
