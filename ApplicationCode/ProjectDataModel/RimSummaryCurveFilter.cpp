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

#include "RimSummaryCurveFilter.h"

#include "RiaApplication.h"
#include "RifReaderEclipseSummary.h"
#include "RimDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "RiuLineSegmentQwtPlotCurve.h"
#include "qwt_date.h"
#include "RimSummaryCase.h"
#include "RigSummaryCaseData.h"
#include "RimSummaryCurve.h"


QTextStream& operator << (QTextStream& str, const std::vector<RifEclipseSummaryAddress>& sobj)
{
    CVF_ASSERT(false);
    return str;
}

QTextStream& operator >> (QTextStream& str, std::vector<RifEclipseSummaryAddress>& sobj)
{
    CVF_ASSERT(false);
    return str;
}


CAF_PDM_SOURCE_INIT(RimSummaryCurveFilter, "SummaryCurveFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter::RimSummaryCurveFilter()
{
    CAF_PDM_InitObject("Curve Filter", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCase, "SummaryCases", "Cases", "", "", "");
    m_selectedSummaryCase.uiCapability()->setUiChildrenHidden(true);
    
    CAF_PDM_InitFieldNoDefault(&m_selectedVariableDisplayField, "SelectedVariableDisplayVar", "Variables", "", "", "");
    m_selectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_selectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_selectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_summaryFilter, "VarListFilter", "Filter", "", "", "");
    m_summaryFilter.uiCapability()->setUiChildrenHidden(true);
    m_summaryFilter.uiCapability()->setUiHidden(true);
    m_summaryFilter = new RimSummaryFilter();

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultMultiSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_uiFilterResultMultiSelection.xmlCapability()->setIOWritable(false);
    m_uiFilterResultMultiSelection.xmlCapability()->setIOReadable(false);
    m_uiFilterResultMultiSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultMultiSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultMultiSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    
    CAF_PDM_InitFieldNoDefault(&m_curves, "FilteredCurves", "Filtered Curves", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiChildrenHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter::~RimSummaryCurveFilter()
{
    m_curves.deleteAllChildObjects();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryCurveFilter::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_selectedSummaryCase)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (size_t i = 0; i < cases.size(); i++)
        {
            RimSummaryCase* rimCase = cases[i];

            optionList.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(rimCase))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if(fieldNeedingOptions == &m_uiFilterResultMultiSelection)
    {
        if(m_selectedSummaryCase)
        {
            RifReaderEclipseSummary* reader = summaryReader();
            int addressCount = 0;
            if(reader)
            {
                const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();
                addressCount = static_cast<int>(allAddresses.size());
                std::set<RifEclipseSummaryAddress> addrUnion;
                for(int i = 0; i <addressCount; i++)
                {
                    if (!m_summaryFilter->isIncludedByFilter(allAddresses[i])) continue;
                    addrUnion.insert(allAddresses[i]);
                }

                for (const auto& address: addrUnion)
                {
                    std::string name = address.uiText();
                    QString s = QString::fromStdString(name);
                    optionList.push_back(caf::PdmOptionItemInfo(s, QVariant::fromValue(address)));
                }
            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), QVariant::fromValue(RifEclipseSummaryAddress())));

            if(useOptionsOnly) *useOptionsOnly = true;
        }
    }
    return optionList;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Variable");
    curveDataGroup->add(&m_selectedSummaryCase);
    curveDataGroup->add(&m_selectedVariableDisplayField);

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Variable Selection");

    m_summaryFilter->defineUiOrdering(uiConfigName, *curveVarSelectionGroup);

    curveVarSelectionGroup->add(&m_uiFilterResultMultiSelection);

    uiOrdering.setForgetRemainingFields(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if(changedField = &m_uiFilterResultMultiSelection)
    {
        this->loadDataAndUpdate();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseSummary* RimSummaryCurveFilter::summaryReader()
{
    if(!m_selectedSummaryCase()) return nullptr;

    if(!m_selectedSummaryCase->caseData()) return nullptr;

    return m_selectedSummaryCase()->caseData()->summaryReader();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::setParentQwtPlot(QwtPlot* plot)
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->setParentQwtPlot(plot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::detachQwtCurve()
{
    for(RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::syncronizeCurves()
{
    
}

