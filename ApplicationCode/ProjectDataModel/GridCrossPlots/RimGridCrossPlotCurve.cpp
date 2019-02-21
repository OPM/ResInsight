/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RimGridCrossPlotCurve.h"

#include "RiaColorTables.h"

#include "RigCaseCellResultCalculator.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlot.h"
#include "RimTools.h"

#include "cafPdmUiComboBoxEditor.h"

#include <QDebug>
#include <QPointF>
#include <QVector>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <random>

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurve, "GridCrossPlotCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::determineColorAndSymbol(int curveSetIndex, int categoryIndex, bool contrastColors)
{
    const caf::ColorTable& colors = RiaColorTables::contrastCategoryPaletteColors();
    int colorIndex = categoryIndex + curveSetIndex; // Offset cycle for each curve set
    setColor(colors.cycledColor3f(colorIndex));
    int numColors = (int) colors.size();

    // Retain same symbol until we've gone full cycle in colors
    int symbolIndex = categoryIndex / numColors;
       
    RiuQwtSymbol::PointSymbolEnum symbol = RiuQwtSymbol::cycledSymbolStyle(curveSetIndex, symbolIndex);
    setSymbol(symbol); 
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurve::RimGridCrossPlotCurve()
{
    CAF_PDM_InitObject("Cross Plot Points", ":/WellLogCurve16x16.png", "", "");
   
    setLineStyle(RiuQwtPlotCurve::STYLE_NONE);
    setSymbolSize(6);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::setSamples(const QVector<QPointF>& samples)
{
    m_qwtPlotCurve->setSamples(samples);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::updateZoomInParentPlot()
{
    RimGridCrossPlot* plot;
    this->firstAncestorOrThisOfTypeAsserted(plot);
    plot->calculateZoomRangeAndUpdateQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::updateLegendsInPlot() 
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurve::createCurveAutoName()
{
    return m_customCurveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    if (updateParentPlot)
    {
        RimGridCrossPlot* crossPlot;
        firstAncestorOrThisOfTypeAsserted(crossPlot);
        crossPlot->reattachCurvesToQwtAndReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCrossPlotCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                           bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    return options;
}
