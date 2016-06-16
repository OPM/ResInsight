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
#include "cafPdmUiPushButtonEditor.h"
#include "WellLogCommands\RicWellLogPlotCurveFeatureImpl.h"


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
    m_curves.uiCapability()->setUiChildrenHidden(false);

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "Apply", "", "", "");
    m_applyButtonField.xmlCapability()->setIOWritable(false);
    m_applyButtonField.xmlCapability()->setIOReadable(false);
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveFilter::~RimSummaryCurveFilter()
{
    delete m_summaryFilter();
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
    uiOrdering.add(&m_applyButtonField);
    uiOrdering.setForgetRemainingFields(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if(changedField == &m_uiFilterResultMultiSelection)
    {
        
    }
    else if (changedField == &m_applyButtonField)
    {
        syncCurvesFromUiSelection();
        loadDataAndUpdate();
        m_applyButtonField = false;
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
    m_parentQwtPlot = plot;
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
void RimSummaryCurveFilter::syncCurvesFromUiSelection()
{
    // Create a search map containing whats supposed to be

    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > newCurveDefinitions;

    // Populate the newCurveDefinitions from the Gui
    
    for (int caseIdx = 0; caseIdx < 1; ++caseIdx)
    {
        RimSummaryCase* currentCase = m_selectedSummaryCase();
        for(const RifEclipseSummaryAddress& addr: m_uiFilterResultMultiSelection.v())
        {
            newCurveDefinitions.insert(std::make_pair(currentCase, addr));
        }
    }

    // Delete all existing curves that is not matching
    // Remove the entries in the search set that we already have
    for(RimSummaryCurve* curve: m_curves)
    {
        auto foundIt = newCurveDefinitions.find(std::make_pair(curve->summaryCase(), curve->summaryAddress() ));
        if (foundIt == newCurveDefinitions.end())
        {
            delete curve;
        }
        else
        {
            newCurveDefinitions.erase(foundIt);
        }
    }
    m_curves.removeChildObject(nullptr);

    // Create all new curves that is missing

    for (auto& caseAddrPair: newCurveDefinitions)
    {
        RimSummaryCase* currentCase = caseAddrPair.first;
        RimSummaryCurve* curve = new RimSummaryCurve();

        curve->setParentQwtPlot(m_parentQwtPlot);
        curve->setSummaryCase(currentCase);
        curve->setSummaryAddress(caseAddrPair.second);
        cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable();
        curve->setColor(curveColor);
        m_curves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::syncUiSelectionFromCurves()
{
    
    for(RimSummaryCurve* curve: m_curves)
    {
        
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if(&m_applyButtonField == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        attrib->m_buttonText = "Apply" ;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::loadDataAndUpdate()
{
    for (RimSummaryCurve* curve: m_curves)
    {
        curve->loadDataAndUpdate();
    }
}
