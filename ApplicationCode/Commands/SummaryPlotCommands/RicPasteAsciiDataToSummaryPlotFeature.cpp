/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicPasteAsciiDataToSummaryPlotFeature.h"

#include "OperationsUsingObjReferences/RicPasteFeatureImpl.h"
#include "RicNewSummaryPlotFeature.h"
#include "RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RiaLogging.h"

#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimAsciiDataCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmDocument.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafPdmSettings.h"

#include "cvfAssert.h"
#include "cvfColor3.h"

#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QMimeData>


CAF_CMD_SOURCE_INIT(RicPasteAsciiDataToSummaryPlotFeature, "RicPasteAsciiDataToSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteAsciiDataToSummaryPlotFeature::isCommandEnabled()
{
    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());

    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);

    RimSummaryPlotCollection* summaryPlotCollection = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlotCollection);
    if (!summaryPlotCollection)
    {
        return false;
    }

    return hasPastedText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeature::onActionTriggered(bool isChecked)
{

    caf::PdmObjectHandle* destinationObject = dynamic_cast<caf::PdmObjectHandle*>(caf::SelectionManager::instance()->selectedItem());
    RimSummaryPlot* summaryPlot = nullptr;
    destinationObject->firstAncestorOrThisOfType(summaryPlot);

    QString text = getPastedData();

    RicPasteAsciiDataToSummaryPlotFeatureUi pasteOptions;
    caf::PdmSettings::readFieldsFromApplicationStore(&pasteOptions);
    if (!summaryPlot) pasteOptions.createNewPlot();
    pasteOptions.setUiModePasteText(text);

    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &pasteOptions, "Set Paste Options", "");
    if (propertyDialog.exec() != QDialog::Accepted) return;

    std::vector<RimAsciiDataCurve*> curves = parseCurves(text, pasteOptions);

    if (curves.size() > 0)
    {
        if (!summaryPlot)
        {
            RimSummaryPlotCollection* summaryPlotCollection = nullptr;
            destinationObject->firstAncestorOrThisOfType(summaryPlotCollection);
            if (!summaryPlotCollection)
            {
                return;
            }
            summaryPlot = createSummaryPlotAndAddToPlotCollection(summaryPlotCollection);
            summaryPlotCollection->updateConnectedEditors();
        }

        caf::PdmSettings::writeFieldsToApplicationStore(&pasteOptions);

        for (RimAsciiDataCurve* curve : curves)
        {
            summaryPlot->addAsciiDataCruve(curve);
        }

        summaryPlot->updateConnectedEditors();
        summaryPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPasteAsciiDataToSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Paste Excel Data to Summary Plot");

    RicPasteFeatureImpl::setIconAndShortcuts(actionToSetup);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicPasteAsciiDataToSummaryPlotFeature::getPastedData()
{
    if (hasPastedText())
    {
        QClipboard* clipboard = QApplication::clipboard();
        return clipboard->text();
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPasteAsciiDataToSummaryPlotFeature::hasPastedText()
{
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if (mimeData->hasText() && mimeData->text().size() > 12)
    {
        QString text = mimeData->text();

        if (text.size() > 12) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimAsciiDataCurve*> RicPasteAsciiDataToSummaryPlotFeature::parseCurves(QString& data, const RicPasteAsciiDataToSummaryPlotFeatureUi& settings)
{
    std::vector<RimAsciiDataCurve*> curves;
    const AsciiDataParseOptions& parseOptions = settings.parseOptions();
    RifCsvUserDataPastedTextParser parser = RifCsvUserDataPastedTextParser(data);
    
    if (!parser.parse(parseOptions))
    {
        return curves;
    }

    if (parser.tableData().columnInfos().empty() || !parser.dateTimeColumn())
    {
        return curves;
    }

    std::map< CurveType, std::vector<RimAsciiDataCurve*> > curveToTypeMap;

    QString curvePrefix = parseOptions.curvePrefix;

    for (size_t i = 0; i < parser.tableData().columnInfos().size(); i++)
    {
        const Column* col = parser.columnInfo(i);
        if (col->dataType != Column::NUMERIC) continue;

        RimAsciiDataCurve* curve = new RimAsciiDataCurve();
        curve->setTimeSteps(parser.dateTimeColumn()->dateTimeValues);
        curve->setValues(parser.columnInfo(i)->values);
        if (curvePrefix.isEmpty())
        {
            curve->setTitle(QString::fromStdString(col->columnName()));
        }
        else
        {
            curve->setTitle(QString("%1: %2").arg(curvePrefix).arg(QString::fromStdString(col->columnName())));
        }
        // Appearance
        curve->setSymbol(parseOptions.curveSymbol);
        curve->setLineStyle(parseOptions.curveLineStyle);
        curve->setSymbolSkipDinstance(parseOptions.curveSymbolSkipDistance);
        curveToTypeMap[guessCurveType(QString::fromStdString(col->columnName()))].push_back(curve);
        curves.push_back(curve);
    }

    for (auto& it : curveToTypeMap)
    {
        for (int i = 0; i < static_cast<int>(it.second.size()); ++i)
        {
            cvf::Color3f color;
            switch (it.first)
            {
            case CURVE_GAS:
                color = RimSummaryCurveAppearanceCalculator::cycledGreenColor(i);
                break;
            case CURVE_OIL:
                color = RimSummaryCurveAppearanceCalculator::cycledRedColor(i);
                break;
            case CURVE_WAT:
                color = RimSummaryCurveAppearanceCalculator::cycledBlueColor(i);
                break;
            default:
                color = RimSummaryCurveAppearanceCalculator::cycledNoneRGBBrColor(i);
                break;
            }
            it.second[i]->setColor(color);
        }
    }

    return curves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPasteAsciiDataToSummaryPlotFeature::CurveType RicPasteAsciiDataToSummaryPlotFeature::guessCurveType(const QString& curveName)
{
    if (curveName.contains("SW") || curveName.contains("water", Qt::CaseInsensitive))
    {
        return CURVE_WAT;
    }
    else if (curveName.contains("oil", Qt::CaseInsensitive))
    {
        return CURVE_OIL;
    }
    else if (curveName.contains("gas", Qt::CaseInsensitive))
    {
        return CURVE_GAS;
    }
    return CURVE_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicPasteAsciiDataToSummaryPlotFeature::createSummaryPlotAndAddToPlotCollection(RimSummaryPlotCollection *plotCollection)
{
    QString name = QString("Summary Plot %1").arg(plotCollection->summaryPlots.size() + 1);

    return plotCollection->createNamedSummaryPlot(name);
}
