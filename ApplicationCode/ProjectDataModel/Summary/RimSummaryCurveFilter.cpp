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

#include "RigSummaryCaseData.h"

#include "RimDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"


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
    m_selectedSummaryCases.uiCapability()->setUiTreeChildrenHidden(true);
    m_selectedSummaryCases.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_selectedSummaryCases.uiCapability()->setAutoAddingOptionFromValue(false);
    m_selectedSummaryCases.xmlCapability()->setIOWritable(false);
    m_selectedSummaryCases.xmlCapability()->setIOReadable(false);
    m_selectedSummaryCases.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitFieldNoDefault(&m_summaryFilter, "VarListFilter", "Filter", "", "", "");
    m_summaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_summaryFilter.uiCapability()->setUiHidden(true);
    m_summaryFilter = new RimSummaryFilter();

    CAF_PDM_InitFieldNoDefault(&m_uiFilterResultMultiSelection, "FilterResultSelection", "Filter Result", "", "Ctrl-A : Select All", "");
    m_uiFilterResultMultiSelection.xmlCapability()->setIOWritable(false);
    m_uiFilterResultMultiSelection.xmlCapability()->setIOReadable(false);
    m_uiFilterResultMultiSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_uiFilterResultMultiSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_uiFilterResultMultiSelection.uiCapability()->setAutoAddingOptionFromValue(false);
    
    CAF_PDM_InitFieldNoDefault(&m_curves, "FilteredCurves", "Filtered Curves", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiTreeChildrenHidden(false);

    CAF_PDM_InitFieldNoDefault(&m_applyButtonField, "ApplySelection", "Apply", "", "", "");
    m_applyButtonField.xmlCapability()->setIOWritable(false);
    m_applyButtonField.xmlCapability()->setIOReadable(false);
    m_applyButtonField = false;
    m_applyButtonField.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_applyButtonField.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_autoApplyFilterChanges, "AutoApplyFilterChanges", false, "Auto Apply Changes", "", "", "");

    CAF_PDM_InitField(&m_showCurves, "IsActive", true, "Show Curves", "", "", "");
    m_showCurves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_useAutoAppearanceAssignment, "UseAutoAppearanceAssignment", true, "Auto", "", "", "" );

    CAF_PDM_InitFieldNoDefault(&m_caseAppearanceType,     "CaseAppearanceType",     "Case",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_variableAppearanceType, "VariableAppearanceType", "Vector", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_wellAppearanceType,     "WellAppearanceType",     "Well",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_groupAppearanceType,    "GroupAppearanceType",    "Group",  "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_regionAppearanceType,   "RegionAppearanceType",   "Region", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");
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
void RimSummaryCurveFilter::createCurves(RimSummaryCase* summaryCase, const QString& stringFilter)
{
    if (summaryCase)
    {
        std::vector<RimSummaryCase*> selectedCases;
        selectedCases.push_back(summaryCase);

        m_summaryFilter->setCompleteVarStringFilter(stringFilter);

        std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > newCurveDefinitions;

        createSetOfCasesAndResultAdresses(selectedCases, *m_summaryFilter, &newCurveDefinitions);

        createCurvesFromCurveDefinitions(newCurveDefinitions);
    }
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
        std::set<RifEclipseSummaryAddress> addrUnion = findPossibleSummaryAddresses();

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

    caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroup("Vector Selection");

    m_summaryFilter->defineUiOrdering(uiConfigName, *curveVarSelectionGroup);

    curveVarSelectionGroup->add(&m_uiFilterResultMultiSelection);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance settings");
    appearanceGroup->add(&m_useAutoAppearanceAssignment);
    if(!m_useAutoAppearanceAssignment())
    {
        appearanceGroup->add(&m_caseAppearanceType);
        appearanceGroup->add(&m_variableAppearanceType);
        appearanceGroup->add(&m_wellAppearanceType);
        appearanceGroup->add(&m_groupAppearanceType);
        appearanceGroup->add(&m_regionAppearanceType);
    }

    // Set sensitivity
    {
        m_caseAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_variableAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_wellAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_groupAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
        m_regionAppearanceType.uiCapability()->setUiReadOnly(m_useAutoAppearanceAssignment);
    }

    uiOrdering.add(&m_plotAxis);
    uiOrdering.add(&m_autoApplyFilterChanges);
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
        if (m_autoApplyFilterChanges)
        {
            loadDataAndUpdatePlot();
        }
    }
    else if (changedField == &m_applyButtonField)
    {
        m_applyButtonField = false;

        loadDataAndUpdatePlot();
    }
    else if (changedField == &m_showCurves)
    {
        for(RimSummaryCurve* curve : m_curves)
        {
            curve->updateCurveVisibility();
        }
        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }
    else if (changedField == &m_plotAxis)
    {
        updatePlotAxisForCurves();
    }
    else if (changedField == &m_selectedSummaryCases)
    {
        if (newValue.toList().size() < 1)
        {
            if (m_selectionCache.size() > 0)
            {
                m_selectedSummaryCases.setValue(m_selectionCache);
            }
        }
        else
        {
            m_selectionCache = m_selectedSummaryCases.value();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::loadDataAndUpdatePlot()
{
    syncCurvesFromUiSelection();
    loadDataAndUpdate();

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    plot->updateAxes();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::syncCurvesFromUiSelection()
{
    // Create a search map containing whats supposed to be curves

    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> > newCurveDefinitions;

    // Populate the newCurveDefinitions from the Gui

    std::set<RifEclipseSummaryAddress> addrUnion = findPossibleSummaryAddresses();

    for (RimSummaryCase* currentCase: m_selectedSummaryCases)
    {
        if (!currentCase || !currentCase->caseData() || !currentCase->caseData()->summaryReader()) continue;

        RifReaderEclipseSummary* reader = currentCase->caseData()->summaryReader();

        for(const RifEclipseSummaryAddress& addr: m_uiFilterResultMultiSelection.v())
        {
            if(!reader->hasAddress(addr)) continue;
            if (addrUnion.count(addr) == 0 ) continue; // Wash the possible "old" ui selection with new filter
             
            newCurveDefinitions.insert(std::make_pair(currentCase, addr));
        }
    }

    m_curves.deleteAllChildObjects();

    createCurvesFromCurveDefinitions(newCurveDefinitions);
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

    if (m_curves.size()) // Only sync the selected cases if we actually have some curves. To avoid user getting an empty variable list accidentally
    {
        m_selectedSummaryCases.clear();

        for(RimSummaryCase* currCase: referredCases)
        {
            m_selectedSummaryCases.push_back(currCase);
        }
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
void RimSummaryCurveFilter::updatePlotAxisForCurves()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->setPlotAxis(m_plotAxis());
        curve->updateQwtPlotAxis();
    }

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    plot->updateAxes();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::createSetOfCasesAndResultAdresses(
    const std::vector<RimSummaryCase*>& cases,
    const RimSummaryFilter& filter,
    std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >* curveDefinitions) const
{
    for (RimSummaryCase* currentCase : cases)
    {
        if (!currentCase || !currentCase->caseData() || !currentCase->caseData()->summaryReader()) continue;

        RifReaderEclipseSummary* reader = currentCase->caseData()->summaryReader();

        const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();
        int addressCount = static_cast<int>(allAddresses.size());
        for (int i = 0; i < addressCount; i++)
        {
            if (!filter.isIncludedByFilter(allAddresses[i])) continue;

            curveDefinitions->insert(std::make_pair(currentCase, allAddresses[i]));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::createCurvesFromCurveDefinitions(const std::set<std::pair<RimSummaryCase*, RifEclipseSummaryAddress> >& curveDefinitions)
{
    int colorIndex = 2;
    int lineStyleIdx = -1;

    RimSummaryCase* prevCase = nullptr;
    RimPlotCurve::LineStyleEnum lineStyle = RimPlotCurve::STYLE_SOLID;
    RimSummaryCurveAppearanceCalculator curveLookCalc(curveDefinitions);

    if (!m_useAutoAppearanceAssignment())
    {
        curveLookCalc.assignDimensions( m_caseAppearanceType(), 
                                        m_variableAppearanceType(),
                                        m_wellAppearanceType(),
                                        m_groupAppearanceType(),
                                        m_regionAppearanceType());
    }
    else
    {
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType caseAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType variAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType wellAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType gropAppearance;
        RimSummaryCurveAppearanceCalculator::CurveAppearanceType regiAppearance;

        curveLookCalc.getDimensions(&caseAppearance,
                                    &variAppearance,
                                    &wellAppearance,
                                    &gropAppearance,
                                    &regiAppearance);

        m_caseAppearanceType     = caseAppearance;
        m_variableAppearanceType = variAppearance;
        m_wellAppearanceType     = wellAppearance;
        m_groupAppearanceType    = gropAppearance;
        m_regionAppearanceType   = regiAppearance;
    }

    for (auto& caseAddrPair : curveDefinitions)
    {
        RimSummaryCase* currentCase = caseAddrPair.first;

        RimSummaryCurve* curve = new RimSummaryCurve();
        curve->setParentQwtPlot(m_parentQwtPlot);
        curve->setSummaryCase(currentCase);
        curve->setSummaryAddress(caseAddrPair.second);
        curve->setPlotAxis(m_plotAxis());

        m_curves.push_back(curve);

        curveLookCalc.setupCurveLook(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::updateCaseNameHasChanged()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->updateCurveName();
        curve->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimDefines::PlotAxis RimSummaryCurveFilter::associatedPlotAxis() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::setPlotAxis(RimDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;
    updateConnectedEditors();

    updatePlotAxisForCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveFilter::updateCompleteVariableStringFilterChanged()
{
    if (m_autoApplyFilterChanges)
    {
        loadDataAndUpdatePlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurveFilter::isCurvesVisible()
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCurveFilter::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryCurveFilter::findPossibleSummaryAddresses()
{
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

    return addrUnion;
}
