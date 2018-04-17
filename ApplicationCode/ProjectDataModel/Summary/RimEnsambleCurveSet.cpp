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
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotSourceStepping.h"
#include "RimSummaryFilter.h"
#include "RimSummaryAddress.h"
#include "RimEnsambleCurveSetCollection.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiTreeViewEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmObject.h"
#include "cafPdmUiPushButtonEditor.h"

#include "cvfScalarMapperContinuousLinear.h"

#include <QTextStream>
#include <QKeyEvent>


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

    //CAF_PDM_InitFieldNoDefault(&m_ySourceStepping, "YSourceStepping", "", "", "", "");
    //m_ySourceStepping = new RimSummaryPlotSourceStepping;
    //m_ySourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::Y_AXIS);
    //m_ySourceStepping.uiCapability()->setUiHidden(true);
    //m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    //m_ySourceStepping.xmlCapability()->disableIO();

    //CAF_PDM_InitFieldNoDefault(&m_xSourceStepping, "XSourceStepping", "", "", "", "");
    //m_xSourceStepping = new RimSummaryPlotSourceStepping;
    //m_xSourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::X_AXIS);
    //m_xSourceStepping.uiCapability()->setUiHidden(true);
    //m_xSourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    //m_xSourceStepping.xmlCapability()->disableIO();

    //CAF_PDM_InitFieldNoDefault(&m_unionSourceStepping, "UnionSourceStepping", "", "", "", "");
    //m_unionSourceStepping = new RimSummaryPlotSourceStepping;
    //m_unionSourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::UNION_X_Y_AXIS);
    //m_unionSourceStepping.uiCapability()->setUiHidden(true);
    //m_unionSourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    //m_unionSourceStepping.xmlCapability()->disableIO();

    // Y Values

    CAF_PDM_InitFieldNoDefault(&m_yValuesSummaryGroup, "SummaryGroup", "Group", "", "", "");
    m_yValuesSummaryGroup.uiCapability()->setUiTreeChildrenHidden(true);
    m_yValuesSummaryGroup.uiCapability()->setAutoAddingOptionFromValue(false);

    CAF_PDM_InitFieldNoDefault(&m_yValuesSelectedVariableDisplayField, "SelectedVariableDisplayVar", "Vector", "", "", "");
    m_yValuesSelectedVariableDisplayField.xmlCapability()->disableIO();
    //m_yValuesSelectedVariableDisplayField.uiCapability()->setUiReadOnly(true);

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

    CAF_PDM_InitField(&m_ensambleParameter, "EnsambleParameter", QString(""), "Ensamble Parameter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendConfig", "", "", "", "");
    m_legendConfig = new RimLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEnsambleCurveSet::~RimEnsambleCurveSet()
{
    m_curves.deleteAllChildObjects();
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
void RimEnsambleCurveSet::loadDataAndUpdate(bool updateParentPlot)
{
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
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::setParentQwtPlotAndReplot(QwtPlot* plot)
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->setParentQwtPlotNoReplot(plot);
    }

    if (plot) plot->replot();
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
RimSummaryCurve* RimEnsambleCurveSet::findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for (RimSummaryCurve* rimCurve : m_curves)
    {
        if (rimCurve->qwtPlotCurve() == qwtCurve)
        {
            return rimCurve;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::addCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
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
void RimEnsambleCurveSet::deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase)
{
    std::vector<RimSummaryCurve*> summaryCurvesToDelete;

    for (RimSummaryCurve* summaryCurve : m_curves)
    {
        if (!summaryCurve) continue;
        if (!summaryCurve->summaryCaseY()) continue;

        if (summaryCurve->summaryCaseY() == summaryCase)
        {
            summaryCurvesToDelete.push_back(summaryCurve);
        }
    }
    for (RimSummaryCurve* summaryCurve : summaryCurvesToDelete)
    {
        m_curves.removeChildObject(summaryCurve);
        delete summaryCurve;
    }

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
void RimEnsambleCurveSet::updateCaseNameHasChanged()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->updateCurveNameNoLegendUpdate();
        curve->updateConnectedEditors();
    }

    RimSummaryPlot* parentPlot;
    firstAncestorOrThisOfTypeAsserted(parentPlot);
    if (parentPlot->qwtPlot()) parentPlot->qwtPlot()->updateLegend();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::setCurrentSummaryCurve(RimSummaryCurve* curve)
{
    m_currentSummaryCurve = curve;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
//std::vector<caf::PdmFieldHandle*> RimEnsambleCurveSet::fieldsToShowInToolbar()
//{
//    RimSummaryCrossPlot* parentCrossPlot;
//    firstAncestorOrThisOfType(parentCrossPlot);
//
//    if (parentCrossPlot)
//    {
//        return m_unionSourceStepping->fieldsToShowInToolbar();
//    }
//
//    return m_ySourceStepping()->fieldsToShowInToolbar();
//}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::handleKeyPressEvent(QKeyEvent* keyEvent)
{
    //if (!keyEvent) return;

    //RimSummaryPlotSourceStepping* sourceStepping = nullptr;
    //{
    //    RimSummaryCrossPlot* summaryCrossPlot = nullptr;
    //    this->firstAncestorOrThisOfType(summaryCrossPlot);

    //    if (summaryCrossPlot)
    //    {
    //        sourceStepping = m_unionSourceStepping();
    //    }
    //    else
    //    {
    //        sourceStepping = m_ySourceStepping();
    //    }
    //}

    //if (keyEvent->key() == Qt::Key_PageUp)
    //{
    //    if (keyEvent->modifiers() & Qt::ShiftModifier)
    //    {
    //        sourceStepping->applyPrevCase();

    //        keyEvent->accept();
    //    }
    //    else if (keyEvent->modifiers() & Qt::ControlModifier)
    //    {
    //        sourceStepping->applyPrevOtherIdentifier();

    //        keyEvent->accept();
    //    }
    //    else
    //    {
    //        sourceStepping->applyPrevQuantity();

    //        keyEvent->accept();
    //    }
    //}
    //else if (keyEvent->key() == Qt::Key_PageDown)
    //{
    //    if (keyEvent->modifiers() & Qt::ShiftModifier)
    //    {
    //        sourceStepping->applyNextCase();

    //        keyEvent->accept();
    //    }
    //    else if (keyEvent->modifiers() & Qt::ControlModifier)
    //    {
    //        sourceStepping->applyNextOtherIdentifier();

    //        keyEvent->accept();
    //    }
    //    else
    //    {
    //        sourceStepping->applyNextQuantity();

    //        keyEvent->accept();
    //    }
    //}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEnsambleCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurves)
    {
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_yValuesSummaryGroup || changedField == &m_yValuesSelectedVariableDisplayField)
    {
        deleteAllCurves();

        RimSummaryPlot* plot;
        firstAncestorOrThisOfType(plot);
        if (plot) plot->loadDataAndUpdate();

        RimSummaryCaseCollection* group = m_yValuesSummaryGroup();
        RifEclipseSummaryAddress addr = m_yValuesSelectedVariableDisplayField();
        if (group && plot && addr.category() != RifEclipseSummaryAddress::SUMMARY_INVALID)
        {
            for (auto& sumCase : group->allSummaryCases())
            {
                RimSummaryCurve* curve = new RimSummaryCurve();
                curve->setSummaryCaseY(sumCase);
                curve->setSummaryAddressY(addr);

                addCurve(curve);
                curve->setParentQwtPlotNoReplot(plot->qwtPlot());

                curve->updateCurveVisibility(true);
                curve->loadDataAndUpdate(true);
            }

            RimSummaryPlot* plot;
            firstAncestorOrThisOfType(plot);
            if (plot && plot->qwtPlot())
            {
                plot->qwtPlot()->replot();
                plot->updateAxes();
            }
        }
    }
    else if (changedField == &m_ensambleParameter)
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

            RimSummaryPlot* plot;
            firstAncestorOrThisOfType(plot);
            if (plot && plot->qwtPlot()) plot->qwtPlot()->replot();
        }
    }
    else if (changedField == &m_color)
    {
        for (auto& curve : m_curves)
        {
            curve->setColor(m_color);
            curve->updateCurveAppearance();
        }

        RimSummaryPlot* plot;
        firstAncestorOrThisOfType(plot);
        if (plot && plot->qwtPlot()) plot->qwtPlot()->replot();
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

    //RimSummaryCrossPlot* parentCrossPlot;
    //firstAncestorOrThisOfType(parentCrossPlot);

    //if (parentCrossPlot)
    //{
    //    {
    //        auto group = uiOrdering.addNewGroup("Y Source Stepping");

    //        m_ySourceStepping()->uiOrdering(uiConfigName, *group);
    //    }

    //    {
    //        auto group = uiOrdering.addNewGroup("X Source Stepping");

    //        m_xSourceStepping()->uiOrdering(uiConfigName, *group);
    //    }

    //    {
    //        auto group = uiOrdering.addNewGroup("XY Union Source Stepping");

    //        m_unionSourceStepping()->uiOrdering(uiConfigName, *group);
    //    }
    //}
    //else
    //{
    //    auto group = uiOrdering.addNewGroup("Plot Source Stepping");

    //    m_ySourceStepping()->uiOrdering(uiConfigName, *group);
    //}
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
void RimEnsambleCurveSet::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>(attribute);
    if (myAttr && m_currentSummaryCurve.notNull())
    {
        myAttr->currentObject = m_currentSummaryCurve.p();
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

        //options.push_back(caf::PdmOptionItemInfo("None", ""));

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
    else if (fieldNeedingOptions == &m_yValuesSelectedVariableDisplayField)
    {
        RimSummaryCaseCollection* group = m_yValuesSummaryGroup;
        std::map<QString, RifEclipseSummaryAddress> allOpts;

        if (group)
        {
            for (auto& sumCase : group->allSummaryCases())
            {
                std::map<QString, RifEclipseSummaryAddress> opts;
                RimSummaryFilter filter;
                getOptionsForSummaryAddresses(&opts, sumCase, &filter);

                for (auto& opt : opts) allOpts.insert(opt);
            }
        }

        for (const auto& opt : allOpts) options.push_back(caf::PdmOptionItemInfo(opt.first, QVariant::fromValue(opt.second)));
        options.push_front(caf::PdmOptionItemInfo(RiaDefines::undefinedResultName(), QVariant::fromValue(RifEclipseSummaryAddress())));
    }

    //else if (fieldNeedingOptions == &m_yValuesUiFilterResultSelection)
    //{
    //    appendOptionItemsForSummaryAddresses(&options, m_yValuesSummaryCase(), m_yValuesSummaryFilter());
    //}

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
