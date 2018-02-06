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
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuSelectionManager.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigActiveCellInfo.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigCaseCellResultsData.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"
#include "Rim2dIntersectionView.h"
#include "RimIntersection.h"

#include "cvfBase.h"
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
RiuRelativePermeabilityPlotUpdater::RiuRelativePermeabilityPlotUpdater(RiuRelativePermeabilityPlotPanel* targetPlotPanel)
:   m_targetPlotPanel(targetPlotPanel),
    m_sourceEclipseViewOfLastPlot(NULL)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotUpdater::updateOnSelectionChanged(const RiuSelectionItem* selectionItem)
{
    if (!m_targetPlotPanel)
    {
        return;
    }

    m_sourceEclipseViewOfLastPlot = NULL;
    bool mustClearPlot = true;

    RiuEclipseSelectionItem* eclipseSelectionItem = dynamic_cast<RiuEclipseSelectionItem*>(const_cast<RiuSelectionItem*>(selectionItem));
    RimEclipseView* eclipseView = eclipseSelectionItem ? eclipseSelectionItem->m_view.p() : NULL;

    if (!eclipseSelectionItem && !eclipseView)
    {
        const Riu2dIntersectionSelectionItem* intersectionSelItem = dynamic_cast<const Riu2dIntersectionSelectionItem*>(selectionItem);
        if (intersectionSelItem && intersectionSelItem->eclipseSelectionItem())
        {
            eclipseSelectionItem = intersectionSelItem->eclipseSelectionItem();
            eclipseView = eclipseSelectionItem->m_view;
        }
    }

    if (m_targetPlotPanel->isVisible() && eclipseSelectionItem && eclipseView)
    {
        const size_t gridIndex = eclipseSelectionItem->m_gridIndex;
        const size_t gridLocalCellIndex = eclipseSelectionItem->m_gridLocalCellIndex;
        if (queryDataAndUpdatePlot(*eclipseView, gridIndex, gridLocalCellIndex, m_targetPlotPanel))
        {
            mustClearPlot = false;
            m_sourceEclipseViewOfLastPlot = eclipseView;
        }
    }

    if (mustClearPlot)
    {
        m_targetPlotPanel->clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotUpdater::updateOnTimeStepChanged(Rim3dView* changedView)
{
    if (!m_targetPlotPanel || !m_targetPlotPanel->isVisible())
    {
        return;
    }

    // Don't update the plot if the view that changed time step is different from the view that was the source of the current plot
    const RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(changedView);
    if (!eclipseView || eclipseView != m_sourceEclipseViewOfLastPlot) 
    {
        return;
    }

    // Fetch the current global selection and only continue if the selection's view matches the view with time step change
    const RiuEclipseSelectionItem* eclipseSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(RiuSelectionManager::instance()->selectedItem());
    if (eclipseSelectionItem && eclipseSelectionItem->m_view == eclipseView)
    {
        const size_t gridIndex = eclipseSelectionItem->m_gridIndex;
        const size_t gridLocalCellIndex = eclipseSelectionItem->m_gridLocalCellIndex;
        if (!queryDataAndUpdatePlot(*eclipseView, gridIndex, gridLocalCellIndex, m_targetPlotPanel))
        {
            m_targetPlotPanel->clearPlot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuRelativePermeabilityPlotUpdater::queryDataAndUpdatePlot(const RimEclipseView& eclipseView, size_t gridIndex, size_t gridLocalCellIndex, RiuRelativePermeabilityPlotPanel* plotPanel)
{
    CVF_ASSERT(plotPanel);

    RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(eclipseView.eclipseCase());
    RigEclipseCaseData* eclipseCaseData = eclipseResultCase ? eclipseResultCase->eclipseCaseData() : NULL;
    if (eclipseResultCase && eclipseCaseData && eclipseResultCase->flowDiagSolverInterface())
    {
        size_t activeCellIndex = CellLookupHelper::mapToActiveCellIndex(eclipseCaseData, gridIndex, gridLocalCellIndex);
        if (activeCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            //cvf::Trace::show("Updating RelPerm plot for active cell index: %d", static_cast<int>(activeCellIndex));

            std::vector<RigFlowDiagSolverInterface::RelPermCurve> relPermCurveArr = eclipseResultCase->flowDiagSolverInterface()->calculateRelPermCurves(activeCellIndex);

            // Make sure we load the results that we'll query below
            RigCaseCellResultsData* cellResultsData = eclipseCaseData->results(RiaDefines::MATRIX_MODEL);
            cellResultsData->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SWAT");
            cellResultsData->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "SGAS");
            cellResultsData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "SATNUM");

            // Fetch SWAT and SGAS cell values for the selected cell
            const size_t timeStepIndex = static_cast<size_t>(eclipseView.currentTimeStep());
            cvf::ref<RigResultAccessor> swatAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "SWAT", RiaDefines::DYNAMIC_NATIVE);
            cvf::ref<RigResultAccessor> sgasAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "SGAS", RiaDefines::DYNAMIC_NATIVE);
            cvf::ref<RigResultAccessor> satnumAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "SATNUM", RiaDefines::STATIC_NATIVE);
            const double cellSWAT = swatAccessor.notNull() ? swatAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;
            const double cellSGAS = sgasAccessor.notNull() ? sgasAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;
            const double cellSATNUM = satnumAccessor.notNull() ? satnumAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;
            //cvf::Trace::show("cellSWAT = %f  cellSGAS = %f  cellSATNUM = %f", cellSWAT, cellSGAS, cellSATNUM);

            QString cellRefText = constructCellReferenceText(eclipseCaseData, gridIndex, gridLocalCellIndex, cellSATNUM);
            QString caseName = eclipseResultCase->caseUserDescription;

            plotPanel->setPlotData(eclipseCaseData->unitsType(), relPermCurveArr, cellSWAT, cellSGAS, caseName, cellRefText);

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotUpdater::constructCellReferenceText(const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex, double satnum)
{
    const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;
    const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid(gridIndex) : NULL;
    if (grid && gridLocalCellIndex < grid->cellCount())
    {
        size_t i = 0;
        size_t j = 0;
        size_t k = 0;
        if (grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k))
        {
            // Adjust to 1-based Eclipse indexing
            i++;
            j++;
            k++;

            QString retText;
            if (gridIndex == 0)
            {
                retText = QString("Cell: [%1, %2, %3]").arg(i).arg(j).arg(k);
            }
            else
            {
                retText = QString("LGR %1, Cell: [%2, %3, %4]").arg(gridIndex).arg(i).arg(j).arg(k);
            }
            if (satnum != HUGE_VAL)
            {
                retText += QString(" (SATNUM=%1)").arg(satnum);
            }

            return retText;
        }
    }

    return QString();
}


//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t CellLookupHelper::mapToActiveCellIndex(const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex)
{
    const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;
    const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid(gridIndex) : NULL;
    if (grid && gridLocalCellIndex < grid->cellCount())
    {
        // Note!!
        // Which type of porosity model to choose? Currently hard-code to MATRIX_MODEL
        const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(RiaDefines::MATRIX_MODEL);
        CVF_ASSERT(activeCellInfo);

        const size_t reservoirCellIndex = grid->reservoirCellIndex(gridLocalCellIndex);
        const size_t activeCellIndex = activeCellInfo->cellResultIndex(reservoirCellIndex);
        return activeCellIndex;
    }

    return cvf::UNDEFINED_SIZE_T;
}

