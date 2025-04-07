/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuTimeStepChangedHandler.h"

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimCase.h"

#include "Riu3dSelectionManager.h"
#include "RiuMainWindow.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuPvtPlotPanel.h"
#include "RiuPvtPlotUpdater.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuResultQwtPlot.h"

#include <QDateTime>

//==================================================================================================
///
/// \class RiuTimeStepChangedHandler
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTimeStepChangedHandler::RiuTimeStepChangedHandler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuTimeStepChangedHandler* RiuTimeStepChangedHandler::instance()
{
    static RiuTimeStepChangedHandler* singletonInstance = new RiuTimeStepChangedHandler;
    return singletonInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuTimeStepChangedHandler::handleTimeStepChanged( Rim3dView* changedView ) const
{
    if ( !changedView ) return;
    if ( !RiaGuiApplication::isRunning() ) return;

    RiuRelativePermeabilityPlotUpdater* relPermPlotUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermPlotUpdater->updateOnTimeStepChanged( changedView );

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnTimeStepChanged( changedView );

    if ( changedView->ownerCase() )
    {
        int        ts    = changedView->currentTimeStep();
        const auto dates = changedView->ownerCase()->timeStepDates();
        if ( dates.size() > (size_t)ts )
        {
            auto resultPlot = RiuMainWindow::instance()->resultPlot();
            resultPlot->showTimeStep( dates[ts] );
            resultPlot->replot();
        }
    }

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if ( mohrsCirclePlot ) mohrsCirclePlot->updateOnTimeStepChanged( changedView );

    std::vector<RiuSelectionItem*> selectedItems;
    Riu3dSelectionManager::instance()->selectedItems( selectedItems );

    for ( RiuSelectionItem* item : selectedItems )
    {
        if ( RiuEclipseSelectionItem* eclipseItem = dynamic_cast<RiuEclipseSelectionItem*>( item ) )
        {
            eclipseItem->m_timestepIdx = changedView->currentTimeStep();
        }
    }

    Riu3dSelectionManager::instance()->updateSelectedItems();
}
