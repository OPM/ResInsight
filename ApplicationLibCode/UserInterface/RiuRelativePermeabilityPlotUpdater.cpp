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

#include "RiuRelativePermeabilityPlotUpdater.h"
#include "Riu3dSelectionManager.h"
#include "RiuRelativePermeabilityPlotPanel.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"

//#include "cvfTrace.h"

#include <cmath>

//==================================================================================================
///
/// \class RiuRelativePermeabilityPlotUpdater
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotUpdater::RiuRelativePermeabilityPlotUpdater( RiuRelativePermeabilityPlotPanel* targetPlotPanel )
    : m_targetPlotPanel( targetPlotPanel )
    , m_viewToFollowAnimationFrom( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotUpdater::updateOnSelectionChanged( const RiuSelectionItem* selectionItem )
{
    if ( !m_targetPlotPanel )
    {
        return;
    }

    Rim3dView*               newFollowAnimView    = nullptr;
    RiuEclipseSelectionItem* eclipseSelectionItem = nullptr;

    eclipseSelectionItem = extractEclipseSelectionItem( selectionItem, newFollowAnimView );

    bool mustClearPlot          = true;
    m_viewToFollowAnimationFrom = nullptr;

    if ( m_targetPlotPanel->isVisible() && eclipseSelectionItem )
    {
        if ( queryDataAndUpdatePlot( eclipseSelectionItem->m_resultDefinition,
                                     eclipseSelectionItem->m_timestepIdx,
                                     eclipseSelectionItem->m_gridIndex,
                                     eclipseSelectionItem->m_gridLocalCellIndex,
                                     m_targetPlotPanel ) )
        {
            mustClearPlot = false;

            m_viewToFollowAnimationFrom = newFollowAnimView;
        }
    }

    if ( mustClearPlot )
    {
        m_targetPlotPanel->clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotUpdater::updateOnTimeStepChanged( Rim3dView* changedView )
{
    if ( !m_targetPlotPanel || !m_targetPlotPanel->isVisible() )
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

    eclipseSelectionItem = extractEclipseSelectionItem( selectionItem, newFollowAnimView );

    if ( eclipseSelectionItem && newFollowAnimView == changedView )
    {
        if ( !queryDataAndUpdatePlot( eclipseSelectionItem->m_resultDefinition,
                                      newFollowAnimView->currentTimeStep(),
                                      eclipseSelectionItem->m_gridIndex,
                                      eclipseSelectionItem->m_gridLocalCellIndex,
                                      m_targetPlotPanel ) )
        {
            m_targetPlotPanel->clearPlot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuRelativePermeabilityPlotUpdater::queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResDef,
                                                                 size_t                            timeStepIndex,
                                                                 size_t                            gridIndex,
                                                                 size_t                            gridLocalCellIndex,
                                                                 RiuRelativePermeabilityPlotPanel* plotPanel )
{
    CVF_ASSERT( plotPanel );

    if ( !eclipseResDef ) return false;

    if ( eclipseResDef->porosityModel() == RiaDefines::PorosityModelType::FRACTURE_MODEL )
    {
        // Fracture model is currently not supported. If a dual porosity model is present, the PORV values
        // for the matrix model is used. It will require some changes in the flow diagnostics module to be
        // able support fracture model plotting.

        return false;
    }

    RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>( eclipseResDef->eclipseCase() );
    RigEclipseCaseData*   eclipseCaseData   = eclipseResultCase ? eclipseResultCase->eclipseCaseData() : nullptr;

    if ( eclipseResultCase && eclipseCaseData && eclipseResultCase->flowDiagSolverInterface() )
    {
        size_t activeCellIndex = CellLookupHelper::mapToActiveCellIndex( eclipseCaseData, gridIndex, gridLocalCellIndex );

        if ( activeCellIndex != cvf::UNDEFINED_SIZE_T )
        {
            // cvf::Trace::show("Updating RelPerm plot for active cell index: %d", static_cast<int>(activeCellIndex));

            std::vector<RigFlowDiagSolverInterface::RelPermCurve> relPermCurveArr =
                eclipseResultCase->flowDiagSolverInterface()->calculateRelPermCurves( activeCellIndex );

            // Make sure we load the results that we'll query below
            RigCaseCellResultsData* cellResultsData =
                eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SWAT" ) );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "SGAS" ) );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "SATNUM" ) );

            // Fetch SWAT and SGAS cell values for the selected cell
            cvf::ref<RigResultAccessor> swatAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            "SWAT" ) );
            cvf::ref<RigResultAccessor> sgasAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            "SGAS" ) );
            cvf::ref<RigResultAccessor> satnumAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                            "SATNUM" ) );
            const double cellSWAT = swatAccessor.notNull() ? swatAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;
            const double cellSGAS = sgasAccessor.notNull() ? sgasAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;
            const double cellSATNUM = satnumAccessor.notNull() ? satnumAccessor->cellScalar( gridLocalCellIndex )
                                                               : HUGE_VAL;
            // cvf::Trace::show("cellSWAT = %f  cellSGAS = %f  cellSATNUM = %f", cellSWAT, cellSGAS, cellSATNUM);

            QString cellRefText =
                constructCellReferenceText( eclipseCaseData, gridIndex, gridLocalCellIndex, "SATNUM", cellSATNUM );
            QString caseName = eclipseResultCase->caseUserDescription;

            plotPanel->setPlotData( eclipseCaseData->unitsType(), relPermCurveArr, cellSWAT, cellSGAS, caseName, cellRefText );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotUpdater::constructCellReferenceText( const RigEclipseCaseData* eclipseCaseData,
                                                                        size_t                    gridIndex,
                                                                        size_t                    gridLocalCellIndex,
                                                                        const QString&            valueName,
                                                                        double                    cellValue )
{
    const size_t       gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;
    const RigGridBase* grid      = gridIndex < gridCount ? eclipseCaseData->grid( gridIndex ) : nullptr;
    if ( grid && gridLocalCellIndex < grid->cellCount() )
    {
        size_t i = 0;
        size_t j = 0;
        size_t k = 0;
        if ( grid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k ) )
        {
            // Adjust to 1-based Eclipse indexing
            i++;
            j++;
            k++;

            QString retText;
            if ( gridIndex == 0 )
            {
                retText = QString( "Cell: [%1, %2, %3]" ).arg( i ).arg( j ).arg( k );
            }
            else
            {
                retText = QString( "LGR %1, Cell: [%2, %3, %4]" ).arg( gridIndex ).arg( i ).arg( j ).arg( k );
            }
            if ( cellValue != HUGE_VAL )
            {
                retText += QString( " (%1=%2)" ).arg( valueName ).arg( cellValue );
            }

            return retText;
        }
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuEclipseSelectionItem*
    RiuRelativePermeabilityPlotUpdater::extractEclipseSelectionItem( const RiuSelectionItem* selectionItem,
                                                                     Rim3dView*&             newFollowAnimView )
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

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t CellLookupHelper::mapToActiveCellIndex( const RigEclipseCaseData* eclipseCaseData,
                                               size_t                    gridIndex,
                                               size_t                    gridLocalCellIndex )
{
    const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;

    const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid( gridIndex ) : nullptr;

    if ( grid && gridLocalCellIndex < grid->cellCount() )
    {
        // Note!!
        // Which type of porosity model to choose? Currently hard-code to MATRIX_MODEL
        const RigActiveCellInfo* activeCellInfo =
            eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

        CVF_ASSERT( activeCellInfo );

        const size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );
        const size_t activeCellIndex    = activeCellInfo->cellResultIndex( reservoirCellIndex );

        return activeCellIndex;
    }

    return cvf::UNDEFINED_SIZE_T;
}
