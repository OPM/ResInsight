#include "RicSwapGridCrossPlotCurveSetAxesFeature.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurveSet.h"

#include <cafSelectionManager.h>

#include <QAction>

CAF_CMD_SOURCE_INIT(RicSwapGridCrossPlotCurveSetAxesFeature, "RicSwapGridCrossPlotCurveSetAxesFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSwapGridCrossPlotCurveSetAxesFeature::isCommandEnabled()
{
    if (caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotCurveSet>())
    {
        return true;
    }
    else if (caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>())
    {
        auto plot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();
        if (!plot->curveSets().empty())
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSwapGridCrossPlotCurveSetAxesFeature::onActionTriggered(bool isChecked)
{
    if (caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotCurveSet>())
    {
        auto curveSet = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotCurveSet>();
        curveSet->swapAxisProperties(true);
    }
    else if (caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>())
    {
        auto plot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();
        plot->swapAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSwapGridCrossPlotCurveSetAxesFeature::setupActionLook(QAction* actionToSetup)
{
    if (caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotCurveSet>())
    {
        actionToSetup->setText("Swap Axis Properties");
    }
    else
    {
        actionToSetup->setText("Swap Axis Properties for all Data Sets in Plot");
    }
    actionToSetup->setIcon(QIcon(":/Swap.png"));
}
