/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicNewGridTimeHistoryCurveFeature.h"

#include "RiaApplication.h"
#include "RiaSummaryTools.h"

#include "RicNewSummaryCurveFeature.h"
#include "RicSelectSummaryPlotUI.h"
#include "RicWellLogTools.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimGridTimeHistoryCurve.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuMainPlotWindow.h"
#include "RiuSelectionManager.h"

#include "cafPdmReferenceHelper.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfAssert.h"
#include "cvfColor3.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewGridTimeHistoryCurveFeature, "RicNewGridTimeHistoryCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::createCurveFromSelectionItem(const RiuSelectionItem* selectionItem, RimSummaryPlot* plot)
{
    CVF_ASSERT(selectionItem);
    CVF_ASSERT(plot);

    RimGridTimeHistoryCurve* newCurve = new RimGridTimeHistoryCurve();
    newCurve->setFromSelectionItem(selectionItem);
    newCurve->setLineThickness(2);
        
    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable(plot->curveCount());
    newCurve->setColor(curveColor);

    plot->addGridTimeHistoryCurve(newCurve);

    newCurve->loadDataAndUpdate(true);

    plot->updateConnectedEditors();

    RiaApplication::instance()->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem(newCurve);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewGridTimeHistoryCurveFeature::userSelectedSummaryPlot()
{
    const QString lastUsedSummaryPlotKey("lastUsedSummaryPlotKey");

    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    RimSummaryPlot* defaultSelectedPlot = nullptr;
    {
        QString lastUsedPlotRef = RiaApplication::instance()->cacheDataObject(lastUsedSummaryPlotKey).toString();
        RimSummaryPlot* lastUsedPlot = dynamic_cast<RimSummaryPlot*>(caf::PdmReferenceHelper::objectFromReference(RiaApplication::instance()->project(), lastUsedPlotRef));
        if (lastUsedPlot)
        {
            defaultSelectedPlot = lastUsedPlot;
        }

        if (!defaultSelectedPlot)
        {
            defaultSelectedPlot = dynamic_cast<RimSummaryPlot*>( RiaApplication::instance()->activePlotWindow() );
        }

        if (!defaultSelectedPlot && summaryPlotColl->summaryPlots().size() > 0)
        {
            defaultSelectedPlot = summaryPlotColl->summaryPlots()[0];
        }
    }

    RicSelectSummaryPlotUI featureUi;
    if (defaultSelectedPlot)
    {
        featureUi.setDefaultSummaryPlot(defaultSelectedPlot);
    }

    QString newPlotName = RicNewGridTimeHistoryCurveFeature::suggestedNewPlotName();
    featureUi.setSuggestedPlotName(newPlotName);

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Select Destination Plot", "");
    propertyDialog.resize(QSize(400, 200));

    if (propertyDialog.exec() != QDialog::Accepted) return nullptr;

    RimSummaryPlot* summaryPlot = nullptr;
    if (featureUi.isCreateNewPlotChecked())
    {
        RimSummaryPlot* plot = summaryPlotColl->createNamedSummaryPlot(featureUi.newPlotName());

        summaryPlotColl->updateConnectedEditors();

        plot->loadDataAndUpdate();

        summaryPlot = plot;
    }
    else
    {
        summaryPlot = featureUi.selectedSummaryPlot();
    }
    
    QString refFromProjectToView = caf::PdmReferenceHelper::referenceFromRootToObject(RiaApplication::instance()->project(), summaryPlot);
    RiaApplication::instance()->setCacheDataObject(lastUsedSummaryPlotKey, refFromProjectToView);

    return summaryPlot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewGridTimeHistoryCurveFeature::suggestedNewPlotName()
{

    QString resultName;
    {
        Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
        RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(activeView);
        if (eclView)
        {
            RimEclipseResultDefinition* resDef = eclView->cellResult();
            resultName = resDef->resultVariableUiShortName();
        }

        RimGeoMechView* geoView = dynamic_cast<RimGeoMechView*>(activeView);
        if (geoView)
        {
            // NOTE: See also RimGeoMechProertyFilter for generation of result name

            RimGeoMechResultDefinition* resultDefinition = geoView->cellResultResultDefinition();

            RigFemResultAddress resultAddress = resultDefinition->resultAddress();

            if (resultAddress.resultPosType == RIG_FORMATION_NAMES)
            {
                resultName = resultDefinition->resultFieldName();
            }
            else
            {
                QString posName;

                switch (resultAddress.resultPosType)
                {
                case RIG_NODAL: posName = "N"; break;
                case RIG_ELEMENT_NODAL: posName = "EN"; break;
                case RIG_INTEGRATION_POINT: posName = "IP"; break;
                }

                QString fieldUiName = resultDefinition->resultFieldUiName();
                QString compoUiName = resultDefinition->resultComponentUiName();

                resultName = posName + ", " + fieldUiName + ", " + compoUiName;
            }
        }
    }
    
    QString plotName = "New Plot Name";
    if (!resultName.isEmpty())
    {
        plotName = QString("Cell Result - %1").arg(resultName);
    }

    return plotName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewGridTimeHistoryCurveFeature::isCommandEnabled()
{
    if (RicWellLogTools::isWellPathOrSimWellSelectedInView()) return false;

    std::vector<RiuSelectionItem*> items;
    RiuSelectionManager::instance()->selectedItems(items);

    if (items.size() > 0)
    {
        const RiuEclipseSelectionItem* eclSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(items[0]);
        if (eclSelectionItem)
        {
            if (eclSelectionItem->m_view->cellResult()->isFlowDiagOrInjectionFlooding() && eclSelectionItem->m_view->cellResult()->resultVariable() != RIG_NUM_FLOODED_PV)
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::onActionTriggered(bool isChecked)
{
    RimSummaryPlot* summaryPlot = RicNewGridTimeHistoryCurveFeature::userSelectedSummaryPlot();
    if (!summaryPlot) return;

    std::vector<RiuSelectionItem*> items;
    RiuSelectionManager::instance()->selectedItems(items);
    CVF_ASSERT(items.size() > 0);

    for (auto item : items)
    {
        RicNewGridTimeHistoryCurveFeature::createCurveFromSelectionItem(item, summaryPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewGridTimeHistoryCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Plot Time History for Selected Cells");
    actionToSetup->setIcon(QIcon(":/SummaryCurve16x16.png"));
}

