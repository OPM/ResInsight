/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleCurveSet.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"

#include "SummaryPlotCommands/RicSummaryCurveCreator.h"

#include "RifEnsembleStatisticsReader.h"
#include "RifReaderEclipseSummary.h"

#include "RigStatisticsMath.h"
#include "RiaTimeHistoryCurveMerger.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleStatistics.h"
#include "RimEnsembleStatisticsCase.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAutoName.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryCurveDefSelectionDialog.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfScalarMapper.h"

#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
    template<>
    void AppEnum< RimEnsembleCurveSet::ColorMode >::setUp()
    {
        addItem(RimEnsembleCurveSet::SINGLE_COLOR, "SINGLE_COLOR", "Single Color");
        addItem(RimEnsembleCurveSet::BY_ENSEMBLE_PARAM, "BY_ENSEMBLE_PARAM", "By Ensemble Parameter");
        setDefault(RimEnsembleCurveSet::SINGLE_COLOR);
    }
}


CAF_PDM_SOURCE_INIT(RimEnsembleCurveSet, "RimEnsembleCurveSet");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::RimEnsembleCurveSet()
{
    CAF_PDM_InitObject("Ensemble Curve Set", ":/EnsembleCurveSet16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curves, "EnsembleCurveSet", "Ensemble Curve Set", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiTreeChildrenHidden(false);

    CAF_PDM_InitField(&m_showCurves, "IsActive", true, "Show Curves", "", "", "");
    m_showCurves.uiCapability()->setUiHidden(true);

    // Y Values
    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryGroup, "SummaryGroup", "Ensemble", "", "", "");
    m_yValuesSummaryGroup.uiCapability()->setUiTreeChildrenHidden(true);
    m_yValuesSummaryGroup.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_yValuesSelectedVariableDisplayField, "SelectedVariableDisplayVar", "Vector", "", "", "");
    m_yValuesSelectedVariableDisplayField.xmlCapability()->disableIO();
    m_yValuesSelectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryFilter, "VarListFilter", "Filter", "", "", "");
    m_yValuesSummaryFilter.uiCapability()->setUiTreeChildrenHidden(true);
    m_yValuesSummaryFilter.uiCapability()->setUiHidden(true);

    m_yValuesSummaryFilter = new RimSummaryFilter;

    CAF_PDM_InitFieldNoDefault(&m_yValuesUiFilterResultSelection, "FilterResultSelection", "Filter Result", "", "", "");
    m_yValuesUiFilterResultSelection.xmlCapability()->disableIO();
    m_yValuesUiFilterResultSelection.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    m_yValuesUiFilterResultSelection.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_yValuesUiFilterResultSelection.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_yValuesCurveVariable, "SummaryAddress", "Summary Address", "", "", "");
    m_yValuesCurveVariable.uiCapability()->setUiHidden(true);
    m_yValuesCurveVariable.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_yPushButtonSelectSummaryAddress, "SelectAddress", "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_yPushButtonSelectSummaryAddress);
    m_yPushButtonSelectSummaryAddress = false;

    m_yValuesCurveVariable = new RimSummaryAddress;

    CAF_PDM_InitField(&m_colorMode, "ColorMode", caf::AppEnum<ColorMode>(SINGLE_COLOR), "Coloring Mode", "", "", "");

    CAF_PDM_InitField(&m_color, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    CAF_PDM_InitField(&m_ensembleParameter, "EnsembleParameter", QString(""), "Ensemble Parameter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendConfig", "", "", "", "");
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setColorRange( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE );

    CAF_PDM_InitFieldNoDefault(&m_curveFilters, "CurveFilters", "Curve Filters", "", "", "");
    m_curveFilters = new RimEnsembleCurveFilterCollection();

    CAF_PDM_InitFieldNoDefault(&m_statistics, "Statistics", "Statistics", "", "", "");
    m_statistics = new RimEnsembleStatistics();
    m_statistics.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_userDefinedName, "UserDefinedName", QString("Ensemble Curve Set"), "Curve Set Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_autoGeneratedName, "AutoGeneratedName", "Curve Set Name", "", "", "");
    m_autoGeneratedName.registerGetMethod(this, &RimEnsembleCurveSet::createAutoName);
    m_autoGeneratedName.uiCapability()->setUiReadOnly(true);
    m_autoGeneratedName.xmlCapability()->disableIO();

    CAF_PDM_InitField(&m_isUsingAutoName, "AutoName", true, "Auto Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_summaryAddressNameTools, "SummaryAddressNameTools", "SummaryAddressNameTools", "", "", "");
    m_summaryAddressNameTools.uiCapability()->setUiHidden(true);
    m_summaryAddressNameTools.uiCapability()->setUiTreeChildrenHidden(true);

    m_summaryAddressNameTools = new RimSummaryCurveAutoName;
    
    m_qwtPlotCurveForLegendText = new QwtPlotCurve;
    m_qwtPlotCurveForLegendText->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);

    m_ensembleStatCase.reset(new RimEnsembleStatisticsCase(this));
    m_ensembleStatCase->createSummaryReaderInterface();

    m_disableStatisticCurves = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::~RimEnsembleCurveSet()
{
    m_curves.deleteAllChildObjects();

    RimSummaryPlot* parentPlot;
    firstAncestorOrThisOfType(parentPlot);
    if (parentPlot && parentPlot->qwtPlot())
    {
        m_qwtPlotCurveForLegendText->detach();
        parentPlot->qwtPlot()->removeEnsembleCurveSetLegend(this);
    }

    delete m_qwtPlotCurveForLegendText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::isCurvesVisible()
{
    RimEnsembleCurveSetCollection* coll = nullptr;
    firstAncestorOrThisOfType(coll);
    return m_showCurves() && (coll ? coll->isCurveSetsVisible() : true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setColor(cvf::Color3f color)
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::loadDataAndUpdate(bool updateParentPlot)
{
    m_yValuesSelectedVariableDisplayField = QString::fromStdString(m_yValuesCurveVariable->address().uiText());
    m_yValuesUiFilterResultSelection = m_yValuesCurveVariable->address();

    updateAllCurves();

    if (updateParentPlot)
    {
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted(parentPlot);
        parentPlot->updateAll();
    }

    m_curveFilters->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setParentQwtPlotNoReplot(QwtPlot* plot)
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->setParentQwtPlotNoReplot(plot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::detachQwtCurves()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }

    m_qwtPlotCurveForLegendText->detach();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::addCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        RimSummaryPlot* plot;
        firstAncestorOrThisOfType(plot);
        if (plot) curve->setParentQwtPlotNoReplot(plot->qwtPlot());

        curve->setColor(m_color);
        m_curves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_curves.removeChildObject(curve);
        delete curve;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryAddress(RifEclipseSummaryAddress address)
{
    m_yValuesCurveVariable->setAddress(address);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseSummaryAddress RimEnsembleCurveSet::summaryAddress() const
{
    return m_yValuesCurveVariable->address();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimEnsembleCurveSet::curves() const
{
    return m_curves.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteEnsembleCurves()
{
    std::vector<size_t> curvesIndexesToDelete;
    for (size_t c = 0; c < m_curves.size(); c++)
    {
        RimSummaryCurve* curve = m_curves[c];
        if (curve->summaryAddressY().category() != RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS)
            curvesIndexesToDelete.push_back(c);
    }

    while (curvesIndexesToDelete.size() > 0)
    {
        size_t currIndex = curvesIndexesToDelete.back();
        delete m_curves[currIndex];
        m_curves.erase(currIndex);
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteStatisticsCurves()
{
    std::vector<size_t> curvesIndexesToDelete;
    for (size_t c = 0; c < m_curves.size(); c++)
    {
        RimSummaryCurve* curve = m_curves[c];
        if (curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS)
            curvesIndexesToDelete.push_back(c);
    }

    while (curvesIndexesToDelete.size() > 0)
    {
        size_t currIndex = curvesIndexesToDelete.back();
        delete m_curves[currIndex];
        m_curves.erase(currIndex);
        curvesIndexesToDelete.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEnsembleCurveSet::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::onLegendDefinitionChanged()
{
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::setSummaryCaseCollection(RimSummaryCaseCollection* sumCaseCollection)
{
    m_yValuesSummaryGroup = sumCaseCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimEnsembleCurveSet::summaryCaseCollection() const
{
    return m_yValuesSummaryGroup();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveFilterCollection* RimEnsembleCurveSet::filterCollection() const
{
    return m_curveFilters;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::ColorMode RimEnsembleCurveSet::colorMode() const
{
    return m_colorMode();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EnsembleParameter::Type RimEnsembleCurveSet::currentEnsembleParameterType() const
{
    if (m_colorMode() == BY_ENSEMBLE_PARAM)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
        QString parameterName = m_ensembleParameter();

        if (group && !parameterName.isEmpty())
        {
            auto eParam = group->ensembleParameter(parameterName);
            return eParam.type;
        }
    }
    return EnsembleParameter::TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllCurves()
{
    updateEnsembleCurves();
    updateStatisticsCurves(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    bool updateTextInPlot = false;

    if (changedField == &m_showCurves)
    {
        loadDataAndUpdate(true);

        updateConnectedEditors();

        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(summaryPlot);
        summaryPlot->updateConnectedEditors();

        updateTextInPlot = true;
    }
    else if (changedField == &m_yValuesUiFilterResultSelection)
    {
        m_yValuesCurveVariable->setAddress(m_yValuesUiFilterResultSelection());

        updateAllCurves();
        
        updateTextInPlot = true;
    }
    else if (changedField == &m_yValuesSummaryGroup)
    {
        // Empty address cache
        //m_allAddressesCache.clear();
        updateAllCurves();

        updateTextInPlot = true;
    }
    else if (changedField == &m_color)
    {
        updateCurveColors();
        
        updateTextInPlot = true;
    }
    else if (changedField == &m_ensembleParameter)
    {
        updateLegendMappingMode();
        updateCurveColors();
    }
    else if (changedField == &m_colorMode)
    {
        if (m_ensembleParameter().isEmpty())
        {
            auto params = ensembleParameterNames();
            m_ensembleParameter = !params.empty() ? params.front() : "";
        }
        updateCurveColors();
        
        updateTextInPlot = true;
    }
    else if (changedField == &m_plotAxis)
    {
        for (RimSummaryCurve* curve : curves())
        {
            curve->setLeftOrRightAxisY(m_plotAxis());
        }
        
        updateQwtPlotAxis();
        plot->updateAxes();
        
        updateTextInPlot = true;
    }
    else if (changedField == &m_isUsingAutoName)
    {
        if (!m_isUsingAutoName)
        {
            m_userDefinedName = createAutoName();
        }

        updateTextInPlot = true;
    }
    else if (changedField == &m_userDefinedName)
    {
        updateTextInPlot = true;
    }
    else if (changedField == &m_yPushButtonSelectSummaryAddress)
    {
        RiuSummaryCurveDefSelectionDialog dlg(nullptr);
        RimSummaryCaseCollection* candidateEnsemble = m_yValuesSummaryGroup();
        RifEclipseSummaryAddress candicateAddress = m_yValuesCurveVariable->address();

        dlg.hideSummaryCases();
        dlg.setEnsembleAndAddress(candidateEnsemble, candicateAddress);

        if (dlg.exec() == QDialog::Accepted)
        {
            auto curveSelection = dlg.curveSelection();
            if (!curveSelection.empty())
            {
                m_yValuesSummaryGroup = curveSelection[0].ensemble();
                m_yValuesCurveVariable->setAddress(curveSelection[0].summaryAddress());

                this->loadDataAndUpdate(true);

                plot->updateAxes();
                plot->updatePlotTitle();
                plot->updateConnectedEditors();

                RiuPlotMainWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
                mainPlotWindow->updateSummaryPlotToolBar();
            }
        }

        m_yPushButtonSelectSummaryAddress = false;
    }

    if (updateTextInPlot)
    {
        updateAllTextInPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        QString curveDataGroupName = "Summary Vector";
        caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroupWithKeyword(curveDataGroupName, "Summary Vector Y");
        curveDataGroup->add(&m_yValuesSummaryGroup);
        curveDataGroup->add(&m_yValuesSelectedVariableDisplayField);
        curveDataGroup->add(&m_plotAxis);
        curveDataGroup->add(&m_yPushButtonSelectSummaryAddress);

        QString curveVarSelectionGroupName = "Vector Selection Filter Y";
        caf::PdmUiGroup* curveVarSelectionGroup = curveDataGroup->addNewGroupWithKeyword("Vector Selection Filter", curveVarSelectionGroupName);
        curveVarSelectionGroup->setCollapsedByDefault(true);
        m_yValuesSummaryFilter->uiOrdering(uiConfigName, *curveVarSelectionGroup);
        curveVarSelectionGroup->add(&m_yValuesUiFilterResultSelection);
    }

    caf::PdmUiGroup* colorsGroup = uiOrdering.addNewGroup("Colors");
    m_colorMode.uiCapability()->setUiReadOnly(!m_yValuesSummaryGroup());
    colorsGroup->add(&m_colorMode);

    if (m_colorMode == SINGLE_COLOR)
    {
        colorsGroup->add(&m_color);
    }
    else if (m_colorMode == BY_ENSEMBLE_PARAM)
    {
        m_ensembleParameter.uiCapability()->setUiReadOnly(!m_yValuesSummaryGroup());
        colorsGroup->add(&m_ensembleParameter);
    }

    {
        caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
        nameGroup->setCollapsedByDefault(true);
        nameGroup->add(&m_isUsingAutoName);
        if (m_isUsingAutoName)
        {
            nameGroup->add(&m_autoGeneratedName);
            m_summaryAddressNameTools->uiOrdering(uiConfigName, *nameGroup);
        }
        else
        {
            nameGroup->add(&m_userDefinedName);
        }
    }

    caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup("Statistics");
    m_statistics->defineUiOrdering(uiConfigName, *statGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (m_colorMode == BY_ENSEMBLE_PARAM)
    {
        uiTreeOrdering.add(m_legendConfig());
    }

    if (uiConfigName != RicSummaryCurveCreator::CONFIGURATION_NAME)
    {
        uiTreeOrdering.add(m_curveFilters);
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveSet::userDescriptionField()
{
    if (m_isUsingAutoName)
    {
        return &m_autoGeneratedName;
    }
    else
    {
        return &m_userDefinedName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsembleCurveSet::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
    if (attrib)
    {
        attrib->m_buttonText = "Vector Selection Dialog";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleCurveSet::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_yValuesSummaryGroup)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimSummaryCaseCollection*> groups = proj->summaryGroups();

        for (RimSummaryCaseCollection* group : groups)
        {
            if(group->isEnsemble()) options.push_back(caf::PdmOptionItemInfo(group->name(), group));
        }

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_colorMode)
    {
        auto singleColorOption = caf::AppEnum<RimEnsembleCurveSet::ColorMode>(RimEnsembleCurveSet::SINGLE_COLOR);
        auto byEnsParamOption = caf::AppEnum<RimEnsembleCurveSet::ColorMode>(RimEnsembleCurveSet::BY_ENSEMBLE_PARAM);

        options.push_back(caf::PdmOptionItemInfo(singleColorOption.uiText(), RimEnsembleCurveSet::SINGLE_COLOR));
        if (!ensembleParameterNames().empty())
        {
            options.push_back(caf::PdmOptionItemInfo(byEnsParamOption.uiText(), RimEnsembleCurveSet::BY_ENSEMBLE_PARAM));
        }
    }
    else if (fieldNeedingOptions == &m_ensembleParameter)
    {
        for (const auto& param : ensembleParameterNames())
        {
            options.push_back(caf::PdmOptionItemInfo(param, param));
        }
    }
    else if (fieldNeedingOptions == &m_yValuesUiFilterResultSelection)
    {
        appendOptionItemsForSummaryAddresses(&options, m_yValuesSummaryGroup(), m_yValuesSummaryFilter());
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::getOptionsForSummaryAddresses(std::map<QString, RifEclipseSummaryAddress>* options,
                                                            RimSummaryCase* summaryCase,
                                                            RimSummaryFilter* summaryFilter)
{
    if (summaryCase)
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        if (reader)
        {
            for (const auto& address : reader->allResultAddresses())
            {
                if (summaryFilter && !summaryFilter->isIncludedByFilter(address)) continue;

                std::string name = address.uiText();
                QString s = QString::fromStdString(name);
                options->insert(std::make_pair(s, address));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Optimization candidate
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                           RimSummaryCaseCollection* summaryCaseGroup,
                                                           RimSummaryFilter* summaryFilter)
{
    if (!summaryCaseGroup) return;

    std::set<RifEclipseSummaryAddress> addrSet;
    for (RimSummaryCase* summaryCase : summaryCaseGroup->allSummaryCases())
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        const std::set<RifEclipseSummaryAddress>& addrs = reader ? reader->allResultAddresses() : std::set<RifEclipseSummaryAddress>();

        for (auto& addr : addrs)
        {
            if (summaryFilter && !summaryFilter->isIncludedByFilter(addr)) continue;
            addrSet.insert(addr);
        }
    }

    for (auto& addr : addrSet)
    {
        std::string name = addr.uiText();
        QString s = QString::fromStdString(name);
        options->push_back(caf::PdmOptionItemInfo(s, QVariant::fromValue(addr)));
    }

    options->push_front(caf::PdmOptionItemInfo(RiaDefines::undefinedResultName(), QVariant::fromValue(RifEclipseSummaryAddress())));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateCurveColors()
{
    if(m_colorMode == BY_ENSEMBLE_PARAM)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
        
        QString parameterName = m_ensembleParameter();

        {
            QString legendTitle;
            if (m_isUsingAutoName)
            {
                legendTitle = m_autoGeneratedName();
            }
            else
            {
                legendTitle += m_userDefinedName();
            }

            legendTitle += "\n";
            legendTitle += parameterName;

            m_legendConfig->setTitle(legendTitle);
        }

        if (group && !parameterName.isEmpty() && !group->allSummaryCases().empty())
        {
            auto ensembleParam = group->ensembleParameter(parameterName);

            if (ensembleParam.isText())
            {
                std::set<QString> categories;

                for (auto value : ensembleParam.values)
                {
                    categories.insert(value.toString());
                }

                std::vector<QString> categoryNames = std::vector<QString>(categories.begin(), categories.end());
                m_legendConfig->setNamedCategories(categoryNames);
                m_legendConfig->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);

                for (auto& curve : m_curves)
                {
                    if (curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS) continue;

                    RimSummaryCase* rimCase = curve->summaryCaseY();
                    QString tValue = rimCase->hasCaseRealizationParameters() ?
                        rimCase->caseRealizationParameters()->parameterValue(parameterName).textValue() :
                        "";
                    double nValue = m_legendConfig->categoryValueFromCategoryName(tValue);
                    if (nValue != DOUBLE_INF)
                    {
                        int iValue = static_cast<int>(nValue);
                        curve->setColor(cvf::Color3f(m_legendConfig->scalarMapper()->mapToColor(iValue)));
                    }
                    else
                    {
                        curve->setColor(RiaColorTables::undefinedCellColor());
                    }
                    curve->updateCurveAppearance();
                }
            }
            else if(ensembleParam.isNumeric())
            {
                double minValue = DOUBLE_INF;
                double maxValue = -DOUBLE_INF;

                for (auto value : ensembleParam.values)
                {
                    double nValue = value.toDouble();
                    if (nValue != DOUBLE_INF)
                    {
                        if (nValue < minValue) minValue = nValue;
                        if (nValue > maxValue) maxValue = nValue;
                    }
                }

                m_legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);

                for (auto& curve : m_curves)
                {
                    if (curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS) continue;

                    RimSummaryCase* rimCase = curve->summaryCaseY();
                    double value = rimCase->hasCaseRealizationParameters() ? 
                        rimCase->caseRealizationParameters()->parameterValue(parameterName).numericValue() :
                        DOUBLE_INF;
                    if(value != DOUBLE_INF) curve->setColor(cvf::Color3f(m_legendConfig->scalarMapper()->mapToColor(value)));
                    else                    curve->setColor(RiaColorTables::undefinedCellColor());
                    curve->updateCurveAppearance();
                }
            }
        }
    }
    else if (m_colorMode == SINGLE_COLOR)
    {
        for (auto& curve : m_curves)
        {
            if (curve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS) continue;

            curve->setColor(m_color);
            curve->updateCurveAppearance();
        }
    }

    RimSummaryPlot* plot;
    firstAncestorOrThisOfType(plot);
    if (plot && plot->qwtPlot())
    {
        if (m_yValuesSummaryGroup() && isCurvesVisible() && m_colorMode == BY_ENSEMBLE_PARAM && m_legendConfig->showLegend())
        {
            plot->qwtPlot()->addOrUpdateEnsembleCurveSetLegend(this);
        }
        else
        {
            plot->qwtPlot()->removeEnsembleCurveSetLegend(this);
        }
        plot->qwtPlot()->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateQwtPlotAxis()
{
    for (RimSummaryCurve* curve : curves())
    {
        curve->updateQwtPlotAxis();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleCurves()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    deleteEnsembleCurves();
    m_qwtPlotCurveForLegendText->detach();
    deleteStatisticsCurves();

    RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
    RimSummaryAddress* addr = m_yValuesCurveVariable();
    if (group && plot && addr->address().category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        if(m_showCurves)
        {
            const auto filteredCases = filterEnsembleCases(group);
            for (auto& sumCase : filteredCases)
            {
                RimSummaryCurve* curve = new RimSummaryCurve();
                curve->setSummaryCaseY(sumCase);
                curve->setSummaryAddressY(addr->address());
                curve->setLeftOrRightAxisY(m_plotAxis());

                addCurve(curve);

                curve->updateCurveVisibility(false);
                curve->loadDataAndUpdate(false);

                if (curve->qwtPlotCurve())
                {
                    curve->qwtPlotCurve()->setItemAttribute(QwtPlotItem::Legend, false);
                }
            }
            m_yValuesSummaryFilter->updateFromAddress(addr->address());

            if (plot->qwtPlot()) m_qwtPlotCurveForLegendText->attach(plot->qwtPlot());
        }

        RimSummaryPlot* plot;
        firstAncestorOrThisOfType(plot);
        if (plot->qwtPlot())
        {
            plot->qwtPlot()->updateLegend();
            plot->qwtPlot()->replot();
            plot->updateAxes();
        }
    }
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateStatisticsCurves(bool calculate = true)
{
    using SAddr = RifEclipseSummaryAddress;

    RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
    RimSummaryAddress* addr = m_yValuesCurveVariable();

    if (m_disableStatisticCurves || !group || addr->address().category() == RifEclipseSummaryAddress::SUMMARY_INVALID) return;

    if (calculate)
    {
        // Calculate
        m_ensembleStatCase->calculate();
    }

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    std::vector<RifEclipseSummaryAddress> addresses;
    if (m_statistics->isActive())
    {
        RifEclipseSummaryAddress dataAddress = m_yValuesCurveVariable->address();

        if (m_statistics->showP10Curve() && m_ensembleStatCase->hasP10Data())
            addresses.push_back(SAddr::ensembleStatisticsAddress(ENSEMBLE_STAT_P10_QUANTITY_NAME, dataAddress.quantityName()));
        if (m_statistics->showP50Curve() && m_ensembleStatCase->hasP50Data())
            addresses.push_back(SAddr::ensembleStatisticsAddress(ENSEMBLE_STAT_P50_QUANTITY_NAME, dataAddress.quantityName()));
        if (m_statistics->showP90Curve() && m_ensembleStatCase->hasP90Data())
            addresses.push_back(SAddr::ensembleStatisticsAddress(ENSEMBLE_STAT_P90_QUANTITY_NAME, dataAddress.quantityName()));
        if (m_statistics->showMeanCurve()) addresses.push_back(SAddr::ensembleStatisticsAddress(ENSEMBLE_STAT_MEAN_QUANTITY_NAME, dataAddress.quantityName()));
    }

    deleteStatisticsCurves();
    for (auto address : addresses)
    {
        auto curve = new RimSummaryCurve();
        curve->setParentQwtPlotNoReplot(plot->qwtPlot());
        m_curves.push_back(curve);
        curve->setColor(m_statistics->color());
        curve->setSymbol(RimPlotCurve::SYMBOL_ELLIPSE);
        curve->setSymbolSkipDinstance(50);
        curve->setSymbolLabel(QString::fromStdString(address.ensembleStatisticsQuantityName()));
        curve->setLineStyle(RimPlotCurve::STYLE_SOLID);
        curve->setSummaryCaseY(m_ensembleStatCase.get());
        curve->setSummaryAddressY(address);
        curve->setZOrder(1000);

        curve->updateCurveVisibility(false);
        curve->loadDataAndUpdate(false);
    }

    if (plot->qwtPlot())
    {
        plot->qwtPlot()->updateLegend();
        plot->qwtPlot()->replot();
        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveSet::clone() const
{
    RimEnsembleCurveSet* copy = dynamic_cast<RimEnsembleCurveSet*>(this->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    copy->m_yValuesSummaryGroup = m_yValuesSummaryGroup();

    // Update summary case references
    for (size_t i = 0; i < m_curves.size(); i++)
    {
        copy->m_curves[i]->setSummaryCaseY(m_curves[i]->summaryCaseY());
    }
    return copy;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::showCurves(bool show)
{
    m_showCurves = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateAllTextInPlot()
{
    updateEnsembleLegendItem();

    RimSummaryPlot* summaryPlot = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(summaryPlot);
    if (summaryPlot->qwtPlot())
    {
        summaryPlot->updatePlotTitle();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEnsembleCurveSet::ensembleParameterNames() const
{
    RimSummaryCaseCollection* group = m_yValuesSummaryGroup;

    std::set<QString> paramSet;
    if (group)
    {
        for (RimSummaryCase* rimCase : group->allSummaryCases())
        {
            if (rimCase->caseRealizationParameters() != nullptr)
            {
                auto ps = rimCase->caseRealizationParameters()->parameters();
                for (auto p : ps) paramSet.insert(p.first);
            }
        }
    }
    return std::vector<QString>(paramSet.begin(), paramSet.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimEnsembleCurveSet::filterEnsembleCases(const RimSummaryCaseCollection* ensemble)
{
    if (!ensemble) return std::vector<RimSummaryCase*>();

    auto sumCases = ensemble->allSummaryCases();

    for (auto& filter : m_curveFilters->filters())
    {
        sumCases = filter->applyFilter(sumCases);
    }

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::disableStatisticCurves()
{
    m_disableStatisticCurves = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP10Data() const
{
    return m_ensembleStatCase->hasP10Data();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP50Data() const
{
    return m_ensembleStatCase->hasP50Data();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSet::hasP90Data() const
{
    return m_ensembleStatCase->hasP90Data();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateEnsembleLegendItem()
{
    m_qwtPlotCurveForLegendText->setTitle(name());

    {
        QwtSymbol* symbol = nullptr;
        
        if (m_colorMode == SINGLE_COLOR)
        {
            symbol = new QwtSymbol(QwtSymbol::HLine);

            QColor curveColor(m_color.value().rByte(), m_color.value().gByte(), m_color.value().bByte());
            QPen curvePen(curveColor);
            curvePen.setWidth(2);

            symbol->setPen(curvePen);
            symbol->setSize(6, 6);
        }
        else if (m_colorMode == BY_ENSEMBLE_PARAM)
        {
            QPixmap p = QPixmap(":/Legend.png");
            
            symbol = new QwtSymbol;
            symbol->setPixmap(p);
            symbol->setSize(8, 8);
        }

        m_qwtPlotCurveForLegendText->setSymbol(symbol);
    }

    bool showLegendItem = isCurvesVisible();
    m_qwtPlotCurveForLegendText->setItemAttribute(QwtPlotItem::Legend, showLegendItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveSet::name() const
{
    QString curveSetName;
    if (m_isUsingAutoName)
    {
        curveSetName = m_autoGeneratedName();
    }
    else
    {
        curveSetName += m_userDefinedName();
    }

    return curveSetName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleCurveSet::createAutoName() const
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);

    QString curveSetName = m_summaryAddressNameTools->curveNameY(m_yValuesCurveVariable->address(), plot->activePlotTitleHelperAllCurves());
    if (curveSetName.isEmpty())
    {
        curveSetName = m_summaryAddressNameTools->curveNameY(m_yValuesCurveVariable->address(), nullptr);
    }

    if (curveSetName.isEmpty())
    {
        curveSetName = "Name Placeholder";
    }

    return curveSetName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::updateLegendMappingMode()
{
    switch (currentEnsembleParameterType())
    {
    case EnsembleParameter::TYPE_TEXT:
        if (m_legendConfig->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER)
            m_legendConfig->setMappingMode(RimRegularLegendConfig::MappingType::CATEGORY_INTEGER);
        break;

    case EnsembleParameter::TYPE_NUMERIC:
        if (m_legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER)
            m_legendConfig->setMappingMode(RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS);
        break;
    }
}
