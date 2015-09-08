/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewWellLogPlotCurveFeature.h"

#include "RimWellLogPlotTrace.h"
#include "RimWellLogExtractionCurve.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafPdmFieldCvfColor.h"

#include <QAction>
#include <QColor>

#include <vector>

static const int RI_LOGPLOT_CURVECOLORSCOUNT = 15;
static const int RI_LOGPLOT_CURVECOLORS[] =
{
    Qt::blue,
    Qt::red,
    Qt::green,
    Qt::yellow,
    Qt::magenta,
    Qt::cyan,
    Qt::gray,
    Qt::darkBlue,
    Qt::darkRed,
    Qt::darkGreen,
    Qt::darkYellow,
    Qt::darkMagenta,
    Qt::darkCyan,
    Qt::darkGray,
    Qt::black
};

//--------------------------------------------------------------------------------------------------
/// Pick default curve color from an index based palette
//--------------------------------------------------------------------------------------------------
static QColor sg_curveColorFromIndex(size_t curveIndex)
{
    return QColor(Qt::GlobalColor(RI_LOGPLOT_CURVECOLORS[curveIndex % RI_LOGPLOT_CURVECOLORSCOUNT]));
}


CAF_CMD_SOURCE_INIT(RicNewWellLogPlotCurveFeature, "RicNewWellLogPlotCurveFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogPlotCurveFeature::isCommandEnabled()
{
    return selectedWellLogPlotTrace() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotCurveFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotTrace* wellLogPlotTrace = selectedWellLogPlotTrace();
    if (wellLogPlotTrace)
    {
        addCurve(wellLogPlotTrace);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotCurveFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Curve");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotTrace* RicNewWellLogPlotCurveFeature::selectedWellLogPlotTrace()
{
    std::vector<RimWellLogPlotTrace*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    return selection.size() > 0 ? selection[0] : NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotCurveFeature::addCurve(RimWellLogPlotTrace* plotTrace)
{
    CVF_ASSERT(plotTrace);

    size_t curveIndex = plotTrace->curveCount();

    RimWellLogPlotCurve* curve = new RimWellLogExtractionCurve();
    plotTrace->addCurve(curve);

    QColor curveColorQt = sg_curveColorFromIndex(curveIndex);
    cvf::Color3f curveColor(curveColorQt.redF(), curveColorQt.greenF(), curveColorQt.blueF());
    curve->setColor(curveColor);

    curve->setDescription(QString("Curve %1").arg(plotTrace->curveCount()));

    plotTrace->updateConnectedEditors();
    RiuMainWindow::instance()->setCurrentObjectInTreeView(curve);
}
