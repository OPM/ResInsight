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

#include "RimSummaryCurveCollection.h"

#include "RiaApplication.h"
#include "RiaSummaryCurveAnalyzer.h"

#include "RifReaderEclipseSummary.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotSourceStepping.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiTreeViewEditor.h"

#include <QKeyEvent>

CAF_PDM_SOURCE_INIT(RimSummaryCurveCollection, "RimSummaryCurveCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::RimSummaryCurveCollection()
{
    CAF_PDM_InitObject("Summary Curves", ":/SummaryCurveFilter16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_curves, "CollectionCurves", "Collection Curves", "", "", "");
    m_curves.uiCapability()->setUiHidden(true);
    m_curves.uiCapability()->setUiTreeChildrenHidden(false);

    CAF_PDM_InitField(&m_showCurves, "IsActive", true, "Show Curves", "", "", "");
    m_showCurves.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_ySourceStepping, "YSourceStepping", "", "", "", "");
    m_ySourceStepping = new RimSummaryPlotSourceStepping;
    m_ySourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::Y_AXIS);
    m_ySourceStepping.uiCapability()->setUiHidden(true);
    m_ySourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    m_ySourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_xSourceStepping, "XSourceStepping", "", "", "", "");
    m_xSourceStepping = new RimSummaryPlotSourceStepping;
    m_xSourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::X_AXIS);
    m_xSourceStepping.uiCapability()->setUiHidden(true);
    m_xSourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    m_xSourceStepping.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_unionSourceStepping, "UnionSourceStepping", "", "", "", "");
    m_unionSourceStepping = new RimSummaryPlotSourceStepping;
    m_unionSourceStepping->setSourceSteppingType(RimSummaryPlotSourceStepping::UNION_X_Y_AXIS);
    m_unionSourceStepping.uiCapability()->setUiHidden(true);
    m_unionSourceStepping.uiCapability()->setUiTreeChildrenHidden(true);
    m_unionSourceStepping.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurveCollection::~RimSummaryCurveCollection()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurveCollection::isCurvesVisible()
{
    return m_showCurves();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::loadDataAndUpdate(bool updateParentPlot)
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->loadDataAndUpdate(false);
    }

    if ( updateParentPlot )
    {
        RimSummaryPlot* parentPlot;
        firstAncestorOrThisOfTypeAsserted(parentPlot);
        if ( parentPlot->qwtPlot() )
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
void RimSummaryCurveCollection::setParentQwtPlotAndReplot(QwtPlot* plot)
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
void RimSummaryCurveCollection::detachQwtCurves()
{
    for (RimSummaryCurve* curve : m_curves)
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RimSummaryCurveCollection::findRimCurveFromQwtCurve(const QwtPlotCurve* qwtCurve) const
{
    for (RimSummaryCurve* rimCurve : m_curves)
    {
        if (rimCurve->qwtPlotCurve() == qwtCurve)
        {
            return rimCurve;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::addCurve(RimSummaryCurve* curve)
{
    if (curve)
    {
        m_curves.push_back(curve);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::deleteCurve(RimSummaryCurve* curve)
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
std::vector<RimSummaryCurve*> RimSummaryCurveCollection::curves() const
{
    return m_curves.childObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::deleteCurvesAssosiatedWithCase(RimSummaryCase* summaryCase)
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
void RimSummaryCurveCollection::deleteAllCurves()
{
    m_curves.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::updateCaseNameHasChanged()
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
void RimSummaryCurveCollection::setCurrentSummaryCurve(RimSummaryCurve* curve)
{
    m_currentSummaryCurve = curve;

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::applyNextIdentifier()
{
    m_ySourceStepping->applyNextIdentifier();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::applyPreviousIdentifier()
{
    m_ySourceStepping->applyPreviousIdentifier();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimSummaryCurveCollection::fieldsToShowInToolbar()
{
    RimSummaryCrossPlot* parentCrossPlot;
    firstAncestorOrThisOfType(parentCrossPlot);

    if (parentCrossPlot)
    {
        return m_unionSourceStepping->fieldsToShowInToolbar();
    }

    return m_ySourceStepping()->fieldsToShowInToolbar();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurveCollection::compileAutoPlotTitle() const
{
    RiaSummaryCurveAnalyzer analyzer;

    std::set<RifEclipseSummaryAddress> addresses;
    for (auto c : m_curves)
    {
        addresses.insert(c->summaryAddressY());

        // TODO : Improve how cross plot curves contribute to title
        // Suggestion : Delegate to RimSummaryPlotSourceStepping to find title
    }

    analyzer.analyzeAdresses(addresses);

    auto quantities = analyzer.quantities();
    auto wellNames = analyzer.wellNames();
    auto wellGroupNames = analyzer.wellGroupNames();
    auto regions = analyzer.regionNumbers();

    QString title;

    if (wellNames.size() == 1)
    {
        title = QString::fromStdString(*(wellNames.begin()));
    }
    else if (wellGroupNames.size() == 1)
    {
        title = QString::fromStdString(*(wellGroupNames.begin()));
    }
    else if (regions.size() == 1)
    {
        title = "Region : " + QString::number(*(regions.begin()));
    }
    else if (quantities.size() == 1)
    {
        title = QString::fromStdString(*(quantities.begin()));
    }

    return title;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::handleKeyPressEvent(QKeyEvent* keyEvent)
{
    if (!keyEvent) return;

    RimSummaryPlotSourceStepping* sourceStepping = nullptr;
    {
        RimSummaryCrossPlot* summaryCrossPlot = nullptr;
        this->firstAncestorOrThisOfType(summaryCrossPlot);
        
        if (summaryCrossPlot)
        {
            sourceStepping = m_unionSourceStepping();
        }
        else
        {
            sourceStepping = m_ySourceStepping();
        }
    }

    if (keyEvent->key() == Qt::Key_PageUp)
    {
        if (keyEvent->modifiers() & Qt::ShiftModifier)
        {
            sourceStepping->applyPrevCase();

            keyEvent->accept();
        }
        else if (keyEvent->modifiers() & Qt::ControlModifier)
        {
            sourceStepping->applyPrevOtherIdentifier();

            keyEvent->accept();
        }
        else
        {
            sourceStepping->applyPrevQuantity();

            keyEvent->accept();
        }
    }
    else if (keyEvent->key() == Qt::Key_PageDown)
    {
        if (keyEvent->modifiers() & Qt::ShiftModifier)
        {
            sourceStepping->applyNextCase();

            keyEvent->accept();
        }
        else if (keyEvent->modifiers() & Qt::ControlModifier)
        {
            sourceStepping->applyNextOtherIdentifier();

            keyEvent->accept();
        }
        else
        {
            sourceStepping->applyNextQuantity();

            keyEvent->accept();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_showCurves)
    {
        loadDataAndUpdate(true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimSummaryCrossPlot* parentCrossPlot;
    firstAncestorOrThisOfType(parentCrossPlot);

    if (parentCrossPlot)
    {
        {
            auto group = uiOrdering.addNewGroup("Y Source Stepping");

            m_ySourceStepping()->uiOrdering(uiConfigName, *group);
        }

        {
            auto group = uiOrdering.addNewGroup("X Source Stepping");

            m_xSourceStepping()->uiOrdering(uiConfigName, *group);
        }

        {
            auto group = uiOrdering.addNewGroup("XY Union Source Stepping");

            m_unionSourceStepping()->uiOrdering(uiConfigName, *group);
        }
    }
    else
    {
        auto group = uiOrdering.addNewGroup("Plot Source Stepping");

        m_ySourceStepping()->uiOrdering(uiConfigName, *group);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryCurveCollection::objectToggleField()
{
    return &m_showCurves;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurveCollection::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTreeViewEditorAttribute* myAttr = dynamic_cast<caf::PdmUiTreeViewEditorAttribute*>(attribute);
    if (myAttr && m_currentSummaryCurve.notNull())
    {
        myAttr->currentObject = m_currentSummaryCurve.p();
    }
}
