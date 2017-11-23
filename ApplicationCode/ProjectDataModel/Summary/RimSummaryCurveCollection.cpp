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

#include "RifReaderEclipseSummary.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurvesModifier.h"
#include "RimSummaryPlot.h"

#include "RiuLineSegmentQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiTreeViewEditor.h"

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

    CAF_PDM_InitFieldNoDefault(&m_curvesModifier, "CurvesModifier", "Curves Modifier", "", "", "");
    m_curvesModifier = new RimSummaryCurvesModifier;
    m_curvesModifier.uiCapability()->setUiHidden(true);
    m_curvesModifier.uiCapability()->setUiTreeChildrenHidden(true);
    m_curvesModifier.xmlCapability()->disableIO();
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
std::vector<RimSummaryCurve*> RimSummaryCurveCollection::curves()
{
    std::vector<RimSummaryCurve*> myCurves;

    for (RimSummaryCurve* curve : m_curves)
    {
        myCurves.push_back(curve);
    }

    return myCurves;
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
    auto group = uiOrdering.addNewGroup("Curve Modifier");

    m_curvesModifier()->uiOrdering(uiConfigName, *group);
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
