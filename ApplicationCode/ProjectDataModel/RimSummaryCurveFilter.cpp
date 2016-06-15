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

    CAF_PDM_InitFieldNoDefault(&m_filterType,"SummaryFilterType","Filter Type","","","");
    m_filterType.xmlCapability()->setIOWritable(false);
    m_filterType.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_completeVarStringFilter, "SummaryCompleteVarStringFilter", "Filter", "", "", "");
    m_completeVarStringFilter.xmlCapability()->setIOWritable(false);
    m_completeVarStringFilter.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_filterQuantityName,"SummaryVarQuantityFilter","Quantity","","","");
    m_filterQuantityName.xmlCapability()->setIOWritable(false);
    m_filterQuantityName.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_regionNumberFilter        ,"SummaryRegionNumberFilter","Region","","","");
    m_regionNumberFilter.xmlCapability()->setIOWritable(false);
    m_regionNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_regionNumber2Filter       ,"SummaryRegionNumber2Filter","Region 2","","","");
    m_regionNumber2Filter.xmlCapability()->setIOWritable(false);
    m_regionNumber2Filter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellGroupNameFilter, "SummaryWellGroupNameFilter", "Well Group", "", "", "");
    m_wellGroupNameFilter.xmlCapability()->setIOWritable(false);
    m_wellGroupNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellNameFilter            ,"SummaryWellNameFilter","Well","","","");
    m_wellNameFilter.xmlCapability()->setIOWritable(false);
    m_wellNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_wellSegmentNumberFilter   ,"SummaryWellSegmentNumberFilter","Segment","","","");
    m_wellSegmentNumberFilter.xmlCapability()->setIOWritable(false);
    m_wellSegmentNumberFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_lgrNameFilter             ,"SummaryLgrNameFilter","Lgr","","","");
    m_lgrNameFilter.xmlCapability()->setIOWritable(false);
    m_lgrNameFilter.xmlCapability()->setIOReadable(false);
    CAF_PDM_InitFieldNoDefault(&m_cellIJKFilter               ,"SummaryCellIJKFilter","I, J, K","","","");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultMultiSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_cellIJKFilter.xmlCapability()->setIOWritable(false);
    m_cellIJKFilter.xmlCapability()->setIOReadable(false);
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
                    if (false) continue;
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
    curveVarSelectionGroup->add(&m_filterType);

    caf::PdmUiGroup* curveVarFilterGroup = nullptr;

    if (m_filterType() == RimSummaryCurve::SUM_FILTER_VAR_STRING)
    {
        curveVarSelectionGroup->add(&m_completeVarStringFilter);
    }
    else
    {
        caf::PdmUiGroup* curveVarFilterGroup = curveVarSelectionGroup->addNewGroup("Filter Settings");

        curveVarFilterGroup->add(&m_filterQuantityName);
    
    switch (m_filterType())
    {
        case RimSummaryCurve::SUM_FILTER_ANY:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_wellGroupNameFilter);
            curveVarFilterGroup->add(&m_regionNumberFilter);
            curveVarFilterGroup->add(&m_regionNumber2Filter);
            curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_REGION:
        {
            curveVarFilterGroup->add(&m_regionNumberFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_REGION_2_REGION:
        {
            curveVarFilterGroup->add(&m_regionNumberFilter);
            curveVarFilterGroup->add(&m_regionNumber2Filter);

        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL_GROUP:
        {
            curveVarFilterGroup->add(&m_wellGroupNameFilter);

        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);

        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL_COMPLETION:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
            
        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL_LGR:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL_COMPLETION_LGR:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_WELL_SEGMENT:
        {
            curveVarFilterGroup->add(&m_wellNameFilter);
            curveVarFilterGroup->add(&m_wellSegmentNumberFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_BLOCK:
        {
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;
        case RimSummaryCurve::SUM_FILTER_BLOCK_LGR:
        {
            curveVarFilterGroup->add(&m_lgrNameFilter);
            curveVarFilterGroup->add(&m_cellIJKFilter);
        }
        break;

    }
    }
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

