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

#include "RiuPvtPlotUpdater.h"
#include "RiuPvtPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuSelectionManager.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigActiveCellInfo.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigCaseCellResultsData.h"

#include "RimView.h"
#include "RimEclipseView.h"
#include "RimEclipseResultCase.h"

#include "cvfBase.h"
//#include "cvfTrace.h"

#include <cmath>


//==================================================================================================
///
/// \class RiuPvtPlotUpdater
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuPvtPlotUpdater::RiuPvtPlotUpdater(RiuPvtPlotPanel* targetPlotPanel)
:   m_targetPlotPanel(targetPlotPanel),
    m_sourceEclipseViewOfLastPlot(NULL)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotUpdater::updateOnSelectionChanged(const RiuSelectionItem* selectionItem)
{
    if (!m_targetPlotPanel)
    {
        return;
    }

    m_sourceEclipseViewOfLastPlot = NULL;
    bool mustClearPlot = true;

    const RiuEclipseSelectionItem* eclipseSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(selectionItem);
    const RimEclipseView* eclipseView = eclipseSelectionItem ? eclipseSelectionItem->m_view.p() : NULL;

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
void RiuPvtPlotUpdater::updateOnTimeStepChanged(RimView* changedView)
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
bool RiuPvtPlotUpdater::queryDataAndUpdatePlot(const RimEclipseView& eclipseView, size_t gridIndex, size_t gridLocalCellIndex, RiuPvtPlotPanel* plotPanel)
{
    CVF_ASSERT(plotPanel);

    RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(eclipseView.eclipseCase());
    RigEclipseCaseData* eclipseCaseData = eclipseResultCase ? eclipseResultCase->eclipseCaseData() : NULL;
    if (eclipseResultCase && eclipseCaseData && eclipseResultCase->flowDiagSolverInterface())
    {
        size_t activeCellIndex = CellLookupHelper::mapToActiveCellIndex(eclipseCaseData, gridIndex, gridLocalCellIndex);
        if (activeCellIndex != cvf::UNDEFINED_SIZE_T)
        {
            //cvf::Trace::show("Update PVT plot for active cell index: %d", static_cast<int>(activeCellIndex));

            std::vector<RigFlowDiagSolverInterface::PvtCurve> fvfCurveArr = eclipseResultCase->flowDiagSolverInterface()->calculatePvtCurves(RigFlowDiagSolverInterface::PVT_CT_FVF, activeCellIndex);
            std::vector<RigFlowDiagSolverInterface::PvtCurve> viscosityCurveArr = eclipseResultCase->flowDiagSolverInterface()->calculatePvtCurves(RigFlowDiagSolverInterface::PVT_CT_VISCOSITY, activeCellIndex);

            const size_t timeStepIndex = static_cast<size_t>(eclipseView.currentTimeStep());

            // The following calls will read results from file in preparation for the queries below
            RigCaseCellResultsData* cellResultsData = eclipseCaseData->results(RiaDefines::MATRIX_MODEL);
            cellResultsData->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "RS");
            cellResultsData->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "RV");
            cellResultsData->findOrLoadScalarResult(RiaDefines::DYNAMIC_NATIVE, "PRESSURE");
            cellResultsData->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, "PVTNUM");

            cvf::ref<RigResultAccessor> rsAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "RS", RiaDefines::DYNAMIC_NATIVE);
            cvf::ref<RigResultAccessor> rvAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "RV", RiaDefines::DYNAMIC_NATIVE);
            cvf::ref<RigResultAccessor> pressureAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "PRESSURE", RiaDefines::DYNAMIC_NATIVE);
            cvf::ref<RigResultAccessor> pvtnumAccessor = RigResultAccessorFactory::createFromNameAndType(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, "PVTNUM", RiaDefines::STATIC_NATIVE);

            RiuPvtPlotPanel::CellValues cellValues;
            cellValues.rs = rsAccessor.notNull() ? rsAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;
            cellValues.rv = rvAccessor.notNull() ? rvAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;
            cellValues.pressure = pressureAccessor.notNull() ? pressureAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;

            const double cellPVTNUM = pvtnumAccessor.notNull() ? pvtnumAccessor->cellScalar(gridLocalCellIndex) : HUGE_VAL;


            RiuPvtPlotPanel::FvfDynProps fvfDynProps;
            eclipseResultCase->flowDiagSolverInterface()->calculatePvtDynamicPropertiesFvf(activeCellIndex, cellValues.pressure, cellValues.rs, cellValues.rv, &fvfDynProps.bo, &fvfDynProps.bg);

            RiuPvtPlotPanel::ViscosityDynProps viscosityDynProps;
            eclipseResultCase->flowDiagSolverInterface()->calculatePvtDynamicPropertiesViscosity(activeCellIndex, cellValues.pressure, cellValues.rs, cellValues.rv, &viscosityDynProps.mu_o, &viscosityDynProps.mu_g);

            QString cellRefText = constructCellReferenceText(eclipseCaseData, gridIndex, gridLocalCellIndex, cellPVTNUM);

            plotPanel->setPlotData(eclipseCaseData->unitsType(), fvfCurveArr, viscosityCurveArr, fvfDynProps, viscosityDynProps, cellValues, cellRefText);

            return true;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuPvtPlotUpdater::constructCellReferenceText(const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex, double pvtnum)
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

            QString retText = QString("Grid index %1, Cell: [%2, %3, %4]").arg(gridIndex).arg(i).arg(j).arg(k);

            if (pvtnum != HUGE_VAL)
            {
                retText += QString(" (PVTNUM=%1)").arg(pvtnum);
            }

            return retText;
        }
    }

    return QString();
}


