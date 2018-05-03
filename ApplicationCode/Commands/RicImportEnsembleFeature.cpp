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

#include "RicImportEnsembleFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RicImportSummaryCasesFeature.h"
#include "RicCreateSummaryCaseCollectionFeature.h"

#include "RifSummaryCaseRestartSelector.h"

#include "RimGridSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


CAF_CMD_SOURCE_INIT(RicImportEnsembleFeature, "RicImportEnsembleFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app   = RiaApplication::instance();
    QStringList fileNames = RicImportSummaryCasesFeature::runRecursiveSummaryCaseFileSearchDialog("Import Ensemble");
    
    if (fileNames.isEmpty()) return;

    QString ensembleName = askForEnsembleName();
    if (ensembleName.isEmpty()) return;

    std::vector<RimSummaryCase*> cases;
    RicImportSummaryCasesFeature::createSummaryCasesFromFiles(fileNames, &cases);

    validateEnsembleCases(cases);

    RicImportSummaryCasesFeature::addSummaryCases(cases);
    RicCreateSummaryCaseCollectionFeature::groupSummaryCases(cases, ensembleName);

    std::vector<RimCase*> allCases;
    app->project()->allCases(allCases);

    if (allCases.size() == 0)
    {
        RiuMainWindow::instance()->close();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEnsembleFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryCase48x48.png"));
    actionToSetup->setText("Import Ensemble");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEnsembleFeature::validateEnsembleCases(std::vector<RimSummaryCase*> cases)
{
    // Validate ensemble parameters
    try
    {
        QString errors;
        std::hash<std::string> paramsHasher;
        size_t paramsHash = 0;

        for (RimSummaryCase* rimCase : cases)
        {
            if (rimCase->caseRealizationParameters() == nullptr || rimCase->caseRealizationParameters()->parameters().empty())
            {
                errors.append(QString("The case %1 has no ensemble parameters\n").arg(QFileInfo(rimCase->summaryHeaderFilename()).fileName()));
            }
            else
            {
                QString paramNames;
                for (std::pair<QString, RigCaseRealizationParameters::Value> paramPair : rimCase->caseRealizationParameters()->parameters())
                {
                    paramNames.append(paramPair.first);
                }

                size_t currHash = paramsHasher(paramNames.toStdString());
                if (paramsHash == 0)
                {
                    paramsHash = currHash;
                }
                else if (paramsHash != currHash)
                {
                    throw QString("Ensemble parameters differ between cases");
                }
            }
        }

        if (!errors.isEmpty())
        {
            throw errors;
        }
        return true;
    }
    catch (QString errorMessage)
    {
        QMessageBox mbox;
        mbox.setIcon(QMessageBox::Icon::Warning);
        mbox.setInformativeText(errorMessage);
        mbox.exec();
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicImportEnsembleFeature::askForEnsembleName()
{
    RimProject* project = RiaApplication::instance()->project();
    int groupCount = (int)project->summaryGroups().size() + 1;

    QInputDialog dialog;
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle("Ensemble Name");
    dialog.setLabelText("Ensemble Name");
    dialog.setTextValue(QString("Ensemble %1").arg(groupCount));
    dialog.resize(300, 50);
    dialog.exec();
    return dialog.result() == QDialog::Accepted ? dialog.textValue() : QString("");
}
