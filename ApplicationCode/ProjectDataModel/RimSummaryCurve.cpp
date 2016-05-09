/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryCurve.h"

#include "RiaApplication.h"
#include "RifReaderEclipseSummary.h"
#include "RimDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RiuResultQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT(RimSummaryCurve, "SummaryCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::RimSummaryCurve()
{
    CAF_PDM_InitObject("Summary Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseCase, "ReferencedEclipseCase", "Eclipse Case", "", "", "");
    m_eclipseCase.uiCapability()->setUiChildrenHidden(true);
    
    // TODO: Implement setUiTreeHidden 
    //m_eclipseCase.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_variableName, "SummaryVariableName", "Variable Name", "", "", "");

    m_variableName.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve::~RimSummaryCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;
    if (fieldNeedingOptions == &m_variableName)
    {
        if (m_eclipseCase)
        {
            RifReaderEclipseSummary* reader = summaryReader();
            if (reader)
            {
                std::vector<std::string> varNames = reader->variableNames();

                for (size_t i = 0; i < varNames.size(); i++)
                {
                    std::string name = varNames[i];

                    QString s = QString::fromStdString(name);
                    optionList.push_back(caf::PdmOptionItemInfo(s, s));
                }
            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), RimDefines::undefinedResultName()));

            if (useOptionsOnly) *useOptionsOnly = true;
        }
    }
    else if (fieldNeedingOptions == &m_eclipseCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimCase*> cases;

        proj->allCases(cases);

        for (size_t i = 0; i < cases.size(); i++)
        {
            RimCase* rimCase = cases[i];

            optionList.push_back(caf::PdmOptionItemInfo(rimCase->caseUserDescription(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(rimCase))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }

    return optionList;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_variableName)
    {
        RimSummaryPlot* summaryPlot = NULL;
        this->firstAnchestorOrThisOfType(summaryPlot);
        if (summaryPlot)
        {
            //summaryPlot->redrawAllCurves();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryCurve::summaryReader()
{
    RimSummaryPlotCollection* plotCollection = NULL;
    this->firstAnchestorOrThisOfType(plotCollection);

    return plotCollection->getOrCreateSummaryFileReader(m_eclipseCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    // TODO: Used to hide the entry for a case in the tree view as we have no
    // setUiTreeHidden(true)
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurve::curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values)
{
    RifReaderEclipseSummary* reader = summaryReader();

    if (timeSteps)
    {
        std::vector<time_t> times = reader->timeSteps();
        *timeSteps = RifReaderEclipseSummary::fromTimeT(times);
    }

    if (values)
    {
        std::string keyword = m_variableName().toStdString();
        reader->values(keyword, values);
    }
}
