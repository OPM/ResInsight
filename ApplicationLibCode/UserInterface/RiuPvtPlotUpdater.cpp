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

#include "RigEclipseCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim3dView.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"

#include "Riu3dSelectionManager.h"
#include "RiuPvtPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"

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
RiuPvtPlotUpdater::RiuPvtPlotUpdater( RiuPvtPlotPanel* targetPlotPanel )
    : m_targetPlotPanel( targetPlotPanel )
    , m_viewToFollowAnimationFrom( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotUpdater::updateOnSelectionChanged( const RiuSelectionItem* selectionItem )
{
    if ( !m_targetPlotPanel )
    {
        return;
    }

    Rim3dView*               newFollowAnimView    = nullptr;
    RiuEclipseSelectionItem* eclipseSelectionItem = nullptr;

    eclipseSelectionItem =
        RiuRelativePermeabilityPlotUpdater::extractEclipseSelectionItem( selectionItem, newFollowAnimView );

    bool mustClearPlot          = true;
    m_viewToFollowAnimationFrom = nullptr;

    if ( m_targetPlotPanel->isVisible() && eclipseSelectionItem && eclipseSelectionItem->m_resultDefinition )
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
void RiuPvtPlotUpdater::updateOnTimeStepChanged( Rim3dView* changedView )
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

    eclipseSelectionItem =
        RiuRelativePermeabilityPlotUpdater::extractEclipseSelectionItem( selectionItem, newFollowAnimView );

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
bool RiuPvtPlotUpdater::queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResDef,
                                                size_t                            timeStepIndex,
                                                size_t                            gridIndex,
                                                size_t                            gridLocalCellIndex,
                                                RiuPvtPlotPanel*                  plotPanel )
{
    CVF_ASSERT( plotPanel );

    if ( !eclipseResDef ) return false;

    RimEclipseResultCase* eclipseResultCase     = dynamic_cast<RimEclipseResultCase*>( eclipseResDef->eclipseCase() );
    RigEclipseCaseData*   eclipseCaseData       = eclipseResultCase ? eclipseResultCase->eclipseCaseData() : nullptr;
    RiaDefines::PorosityModelType porosityModel = eclipseResDef->porosityModel();

    if ( eclipseResultCase && eclipseCaseData && eclipseResultCase->flowDiagSolverInterface() )
    {
        // cvf::Trace::show("Update PVT plot for active cell index: %d", static_cast<int>(activeCellIndex));

        // The following calls will read results from file in preparation for the queries below
        RigCaseCellResultsData* cellResultsData = eclipseCaseData->results( porosityModel );

        cellResultsData->ensureKnownResultLoaded(
            RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "RS" ) );
        cellResultsData->ensureKnownResultLoaded(
            RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "RV" ) );
        cellResultsData->ensureKnownResultLoaded(
            RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "PRESSURE" ) );
        cellResultsData->ensureKnownResultLoaded(
            RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PVTNUM" ) );

        cvf::ref<RigResultAccessor> rsAccessor =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               gridIndex,
                                                               porosityModel,
                                                               timeStepIndex,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                        "RS" ) );
        cvf::ref<RigResultAccessor> rvAccessor =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               gridIndex,
                                                               porosityModel,
                                                               timeStepIndex,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                        "RV" ) );
        cvf::ref<RigResultAccessor> pressureAccessor =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               gridIndex,
                                                               porosityModel,
                                                               timeStepIndex,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                        "PRESSURE" ) );
        cvf::ref<RigResultAccessor> pvtnumAccessor =
            RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                               gridIndex,
                                                               porosityModel,
                                                               timeStepIndex,
                                                               RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                        "PVTNUM" ) );

        RiuPvtPlotPanel::CellValues cellValues;
        cellValues.rs       = rsAccessor.notNull() ? rsAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;
        cellValues.rv       = rvAccessor.notNull() ? rvAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;
        cellValues.pressure = pressureAccessor.notNull() ? pressureAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;

        const double cellPvtNumDouble = pvtnumAccessor.notNull() ? pvtnumAccessor->cellScalar( gridLocalCellIndex )
                                                                 : HUGE_VAL;

        const int cellPvtNum = ( cellPvtNumDouble != HUGE_VAL ) ? static_cast<int>( cellPvtNumDouble )
                                                                : std::numeric_limits<int>::max();

        std::vector<RigFlowDiagSolverInterface::PvtCurve> fvfCurveArr =
            eclipseResultCase->flowDiagSolverInterface()->calculatePvtCurves( RigFlowDiagSolverInterface::PVT_CT_FVF,
                                                                              cellPvtNum );
        std::vector<RigFlowDiagSolverInterface::PvtCurve> viscosityCurveArr =
            eclipseResultCase->flowDiagSolverInterface()->calculatePvtCurves( RigFlowDiagSolverInterface::PVT_CT_VISCOSITY,
                                                                              cellPvtNum );

        RiuPvtPlotPanel::FvfDynProps fvfDynProps;
        eclipseResultCase->flowDiagSolverInterface()->calculatePvtDynamicPropertiesFvf( cellPvtNum,
                                                                                        cellValues.pressure,
                                                                                        cellValues.rs,
                                                                                        cellValues.rv,
                                                                                        &fvfDynProps.bo,
                                                                                        &fvfDynProps.bg );

        RiuPvtPlotPanel::ViscosityDynProps viscosityDynProps;
        eclipseResultCase->flowDiagSolverInterface()->calculatePvtDynamicPropertiesViscosity( cellPvtNum,
                                                                                              cellValues.pressure,
                                                                                              cellValues.rs,
                                                                                              cellValues.rv,
                                                                                              &viscosityDynProps.mu_o,
                                                                                              &viscosityDynProps.mu_g );

        QString cellRefText = RiuRelativePermeabilityPlotUpdater::constructCellReferenceText( eclipseCaseData,
                                                                                              gridIndex,
                                                                                              gridLocalCellIndex,
                                                                                              "PVTNUM",
                                                                                              cellPvtNumDouble );

        plotPanel->setPlotData( eclipseCaseData->unitsType(),
                                fvfCurveArr,
                                viscosityCurveArr,
                                fvfDynProps,
                                viscosityDynProps,
                                cellValues,
                                cellRefText );

        return true;
    }

    return false;
}
