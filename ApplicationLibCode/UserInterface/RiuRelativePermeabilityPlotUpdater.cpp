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

#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigGridBase.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"

#include "Riu3dSelectionManager.h"
#include "RiuRelativePermeabilityPlotPanel.h"

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
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotUpdater::clearPlot()
{
    if ( m_targetPlotPanel ) m_targetPlotPanel->clearPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RiuRelativePermeabilityPlotUpdater::plotPanel()
{
    return m_targetPlotPanel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuRelativePermeabilityPlotUpdater::queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResDef,
                                                                 size_t                            timeStepIndex,
                                                                 size_t                            gridIndex,
                                                                 size_t                            gridLocalCellIndex )
{
    if ( m_targetPlotPanel == nullptr ) return false;

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

            // Make sure we load the results that we'll query below
            RigCaseCellResultsData* cellResultsData = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::satnumResult() ) );
            cellResultsData->ensureKnownResultLoaded(
                RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::imbnumResult() ) );

            // Fetch SWAT and SGAS cell values for the selected cell
            cvf::ref<RigResultAccessor> swatAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            RiaResultNames::swat() ) );
            cvf::ref<RigResultAccessor> sgasAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                            RiaResultNames::sgas() ) );
            cvf::ref<RigResultAccessor> satnumAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                            RiaResultNames::satnumResult() ) );

            cvf::ref<RigResultAccessor> imbnumAccessor =
                RigResultAccessorFactory::createFromResultAddress( eclipseCaseData,
                                                                   gridIndex,
                                                                   RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                   timeStepIndex,
                                                                   RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                                            RiaResultNames::imbnumResult() ) );

            const double cellSWAT = swatAccessor.notNull() ? swatAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;
            const double cellSGAS = sgasAccessor.notNull() ? sgasAccessor->cellScalar( gridLocalCellIndex ) : HUGE_VAL;

            std::vector<RigFlowDiagDefines::RelPermCurve> relPermCurveArr =
                eclipseResultCase->flowDiagSolverInterface()->calculateRelPermCurves( activeCellIndex );

            QString cellRefText = constructCellReferenceText( eclipseCaseData, gridIndex, gridLocalCellIndex );

            if ( satnumAccessor.notNull() )
            {
                const int cellSATNUM = satnumAccessor->cellScalar( gridLocalCellIndex );
                cellRefText += QString( ", SATNUM: %1" ).arg( cellSATNUM );
            }

            if ( imbnumAccessor.notNull() )
            {
                const int cellIMBNUM = imbnumAccessor->cellScalar( gridLocalCellIndex );
                cellRefText += QString( ", IMBNUM: %1" ).arg( cellIMBNUM );
            }

            QString caseName = eclipseResultCase->caseUserDescription();

            const bool enableImbibitionCurveSelection = ( imbnumAccessor.notNull() && imbnumAccessor->cellScalar( gridLocalCellIndex ) > 0 );
            m_targetPlotPanel->enableImbibitionCurveSelection( enableImbibitionCurveSelection );
            m_targetPlotPanel->setPlotData( eclipseCaseData->unitsType(), relPermCurveArr, cellSWAT, cellSGAS, caseName, cellRefText );

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
                                                                        size_t                    gridLocalCellIndex )
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
size_t CellLookupHelper::mapToActiveCellIndex( const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex )
{
    const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;

    const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid( gridIndex ) : nullptr;

    if ( grid && gridLocalCellIndex < grid->cellCount() )
    {
        // Note!!
        // Which type of porosity model to choose? Currently hard-code to MATRIX_MODEL
        const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

        CVF_ASSERT( activeCellInfo );

        const size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );
        const size_t activeCellIndex    = activeCellInfo->cellResultIndex( reservoirCellIndex );

        return activeCellIndex;
    }

    return cvf::UNDEFINED_SIZE_T;
}
