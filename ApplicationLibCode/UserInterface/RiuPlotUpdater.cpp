/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RiuPlotUpdater.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"

#include "Riu3dSelectionManager.h"

#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotUpdater::RiuPlotUpdater()
    : m_viewToFollowAnimationFrom( nullptr )
    , m_eclipseResultDef( nullptr )
    , m_gridIndex( 0 )
    , m_gridLocalCellIndex( 0 )
    , m_timeStepIndex( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem* RiuPlotUpdater::extractEclipseSelectionItem( const RiuSelectionItem* selectionItem, Rim3dView*& newFollowAnimView )
{
    newFollowAnimView                             = nullptr;
    RiuEclipseSelectionItem* eclipseSelectionItem = nullptr;

    eclipseSelectionItem = dynamic_cast<RiuEclipseSelectionItem*>( const_cast<RiuSelectionItem*>( selectionItem ) );

    if ( eclipseSelectionItem )
    {
        // If we clicked in an eclipse view, and hit something using the standard result definition there,
        // set this up to follow the animation there.

        RimEclipseView* clickedInEclView = dynamic_cast<RimEclipseView*>( eclipseSelectionItem->m_view.p() );

        if ( clickedInEclView && clickedInEclView->cellResult() == eclipseSelectionItem->m_resultDefinition.p() )
        {
            newFollowAnimView = eclipseSelectionItem->m_view;
        }
    }
    else
    {
        auto intersectionSelItem = dynamic_cast<const Riu2dIntersectionSelectionItem*>( selectionItem );

        if ( intersectionSelItem && intersectionSelItem->eclipseSelectionItem() )
        {
            eclipseSelectionItem = intersectionSelItem->eclipseSelectionItem();
            newFollowAnimView    = intersectionSelItem->view();
        }
    }

    return eclipseSelectionItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotUpdater::doDelayedUpdate()
{
    if ( m_eclipseResultDef != nullptr )
    {
        if ( !queryDataAndUpdatePlot( m_eclipseResultDef, m_timeStepIndex, m_gridIndex, m_gridLocalCellIndex ) )
        {
            clearPlot();
        }
        clearDelayedInformation();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotUpdater::storeDelayedInformation( const RiuEclipseSelectionItem* selItem )
{
    if ( selItem != nullptr )
    {
        m_eclipseResultDef   = selItem->m_resultDefinition;
        m_timeStepIndex      = selItem->m_timestepIdx;
        m_gridIndex          = selItem->m_gridIndex;
        m_gridLocalCellIndex = selItem->m_gridLocalCellIndex;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotUpdater::clearDelayedInformation()
{
    m_eclipseResultDef = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotUpdater::updateOnSelectionChanged( const RiuSelectionItem* selectionItem )
{
    if ( !plotPanel() )
    {
        return;
    }

    Rim3dView*               newFollowAnimView    = nullptr;
    RiuEclipseSelectionItem* eclipseSelectionItem = nullptr;

    eclipseSelectionItem = extractEclipseSelectionItem( selectionItem, newFollowAnimView );

    bool mustClearPlot          = true;
    m_viewToFollowAnimationFrom = nullptr;

    if ( eclipseSelectionItem && eclipseSelectionItem->m_resultDefinition )
    {
        if ( plotPanel()->isVisible() )
        {
            if ( queryDataAndUpdatePlot( eclipseSelectionItem->m_resultDefinition,
                                         eclipseSelectionItem->m_timestepIdx,
                                         eclipseSelectionItem->m_gridIndex,
                                         eclipseSelectionItem->m_gridLocalCellIndex ) )
            {
                mustClearPlot = false;
            }
        }
        else
        {
            storeDelayedInformation( eclipseSelectionItem );
            mustClearPlot = false;
        }
        m_viewToFollowAnimationFrom = newFollowAnimView;
    }

    if ( mustClearPlot )
    {
        clearDelayedInformation();
        clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotUpdater::updateOnTimeStepChanged( Rim3dView* changedView )
{
    if ( !plotPanel() )
    {
        return;
    }

    // Don't update the plot if the view that changed time step is different
    // from the view that was the source of the current plot

    if ( changedView != m_viewToFollowAnimationFrom )
    {
        return;
    }

    // Fetch the current global selection and only continue if the selection's view matches the view with time step change

    const RiuSelectionItem*  selectionItem        = Riu3dSelectionManager::instance()->selectedItem();
    Rim3dView*               newFollowAnimView    = nullptr;
    RiuEclipseSelectionItem* eclipseSelectionItem = nullptr;

    eclipseSelectionItem = RiuPlotUpdater::extractEclipseSelectionItem( selectionItem, newFollowAnimView );

    if ( eclipseSelectionItem && newFollowAnimView == changedView )
    {
        if ( plotPanel()->isVisible() )
        {
            if ( !queryDataAndUpdatePlot( eclipseSelectionItem->m_resultDefinition,
                                          newFollowAnimView->currentTimeStep(),
                                          eclipseSelectionItem->m_gridIndex,
                                          eclipseSelectionItem->m_gridLocalCellIndex ) )
            {
                clearPlot();
            }
        }
        else
        {
            storeDelayedInformation( eclipseSelectionItem );
            m_timeStepIndex = newFollowAnimView->currentTimeStep();
        }
    }
}
