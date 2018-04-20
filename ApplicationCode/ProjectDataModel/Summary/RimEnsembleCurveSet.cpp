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

#include "RifReaderEclipseSummary.h"

#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryFilter.h"
#include "RimSummaryAddress.h"
#include "RimEnsembleCurveSetCollection.h"

#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfScalarMapper.h"


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
    CAF_PDM_InitObject("Ensemble Curve Set", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curves, "EnsembleCurveSet", "Ensemble Curve Set", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiTreeChildrenHidden(false);

    CAF_PDM_InitField(&m_showCurves, "IsActive", true, "Show Curves", "", "", "");
    m_showCurves.uiCapability()->setUiHidden(true);

    // Y Values
    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryGroup, "SummaryGroup", "Group", "", "", "");
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
    m_yPushButtonSelectSummaryAddress.uiCapability()->setUiReadOnly(true);
    m_yPushButtonSelectSummaryAddress.uiCapability()->setUiHidden(true);

    m_yValuesCurveVariable = new RimSummaryAddress;

    CAF_PDM_InitField(&m_colorMode, "ColorMode", caf::AppEnum<ColorMode>(SINGLE_COLOR), "Coloring Mode", "", "", "");

    CAF_PDM_InitField(&m_color, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    CAF_PDM_InitField(&m_ensembleParameter, "EnsembleParameter", QString(""), "Ensemble Parameter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendConfig", "", "", "", "");
    m_legendConfig = new RimRegularLegendConfig();
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

    m_yValuesSummaryFilter->updateFromAddress(m_yValuesCurveVariable->address());

    for (RimSummaryCurve* curve : m_curves)
    {
        curve->loadDataAndUpdate(false);
        curve->updateQwtPlotAxis();
    }

    if (updateParentPlot)
    {
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted(parentPlot);
        if (parentPlot->qwtPlot())
        {
            parentPlot->qwtPlot()->updateLegend();
            parentPlot->updateAxes();
            parentPlot->updateZoomInQwt();

            if (m_showCurves() && m_colorMode() == BY_ENSEMBLE_PARAM)
            {
                parentPlot->qwtPlot()->addOrUpdateEnsembleCurveSetLegend(this);
            }
            else
            {
                parentPlot->qwtPlot()->removeEnsembleCurveSetLegend(this);
            }
        }
    }

    updateCurveColors();
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
void RimEnsembleCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);
    CVF_ASSERT(plot);

    if (changedField == &m_showCurves)
    {
        loadDataAndUpdate(true);
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
    else if (changedField == &m_ensembleParameter ||
             changedField == &m_color)
    {
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
            options.push_back(caf::PdmOptionItemInfo(group->name(), group));
        }

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
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
        if (group && !parameterName.isEmpty())
        {
            double minValue = std::numeric_limits<double>::infinity();
            double maxValue = -std::numeric_limits<double>::infinity();

            for (RimSummaryCase* rimCase : group->allSummaryCases())
            {
                if (!rimCase->caseRealizationParameters().isNull())
                {
                    double value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                    if (value != std::numeric_limits<double>::infinity())
                    {
                        if (value < minValue) minValue = value;
                        if (value > maxValue) maxValue = value;
                    }
                }
            }

            m_legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);

            for (auto& curve : m_curves)
            {
                RimSummaryCase* rimCase = curve->summaryCaseY();
                double value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                curve->setColor(cvf::Color3f(m_legendConfig->scalarMapper()->mapToColor(value)));
                curve->updateCurveAppearance();
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
        if (m_colorMode == BY_ENSEMBLE_PARAM)
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

    plot->loadDataAndUpdate();

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
        }

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
std::vector<QString> RimEnsembleCurveSet::ensembleParameters() const
{
    RimSummaryCaseCollection* group = m_yValuesSummaryGroup;

    std::set<QString> paramSet;
    if (group)
    {
        for (RimSummaryCase* rimCase : group->allSummaryCases())
        {
            if (!rimCase->caseRealizationParameters().isNull())
            {
                auto ps = rimCase->caseRealizationParameters()->parameters();
                for (auto p : ps) paramSet.insert(p.first);
            }
        }
    }
    return std::vector<QString>(paramSet.begin(), paramSet.end());
}