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

#include "RimEnsambleCurveSet.h"

#include "RiaApplication.h"

#include "RifReaderEclipseSummary.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryFilter.h"
#include "RimSummaryAddress.h"
#include "RimEnsambleCurveSetCollection.h"

#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfScalarMapperContinuousLinear.h"


namespace caf
{
    template<>
    void AppEnum< RimEnsambleCurveSet::ColorMode >::setUp()
    {
        addItem(RimEnsambleCurveSet::SINGLE_COLOR, "SINGLE_COLOR", "Single Color");
        addItem(RimEnsambleCurveSet::BY_ENSAMBLE_PARAM, "BY_ENSAMBLE_PARAM", "By Ensamble Parameter");
        setDefault(RimEnsambleCurveSet::SINGLE_COLOR);
    }
}


CAF_PDM_SOURCE_INIT(RimEnsambleCurveSet, "RimEnsambleCurveSet");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsambleCurveSet::RimEnsambleCurveSet()
{
    CAF_PDM_InitObject("Ensamble Curve Set", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curves, "EnsambleCurveSet", "Ensamble Curve Set", "", "", "");
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

    m_yValuesCurveVariable = new RimSummaryAddress;

    CAF_PDM_InitField(&m_colorMode, "ColorMode", caf::AppEnum<ColorMode>(SINGLE_COLOR), "Coloring Mode", "", "", "");

    CAF_PDM_InitField(&m_color, "Color", cvf::Color3f(cvf::Color3::BLACK), "Color", "", "", "");

    CAF_PDM_InitField(&m_ensambleParameter, "EnsambleParameter", QString(""), "Ensamble Parameter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis, "PlotAxis", "Axis", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendConfig", "", "", "", "");
    m_legendConfig = new RimRegularLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsambleCurveSet::~RimEnsambleCurveSet()
{
    m_curves.deleteAllChildObjects();

    RimSummaryPlot* parentPlot;
    firstAncestorOrThisOfTypeAsserted(parentPlot);
    if (parentPlot->qwtPlot())
    {
        parentPlot->qwtPlot()->removeEnsambleCurveSetLegend(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEnsambleCurveSet::isCurvesVisible()
{
    RimEnsambleCurveSetCollection* coll = nullptr;
    firstAncestorOrThisOfType(coll);
    return m_showCurves() && (coll ? coll->isCurveSetsVisible() : true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::setColor(cvf::Color3f color)
{
    m_color = color;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::loadDataAndUpdate(bool updateParentPlot)
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

            if (m_showCurves() && m_colorMode() == BY_ENSAMBLE_PARAM)
            {
                parentPlot->qwtPlot()->addOrUpdateEnsambleCurveSetLegend(this);
            }
            else
            {
                parentPlot->qwtPlot()->removeEnsambleCurveSetLegend(this);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::setParentQwtPlotNoReplot(QwtPlot* plot)
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->setParentQwtPlotNoReplot(plot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::detachQwtCurves()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::addCurve(RimSummaryCurve* curve)
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
void RimEnsambleCurveSet::deleteCurve(RimSummaryCurve* curve)
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
std::vector<RimSummaryCurve*> RimEnsambleCurveSet::curves() const
{
    return m_curves.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RimEnsambleCurveSet::visibleCurves() const
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
void RimEnsambleCurveSet::deleteAllCurves()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEnsambleCurveSet::legendConfig()
{
    return m_legendConfig;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
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
        updateAllCurves();
    }
    else if (changedField == &m_ensambleParameter ||
             changedField == &m_color ||
             changedField == &m_colorMode)
    {
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
    else if (changedField == &m_yPushButtonSelectSummaryAddress)
    {
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
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
    colorsGroup->add(&m_colorMode);

    if (m_colorMode == SINGLE_COLOR)
    {
        colorsGroup->add(&m_color);
    }
    else if (m_colorMode == BY_ENSAMBLE_PARAM)
    {
        colorsGroup->add(&m_ensambleParameter);
    }
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(m_legendConfig());
    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimEnsambleCurveSet::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
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
QList<caf::PdmOptionItemInfo> RimEnsambleCurveSet::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
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
    else if (fieldNeedingOptions == &m_ensambleParameter)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup;

        if (group)
        {
            std::set<QString> paramSet;
            for (RimSummaryCase* rimCase : group->allSummaryCases())
            {
                if (!rimCase->caseRealizationParameters().isNull())
                {
                    auto ps = rimCase->caseRealizationParameters()->parameters();
                    for (auto p : ps) paramSet.insert(p.first);
                }
            }

            for (auto param : paramSet)
            {
                options.push_back(caf::PdmOptionItemInfo(param, param));
            }
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
void RimEnsambleCurveSet::getOptionsForSummaryAddresses(std::map<QString, RifEclipseSummaryAddress>* options,
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
void RimEnsambleCurveSet::appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                           RimSummaryCaseCollection* summaryCaseGroup,
                                                           RimSummaryFilter* summaryFilter)
{
    std::set<RifEclipseSummaryAddress> allAddresses;
    
    for (RimSummaryCase* summaryCase : summaryCaseGroup->allSummaryCases())
    {
        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        const std::vector<RifEclipseSummaryAddress> addrs = reader ? reader->allResultAddresses() : std::vector<RifEclipseSummaryAddress>();
        allAddresses.insert(addrs.begin(), addrs.end());
    }

    for (auto& address : allAddresses)
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
void RimEnsambleCurveSet::updateCurveColors()
{
    if(m_colorMode == BY_ENSAMBLE_PARAM)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
        QString parameterName = m_ensambleParameter();
        if (group && !parameterName.isEmpty())
        {
            double minValue = HUGE_VAL;
            double maxValue = -HUGE_VAL;

            for (RimSummaryCase* rimCase : group->allSummaryCases())
            {
                if (!rimCase->caseRealizationParameters().isNull())
                {
                    double value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                    if (value != HUGE_VAL)
                    {
                        if (value < minValue) minValue = value;
                        if (value > maxValue) maxValue = value;
                    }
                }
            }

            cvf::ScalarMapperContinuousLinear colorMapper;
            colorMapper.setRange(minValue, maxValue);

            for (auto& curve : m_curves)
            {
                RimSummaryCase* rimCase = curve->summaryCaseY();
                double value = rimCase->caseRealizationParameters()->parameterValue(parameterName);
                curve->setColor(cvf::Color3f(colorMapper.mapToColor(value)));
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
    if (plot && plot->qwtPlot()) plot->qwtPlot()->replot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::updateQwtPlotAxis()
{
    for (RimSummaryCurve* curve : curves())
    {
        curve->updateQwtPlotAxis();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::updateAllCurves()
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
