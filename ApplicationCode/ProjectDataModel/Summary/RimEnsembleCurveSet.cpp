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

#include "RifReaderEclipseSummary.h"

#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
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
        parentPlot->qwtPlot()->removeEnsembleCurveSetLegend(this);
    }
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

    m_qwtPlotCurveForLegendText->attach(plot);
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
std::vector<RimSummaryCurve*> RimEnsembleCurveSet::visibleCurves() const
{
    std::vector<RimSummaryCurve*> visible;

    for (auto c : m_curves)
    {
        if (c->isCurveVisible())
        {
            visible.push_back(c);
        }
    }

    return visible;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::deleteAllCurves()
{
    m_curves.deleteAllChildObjects();
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
RimEnsembleCurveSet::ColorMode RimEnsembleCurveSet::colorMode() const
{
    return m_colorMode();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet::EnsembleParameterType RimEnsembleCurveSet::currentEnsembleParameterType() const
{
    if (m_colorMode() == BY_ENSEMBLE_PARAM)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
        QString parameterName = m_ensembleParameter();

        if (group && !parameterName.isEmpty() && !group->allSummaryCases().empty())
        {
            bool isTextParameter = group->allSummaryCases().front()->caseRealizationParameters() != nullptr ?
                group->allSummaryCases().front()->caseRealizationParameters()->parameterValue(parameterName).isText() : false;

            return isTextParameter ? TYPE_TEXT : TYPE_NUMERIC;
        }
    }
    return TYPE_NONE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    if (changedField == &m_showCurves)
    {
        loadDataAndUpdate(true);

        updateConnectedEditors();

        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(summaryPlot);
        summaryPlot->updateConnectedEditors();
    }
    else if (changedField == &m_yValuesUiFilterResultSelection)
    {
        m_yValuesCurveVariable->setAddress(m_yValuesUiFilterResultSelection());

        updateAllCurves();
    }
    else if (changedField == &m_yValuesSummaryGroup)
    {
        // Empty address cache
        m_allAddressesCache.clear();
        updateAllCurves();
    }
    else if (changedField == &m_color)
    {
        updateCurveColors();
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
            auto params = ensembleParameters();
            m_ensembleParameter = !params.empty() ? params.front() : "";
        }
        updateCurveColors();
    }
    else if (changedField == &m_plotAxis)
    {
        for (RimSummaryCurve* curve : curves())
        {
            curve->setLeftOrRightAxisY(m_plotAxis());
        }
        updateQwtPlotAxis();
        plot->updateAxes();
    }
    else if (changedField == &m_isUsingAutoName && !m_isUsingAutoName)
    {
        m_userDefinedName = createAutoName();
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
            if (curveSelection.size() > 0)
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


    if (changedField == &m_isUsingAutoName ||
        changedField == &m_userDefinedName ||
        changedField == &m_colorMode ||
        changedField == &m_color)
    {
        updateEnsembleLegendItem();

        RimSummaryPlot* summaryPlot = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(summaryPlot);
        if (summaryPlot->qwtPlot())
        {
            summaryPlot->qwtPlot()->updateLegend();
        }
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
        if (!ensembleParameters().empty())
        {
            options.push_back(caf::PdmOptionItemInfo(byEnsParamOption.uiText(), RimEnsembleCurveSet::BY_ENSEMBLE_PARAM));
        }
    }
    else if (fieldNeedingOptions == &m_ensembleParameter)
    {
        for (auto param : ensembleParameters())
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
            const std::vector<RifEclipseSummaryAddress> allAddresses = reader->allResultAddresses();

            for (auto& address : allAddresses)
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

    if (m_allAddressesCache.empty())
    {
        for (RimSummaryCase* summaryCase : summaryCaseGroup->allSummaryCases())
        {
            RifSummaryReaderInterface* reader = summaryCase->summaryReader();
            const std::vector<RifEclipseSummaryAddress> addrs = reader ? reader->allResultAddresses() : std::vector<RifEclipseSummaryAddress>();
            m_allAddressesCache.insert(addrs.begin(), addrs.end());
        }
    }

    for (auto& address : m_allAddressesCache)
    {
        if (summaryFilter && !summaryFilter->isIncludedByFilter(address)) continue;

        std::string name = address.uiText();
        QString s = QString::fromStdString(name);
        options->push_back(caf::PdmOptionItemInfo(s, QVariant::fromValue(address)));
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
            bool isTextParameter = group->allSummaryCases().front()->caseRealizationParameters() != nullptr ?
                group->allSummaryCases().front()->caseRealizationParameters()->parameterValue(parameterName).isText() : false;

            if (isTextParameter)
            {
                std::set<QString> categories;

                for (RimSummaryCase* rimCase : group->allSummaryCases())
                {
                    if (rimCase->caseRealizationParameters() != nullptr)
                    {
                        RigCaseRealizationParameters::Value value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                        if (value.isText())
                        {
                            categories.insert(value.textValue());
                        }
                    }
                }

                std::vector<QString> categoryNames = std::vector<QString>(categories.begin(), categories.end());
                m_legendConfig->setNamedCategories(categoryNames);
                m_legendConfig->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);

                for (auto& curve : m_curves)
                {
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
            else
            {
                double minValue = DOUBLE_INF;
                double maxValue = -DOUBLE_INF;

                for (RimSummaryCase* rimCase : group->allSummaryCases())
                {
                    if (rimCase->caseRealizationParameters() != nullptr)
                    {
                        RigCaseRealizationParameters::Value value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                        if (value.isNumeric())
                        {
                            double nValue = value.numericValue();
                            if (nValue != DOUBLE_INF)
                            {
                                if (nValue < minValue) minValue = nValue;
                                if (nValue > maxValue) maxValue = nValue;
                            }
                        }
                    }
                }

                m_legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);

                for (auto& curve : m_curves)
                {
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
void RimEnsembleCurveSet::updateAllCurves()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    deleteAllCurves();

    RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
    RimSummaryAddress* addr = m_yValuesCurveVariable();
    if (group && plot && addr->address().category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
    {
        for (auto& sumCase : group->allSummaryCases())
        {
            RimSummaryCurve* curve = new RimSummaryCurve();
            curve->setSummaryCaseY(sumCase);
            curve->setSummaryAddressY(addr->address());

            addCurve(curve);

            curve->updateCurveVisibility(true);
            curve->loadDataAndUpdate(true);

            if (curve->qwtPlotCurve())
            {
                curve->qwtPlotCurve()->setItemAttribute(QwtPlotItem::Legend, false);
            }
        }
        m_yValuesSummaryFilter->updateFromAddress(addr->address());

        RimSummaryPlot* plot;
        firstAncestorOrThisOfType(plot);
        if (plot->qwtPlot())
        {
            plot->qwtPlot()->replot();
            plot->updateAxes();
        }
    }
    updateCurveColors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RimEnsembleCurveSet::clone() const
{
    RimEnsembleCurveSet* copy = dynamic_cast<RimEnsembleCurveSet*>(this->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    copy->m_yValuesSummaryGroup = m_yValuesSummaryGroup();
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
        }
        else if (m_colorMode == BY_ENSEMBLE_PARAM)
        {
            symbol = new QwtSymbol(QwtSymbol::Star1);
        }

        symbol->setSize(6, 6);
        m_qwtPlotCurveForLegendText->setSymbol(symbol);
    }

    bool showLegendItem = isCurvesVisible();
    m_qwtPlotCurveForLegendText->setItemAttribute(QwtPlotItem::Legend, showLegendItem);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEnsembleCurveSet::ensembleParameters() const
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

    QString curveSetName = m_summaryAddressNameTools->curveNameY(m_yValuesCurveVariable->address(), plot->activePlotTitleHelper());
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
    case TYPE_TEXT:
        if (m_legendConfig->mappingMode() != RimRegularLegendConfig::MappingType::CATEGORY_INTEGER)
            m_legendConfig->setMappingMode(RimRegularLegendConfig::MappingType::CATEGORY_INTEGER);
        break;

    case TYPE_NUMERIC:
        if (m_legendConfig->mappingMode() == RimRegularLegendConfig::MappingType::CATEGORY_INTEGER)
            m_legendConfig->setMappingMode(RimRegularLegendConfig::MappingType::LINEAR_CONTINUOUS);
        break;
    }
}
