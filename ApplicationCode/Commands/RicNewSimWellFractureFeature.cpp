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

#include "RicNewSimWellFractureFeature.h"

#include "RiaApplication.h"
//  
// #include "RimMainPlotCollection.h"
// #include "RimOilField.h"
#include "RimProject.h"
#include "RimFractureCollection.h"
// #include "RimSummaryCaseCollection.h"
// #include "RimSummaryCurve.h"
// #include "RimSummaryPlot.h"
// #include "RimSummaryPlotCollection.h"
// 
// #include "RiuMainPlotWindow.h"
// 
// #include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"
// 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include "RimOilField.h"
#include "RimFracture.h"


CAF_CMD_SOURCE_INIT(RicNewSimWellFractureFeature, "RicNewSimWellFractureFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::onActionTriggered(bool isChecked)
{

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    RimOilField* oilfield = project->activeOilField();
    if (oilfield == nullptr) return;

    RimFractureCollection* fracColl = oilfield->fractureCollection();


    if (fracColl)
    {
        RimFracture* fracture = new RimFracture();
        fracColl->fractures.push_back(fracture);
        
        fracture->name = "New Fracture";


        fracColl->updateConnectedEditors();

    }
    



 
//     RimSummaryPlot* plot = selectedSummaryPlot();
//     if (plot)
//     {
//         RimSummaryCurve* newCurve = new RimSummaryCurve();
//         plot->addCurve(newCurve);
// 
//         cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable();
//         newCurve->setColor(curveColor);
// 
//         RimSummaryCase* defaultCase = nullptr;
//         if (project->activeOilField()->summaryCaseCollection()->summaryCaseCount() > 0)
//         {
//             defaultCase = project->activeOilField()->summaryCaseCollection()->summaryCase(0);
//             newCurve->setSummaryCase(defaultCase);
//             newCurve->setVariable("FOPT");
//             newCurve->loadDataAndUpdate();
//         }
// 
//         plot->updateConnectedEditors();
// 
//         RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);
//     }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureFeature::setupActionLook(QAction* actionToSetup)
{
//    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureFeature::isCommandEnabled()
{
    return true;
}
