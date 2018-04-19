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

#include "RicImportEnsambleFeature.h"

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

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include "SummaryPlotCommands/RicNewSummaryPlotFeature.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>


CAF_CMD_SOURCE_INIT(RicImportEnsambleFeature, "RicImportEnsambleFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEnsambleFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEnsambleFeature::onActionTriggered(bool isChecked)
{
    RiaApplication*                 app = RiaApplication::instance();

    std::vector<RimSummaryCase*> cases = RicImportSummaryCasesFeature::importSummaryCases("Import Ensamble");
    
    if (cases.empty()) return;
    validateEnsambleCases(cases);

    QString ensambleName = askForEnsambleName();
    if (ensambleName.isEmpty()) return;

    RicImportSummaryCasesFeature::addSummaryCases(cases);
    RicCreateSummaryCaseCollectionFeature::groupSummaryCases(cases, ensambleName);

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
void RicImportEnsambleFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/SummaryCase48x48.png"));
    actionToSetup->setText("Import Ensamble");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEnsambleFeature::validateEnsambleCases(std::vector<RimSummaryCase*> cases)
{
    // Validate ensamble parameters
    try
    {
        std::hash<std::string> paramsHasher;
        size_t paramsHash = 0;

        for (RimSummaryCase* rimCase : cases)
        {
            if (rimCase->caseRealizationParameters().isNull() || rimCase->caseRealizationParameters()->parameters().empty())
            {
                throw QString("The case %1 has no ensamble parameters").arg(rimCase->summaryHeaderFilename());
            }
            else
            {
                QString paramNames;
                for (std::pair<QString, double> paramPair : rimCase->caseRealizationParameters()->parameters())
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
                    throw QString("Ensamble parameters differ between cases");
                }
            }
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
QString RicImportEnsambleFeature::askForEnsambleName()
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
