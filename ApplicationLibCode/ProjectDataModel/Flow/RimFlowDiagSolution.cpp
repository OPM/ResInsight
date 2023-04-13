/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimFlowDiagSolution.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"
#include "RigWellResultPoint.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"

CAF_PDM_SOURCE_INIT( RimFlowDiagSolution, "FlowDiagSolution" );

#define CROSS_FLOW_ENDING "-XF"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFlowDiagSolution::hasCrossFlowEnding( const QString& tracerName )
{
    return tracerName.endsWith( CROSS_FLOW_ENDING );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFlowDiagSolution::removeCrossFlowEnding( const QString& tracerName )
{
    if ( tracerName.endsWith( CROSS_FLOW_ENDING ) )
    {
        return tracerName.left( tracerName.size() - 3 );
    }
    else
    {
        return tracerName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFlowDiagSolution::addCrossFlowEnding( const QString& wellName )
{
    return wellName + CROSS_FLOW_ENDING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::RimFlowDiagSolution( void )
{
    CAF_PDM_InitObject( "Flow Diagnostics Solution" );
    CAF_PDM_InitField( &m_userDescription, "UserDescription", QString( "All Wells" ), "Description" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::~RimFlowDiagSolution( void )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFlowDiagSolution::userDescription() const
{
    return m_userDescription();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults* RimFlowDiagSolution::flowDiagResults()
{
    if ( m_flowDiagResults.isNull() )
    {
        size_t timeStepCount;
        {
            RimEclipseResultCase* eclCase;
            this->firstAncestorOrThisOfType( eclCase );

            if ( !eclCase || !eclCase->eclipseCaseData() )
            {
                return nullptr;
            }

            timeStepCount = eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->maxTimeStepCount();
        }

        m_flowDiagResults = new RigFlowDiagResults( this, timeStepCount );
    }

    return m_flowDiagResults.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagSolution::tracerNames() const
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType( eclCase );

    std::vector<QString> tracerNameSet;

    if ( eclCase && eclCase->eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclCase->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            tracerNameSet.push_back( simWellData[wIdx]->m_wellName );
            tracerNameSet.push_back( addCrossFlowEnding( simWellData[wIdx]->m_wellName ) );
        }
    }

    return tracerNameSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int>> RimFlowDiagSolution::allInjectorTracerActiveCellIndices( size_t timeStepIndex ) const
{
    return allTracerActiveCellIndices( timeStepIndex, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int>> RimFlowDiagSolution::allProducerTracerActiveCellIndices( size_t timeStepIndex ) const
{
    return allTracerActiveCellIndices( timeStepIndex, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<int>> RimFlowDiagSolution::allTracerActiveCellIndices( size_t timeStepIndex, bool useInjectors ) const
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType( eclCase );

    std::map<std::string, std::vector<int>> tracersWithCells;

    if ( eclCase && eclCase->eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclCase->eclipseCaseData()->wellResults();
        RigMainGrid*                           mainGrid    = eclCase->eclipseCaseData()->mainGrid();
        RigActiveCellInfo*                     activeCellInfo =
            eclCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ); // Todo: Must come from the results
                                                                                                       // definition

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            if ( !simWellData[wIdx]->hasWellResult( timeStepIndex ) ) continue;
            const RigWellResultFrame* wellResFrame = simWellData[wIdx]->wellResultFrame( timeStepIndex );

            bool isInjectorWell = ( wellResFrame->productionType() != RiaDefines::WellProductionType::PRODUCER &&
                                    wellResFrame->productionType() != RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );

            std::string wellName   = simWellData[wIdx]->m_wellName.toStdString();
            std::string wellNameXf = addCrossFlowEnding( simWellData[wIdx]->m_wellName ).toStdString();

            std::vector<int>& tracerCells          = tracersWithCells[wellName];
            std::vector<int>& tracerCellsCrossFlow = tracersWithCells[wellNameXf];

            for ( const RigWellResultBranch& wBr : wellResFrame->wellResultBranches() )
            {
                for ( const RigWellResultPoint& wrp : wBr.branchResultPoints() )
                {
                    if ( wrp.isValid() && wrp.isOpen() &&
                         ( ( useInjectors && wrp.flowRate() < 0.0 ) || ( !useInjectors && wrp.flowRate() > 0.0 ) ) )
                    {
                        RigGridBase* grid               = mainGrid->gridByIndex( wrp.gridIndex() );
                        size_t       reservoirCellIndex = grid->reservoirCellIndex( wrp.cellIndex() );

                        int cellActiveIndex = static_cast<int>( activeCellInfo->cellResultIndex( reservoirCellIndex ) );

                        if ( useInjectors == isInjectorWell )
                        {
                            tracerCells.push_back( cellActiveIndex );
                        }
                        else
                        {
                            tracerCellsCrossFlow.push_back( cellActiveIndex );
                        }
                    }
                }
            }

            if ( tracerCells.empty() ) tracersWithCells.erase( wellName );
            if ( tracerCellsCrossFlow.empty() ) tracersWithCells.erase( wellNameXf );
        }
    }

    return tracersWithCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::TracerStatusType RimFlowDiagSolution::tracerStatusOverall( const QString& tracerName ) const
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfTypeAsserted( eclCase );

    TracerStatusType tracerStatus = TracerStatusType::UNDEFINED;
    if ( eclCase && eclCase->eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclCase->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            QString wellName = removeCrossFlowEnding( tracerName );

            if ( simWellData[wIdx]->m_wellName != wellName ) continue;

            tracerStatus = TracerStatusType::CLOSED;
            for ( const RigWellResultFrame& wellResFrame : simWellData[wIdx]->m_wellCellsTimeSteps )
            {
                if ( RiaDefines::isInjector( wellResFrame.productionType() ) )
                {
                    if ( tracerStatus == TracerStatusType::PRODUCER )
                        tracerStatus = TracerStatusType::VARYING;
                    else
                        tracerStatus = TracerStatusType::INJECTOR;
                }
                else if ( wellResFrame.productionType() == RiaDefines::WellProductionType::PRODUCER )
                {
                    if ( tracerStatus == TracerStatusType::INJECTOR )
                        tracerStatus = TracerStatusType::VARYING;
                    else
                        tracerStatus = TracerStatusType::PRODUCER;
                }
                if ( tracerStatus == TracerStatusType::VARYING ) break;
            }

            break;
        }

        if ( hasCrossFlowEnding( tracerName ) )
        {
            if ( tracerStatus == TracerStatusType::PRODUCER )
                tracerStatus = TracerStatusType::INJECTOR;
            else if ( tracerStatus == TracerStatusType::INJECTOR )
                tracerStatus = TracerStatusType::PRODUCER;
        }
    }

    return tracerStatus;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution::TracerStatusType RimFlowDiagSolution::tracerStatusInTimeStep( const QString& tracerName, size_t timeStepIndex ) const
{
    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfTypeAsserted( eclCase );

    if ( eclCase && eclCase->eclipseCaseData() )
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclCase->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            QString wellName = removeCrossFlowEnding( tracerName );

            if ( simWellData[wIdx]->m_wellName != wellName ) continue;
            if ( !simWellData[wIdx]->hasWellResult( timeStepIndex ) ) return TracerStatusType::CLOSED;

            const RigWellResultFrame* wellResFrame = simWellData[wIdx]->wellResultFrame( timeStepIndex );

            if ( RiaDefines::isInjector( wellResFrame->productionType() ) )
            {
                if ( hasCrossFlowEnding( tracerName ) ) return TracerStatusType::PRODUCER;

                return TracerStatusType::INJECTOR;
            }
            else if ( wellResFrame->productionType() == RiaDefines::WellProductionType::PRODUCER ||
                      wellResFrame->productionType() == RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
            {
                if ( hasCrossFlowEnding( tracerName ) ) return TracerStatusType::INJECTOR;

                return TracerStatusType::PRODUCER;
            }
            else
            {
                CVF_ASSERT( false );
            }
        }
    }

    return TracerStatusType::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFlowDiagSolution::tracerColor( const QString& tracerName ) const
{
    QString wellName = removeCrossFlowEnding( tracerName );

    if ( wellName == RIG_FLOW_TOTAL_NAME ) return cvf::Color3f::LIGHT_GRAY;
    if ( wellName == RIG_RESERVOIR_TRACER_NAME ) return cvf::Color3f::LIGHT_GRAY;
    if ( wellName == RIG_TINY_TRACER_GROUP_NAME ) return cvf::Color3f::DARK_GRAY;

    RimEclipseResultCase* eclCase;
    this->firstAncestorOrThisOfType( eclCase );

    if ( eclCase )
    {
        return eclCase->defaultWellColor( wellName );
    }

    return cvf::Color3f::LIGHT_GRAY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimFlowDiagSolution::userDescriptionField()
{
    return &m_userDescription;
}
