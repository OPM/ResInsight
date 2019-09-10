#include "RicSwapGridCrossPlotDataSetAxesFeature.h"

#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotDataSet.h"

#include <cafSelectionManager.h>

#include <QAction>

CAF_CMD_SOURCE_INIT( RicSwapGridCrossPlotDataSetAxesFeature, "RicSwapGridCrossPlotDataSetAxesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSwapGridCrossPlotDataSetAxesFeature::isCommandEnabled()
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotDataSet>() )
    {
        return true;
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>() )
    {
        auto plot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();
        if ( !plot->dataSets().empty() ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSwapGridCrossPlotDataSetAxesFeature::onActionTriggered( bool isChecked )
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotDataSet>() )
    {
        auto dataSet = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotDataSet>();
        dataSet->swapAxisProperties( true );
    }
    else if ( caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>() )
    {
        auto plot = caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlot>();
        plot->swapAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSwapGridCrossPlotDataSetAxesFeature::setupActionLook( QAction* actionToSetup )
{
    if ( caf::SelectionManager::instance()->selectedItemOfType<RimGridCrossPlotDataSet>() )
    {
        actionToSetup->setText( "Swap Axis Properties" );
    }
    else
    {
        actionToSetup->setText( "Swap Axis Properties for all Data Sets in Plot" );
    }
    actionToSetup->setIcon( QIcon( ":/Swap.png" ) );
}
