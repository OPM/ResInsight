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
#include "Riu3dSelectionManager.h"

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
    m_sourceEclipseViewOfLastPlot(nullptr)
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

    m_sourceEclipseViewOfLastPlot = nullptr;
    bool mustClearPlot = true;

    RiuEclipseSelectionItem* eclipseSelectionItem = dynamic_cast<RiuEclipseSelectionItem*>(const_cast<RiuSelectionItem*>(selectionItem));
    RimEclipseView* eclipseView = eclipseSelectionItem ? eclipseSelectionItem->m_view.p() : nullptr;

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
void RiuPvtPlotUpdater::updateOnTimeStepChanged(Rim3dView* changedView)
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
    const RiuEclipseSelectionItem* eclipseSelectionItem = dynamic_cast<const RiuEclipseSelectionItem*>(Riu3dSelectionManager::instance()->selectedItem());
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
    RigEclipseCaseData* eclipseCaseData = eclipseResultCase ? eclipseResultCase->eclipseCaseData() : nullptr;
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
            cellResultsData->ensureKnownResultLoaded(RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "RS"));
            cellResultsData->ensureKnownResultLoaded(RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "RV"));
            cellResultsData->ensureKnownResultLoaded(RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "PRESSURE"));
            cellResultsData->ensureKnownResultLoaded(RigEclipseResultAddress(RiaDefines::STATIC_NATIVE, "PVTNUM"));

            cvf::ref<RigResultAccessor> rsAccessor       = RigResultAccessorFactory::createFromResultAddress(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "RS"       ));
            cvf::ref<RigResultAccessor> rvAccessor       = RigResultAccessorFactory::createFromResultAddress(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "RV"       ));
            cvf::ref<RigResultAccessor> pressureAccessor = RigResultAccessorFactory::createFromResultAddress(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, RigEclipseResultAddress(RiaDefines::DYNAMIC_NATIVE, "PRESSURE" ));
            cvf::ref<RigResultAccessor> pvtnumAccessor   = RigResultAccessorFactory::createFromResultAddress(eclipseCaseData, gridIndex, RiaDefines::MATRIX_MODEL, timeStepIndex, RigEclipseResultAddress(RiaDefines::STATIC_NATIVE , "PVTNUM"   ));

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
    const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid(gridIndex) : nullptr;
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

            if (pvtnum != HUGE_VAL)
            {
                retText += QString(" (PVTNUM=%1)").arg(pvtnum);
            }

            return retText;
        }
    }

    return QString();
}


