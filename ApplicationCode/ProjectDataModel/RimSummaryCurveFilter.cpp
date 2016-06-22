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
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"


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
    CAF_PDM_InitObject("Curve Filter", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedSummaryCases, "SummaryCases", "Cases", "", "", "");
    m_selectedSummaryCases.uiCapability()->setUiChildrenHidden(true);
    m_selectedSummaryCases.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedSummaryCases.uiCapability()->setAutoAddingOptionFromValue(false);
    m_selectedSummaryCases.xmlCapability()->setIOWritable(false);
    m_selectedSummaryCases.xmlCapability()->setIOReadable(false);
    m_selectedSummaryCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitFieldNoDefault(&m_selectedVariableDisplayField, "SelectedVariableDisplayVar", "Variables", "", "", "");
    m_selectedVariableDisplayField.xmlCapability()->setIOWritable(false);
    m_selectedVariableDisplayField.xmlCapability()->setIOReadable(false);
    m_selectedVariableDisplayField.uiCapability()->setUiReadOnly(true);
    m_selectedVariableDisplayField.uiCapability()->setUiHidden(true);

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

    if (fieldNeedingOptions == &m_selectedSummaryCases)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCase*> cases;

        proj->allSummaryCases(cases);

        for (size_t i = 0; i < cases.size(); i++)
        {
            RimSummaryCase* rimCase = cases[i];

            optionList.push_back(caf::PdmOptionItemInfo(rimCase->caseName(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(rimCase))));
        }
    }
    else if(fieldNeedingOptions == &m_uiFilterResultMultiSelection)
    {
        size_t caseCount = m_selectedSummaryCases.size();
        std::set<RifEclipseSummaryAddress> addrUnion;

        for(RimSummaryCase* currCase: m_selectedSummaryCases)
        {
            RifReaderEclipseSummary* reader = nullptr;
            if(currCase && currCase->caseData()) reader = currCase->caseData()->summaryReader();

            if(reader)
            {
                const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();
                int addressCount = static_cast<int>(allAddresses.size());

                for(int i = 0; i <addressCount; i++)
                {
                    if(!m_summaryFilter->isIncludedByFilter(allAddresses[i])) continue;
                    addrUnion.insert(allAddresses[i]);
                }
            }
        }

        for(const auto& address: addrUnion)
        {
            std::string name = address.uiText();
            QString s = QString::fromStdString(name);
            optionList.push_back(caf::PdmOptionItemInfo(s, QVariant::fromValue(address)));
        }
    }

    if(useOptionsOnly) *useOptionsOnly = true;

    return optionList;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Summary Vectors");
    curveDataGroup->add(&m_selectedSummaryCases);
    curveDataGroup->add(&m_selectedVariableDisplayField);

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Vector Selection");

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

        RimSummaryPlot* plot = nullptr;
        firstAnchestorOrThisOfType(plot);
        plot->updateYAxisUnit();
    }
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
void RimSummaryCurveFilter::detachQwtCurves()
{
    for(RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryCurveFilter::findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for(RimSummaryCurve* rimCurve: m_curves)
    {
        if(rimCurve->qwtPlotCurve() == qwtCurve)
        {
            return rimCurve;
        }
    }

    return NULL;
}



static const int RI_LOGPLOT_CURVECOLORSCOUNT = 15;
static const int RI_LOGPLOT_CURVECOLORS[] =
{
    Qt::black,
    Qt::darkBlue,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkYellow,
    Qt::darkMagenta,
    Qt::darkCyan,
    Qt::darkGray,
    Qt::blue,
    Qt::red,
    Qt::green,
    Qt::yellow,
    Qt::magenta,
    Qt::cyan,
    Qt::gray
};

//--------------------------------------------------------------------------------------------------
/// Pick default curve color from an index based palette
//--------------------------------------------------------------------------------------------------
cvf::Color3f curveColorFromTable(int colorIndex)
{
    QColor color = QColor(Qt::GlobalColor(RI_LOGPLOT_CURVECOLORS[colorIndex % RI_LOGPLOT_CURVECOLORSCOUNT]));
    ++colorIndex;
    cvf::Color3f cvfColor(color.redF(), color.greenF(), color.blueF());
    return cvfColor;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::syncCurvesFromUiSelection()
{
    // Create a search map containing whats supposed to be curves

    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > newCurveDefinitions;

    // Populate the newCurveDefinitions from the Gui
    
    for (RimSummaryCase* currentCase: m_selectedSummaryCases)
    {
        if (!currentCase || !currentCase->caseData() || !currentCase->caseData()->summaryReader()) continue;

        RifReaderEclipseSummary* reader = currentCase->caseData()->summaryReader();

        for(const RifEclipseSummaryAddress& addr: m_uiFilterResultMultiSelection.v())
        {
            if(!reader->hasAddress(addr)) continue;

            newCurveDefinitions.insert(std::make_pair(currentCase, addr));
        }
    }

    #if 0
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
    #else
    m_curves.deleteAllChildObjects();
    #endif


    // Create all new curves that is missing
    int colorIndex = 2;
    int lineStyleIdx = -1;

    RimSummaryCase* prevCase = nullptr;
    RimPlotCurve::LineStyleEnum lineStyle = RimPlotCurve::STYLE_SOLID;

    for (auto& caseAddrPair: newCurveDefinitions)
    {
        RimSummaryCase* currentCase = caseAddrPair.first;


        RimSummaryCurve* curve = new RimSummaryCurve();
        curve->setParentQwtPlot(m_parentQwtPlot);
        curve->setSummaryCase(currentCase);
        curve->setSummaryAddress(caseAddrPair.second);

        if(currentCase != prevCase)
        {
            prevCase = currentCase;
            colorIndex = 2;
            lineStyleIdx ++;
            lineStyle = caf::AppEnum<RimPlotCurve::LineStyleEnum>::fromIndex(lineStyleIdx%caf::AppEnum<RimPlotCurve::LineStyleEnum>::size());
            if(lineStyle == RimPlotCurve::STYLE_NONE)
            {
                lineStyle = RimPlotCurve::STYLE_SOLID;
                lineStyleIdx++;
            }
        }

        cvf::Color3f curveColor = curveColorFromTable(colorIndex);
        colorIndex++;

        curve->setColor(curveColor);
        curve->setLineStyle(lineStyle);

        m_curves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::syncUiSelectionFromCurves()
{
    // Create a search map containing whats supposed to be uiSelected
    std::set<RimSummaryCase*> referredCases;
    std::set<RifEclipseSummaryAddress> existingCurveDefinitions;

    // Populate the existingCurveDefinitions from the existing curves
    for(RimSummaryCurve* curve: m_curves)
    {
        existingCurveDefinitions.insert(curve->summaryAddress());
        referredCases.insert(curve->summaryCase());
    }

    m_selectedSummaryCases.clear();

    for (RimSummaryCase* currCase: referredCases)
    {
        m_selectedSummaryCases.push_back(currCase);
    }

    m_uiFilterResultMultiSelection.v().clear();
    
    for(const RifEclipseSummaryAddress& addr: existingCurveDefinitions)
    {
        m_uiFilterResultMultiSelection.v().push_back(addr);
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

    syncUiSelectionFromCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<std::string> RimSummaryCurveFilter::unitNames()
{
    std::set<std::string> unitNames;
    for(RimSummaryCurve* curve: m_curves)
    {
        if (curve->isCurveVisible()) unitNames.insert( curve->unitName());
    }
    return unitNames;
}
