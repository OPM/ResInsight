/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RicImportGeneralDataFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RicImportEclipseCaseFeature.h"
#include "RicImportInputEclipseCaseFeature.h"
#include "RicImportSummaryCaseFeature.h"
#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QFileDialog>
#include <QString>
#include <QStringList>

CAF_CMD_SOURCE_INIT(RicImportGeneralDataFeature, "RicImportGeneralDataFeature");
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportGeneralDataFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGeneralDataFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString eclipseGridFilePattern("*.GRID *.EGRID");
    QString eclipseInputFilePattern("*.GRDECL");
    QString eclipseSummaryFilePattern("*.SMSPEC");
        
    QStringList filePatternTexts;
    filePatternTexts += QString("Eclipse Files (%1 %2 %3)").arg(eclipseGridFilePattern).arg(eclipseInputFilePattern).arg(eclipseSummaryFilePattern);
    filePatternTexts += QString("Eclipse Grid Files (%1)").arg(eclipseGridFilePattern);
    filePatternTexts += QString("Eclipse Input Files and Input Properties (%1)").arg(eclipseInputFilePattern);
    filePatternTexts += QString("Eclipse Summary File (%1)").arg(eclipseSummaryFilePattern);

    QString fullPattern = filePatternTexts.join(";;");

    QString     defaultDir = app->lastUsedDialogDirectory("GENERAL_DATA");
    QStringList fileNames  = QFileDialog::getOpenFileNames(
        Riu3DMainWindowTools::mainWindowWidget(), "Import Data File", defaultDir, fullPattern);
   
    if (fileNames.empty()) return;
        
    defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setLastUsedDialogDirectory("GENERAL_DATA", defaultDir);

    QStringList eclipseCaseFiles;
    QStringList eclipseInputFiles;
    QStringList eclipseSummaryFiles;

    for (const QString& fileName : fileNames)
    {
        if (fileName.endsWith("GRID"))
        {
            eclipseCaseFiles.push_back(fileName);
        }
        else if (fileName.endsWith("GRDECL"))
        {
            eclipseInputFiles.push_back(fileName);
        }
        else if (fileName.endsWith("SMSPEC"))
        {
            eclipseSummaryFiles.push_back(fileName);
        }
    }

    if (!eclipseCaseFiles.empty())
    {
        RicImportEclipseCaseFeature::openEclipseCaseFromFileNames(eclipseCaseFiles);
    }
    if (!eclipseInputFiles.empty())
    {
        RicImportInputEclipseCaseFeature::openInputEclipseCaseFromFileNames(eclipseInputFiles);
    }
    if (!eclipseSummaryFiles.empty())
    {
        RicImportSummaryCaseFeature::openSummaryCaseFromFileNames(eclipseSummaryFiles);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportGeneralDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Case48x48.png"));
    actionToSetup->setText("Import Eclipse Data Files");
}
