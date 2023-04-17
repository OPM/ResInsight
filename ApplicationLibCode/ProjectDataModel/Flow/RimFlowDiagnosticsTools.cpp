/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimFlowDiagnosticsTools.h"

#include "RigFlowDiagResults.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFlowDiagnosticsTools::TracerComp::operator()( const QString& lhs, const QString& rhs ) const
{
    if ( !lhs.endsWith( "-XF" ) && rhs.endsWith( "-XF" ) )
    {
        return true;
    }
    else if ( lhs.endsWith( "-XF" ) && !rhs.endsWith( "-XF" ) )
    {
        return false;
    }
    else
    {
        return lhs < rhs;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFlowDiagnosticsTools::calcOptionsForSelectedTracerField( RimFlowDiagSolution* flowSol, bool isInjector )
{
    if ( !flowSol ) return {};

    QList<caf::PdmOptionItemInfo> options;
    std::set<QString, TracerComp> sortedTracers = setOfTracersOfType( flowSol, isInjector );
    for ( const QString& tracerName : sortedTracers )
    {
        QString                               postfix;
        RimFlowDiagSolution::TracerStatusType status = flowSol->tracerStatusOverall( tracerName );
        if ( status == RimFlowDiagSolution::TracerStatusType::VARYING )
        {
            postfix = " [I/P]";
        }
        else if ( status == RimFlowDiagSolution::TracerStatusType::UNDEFINED )
        {
            postfix = " [U]";
        }
        options.push_back( caf::PdmOptionItemInfo( tracerName + postfix, tracerName ) );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp> RimFlowDiagnosticsTools::setOfTracersOfType( RimFlowDiagSolution* flowSol,
                                                                                                    bool                 isInjector )
{
    if ( !flowSol ) return {};

    std::set<QString, TracerComp> sortedTracers;
    std::vector<QString>          tracerNames = flowSol->tracerNames();
    for ( const QString& tracerName : tracerNames )
    {
        RimFlowDiagSolution::TracerStatusType status        = flowSol->tracerStatusOverall( tracerName );
        bool                                  includeTracer = status == RimFlowDiagSolution::TracerStatusType::VARYING ||
                             status == RimFlowDiagSolution::TracerStatusType::UNDEFINED;
        includeTracer |= isInjector && status == RimFlowDiagSolution::TracerStatusType::INJECTOR;
        includeTracer |= !isInjector && status == RimFlowDiagSolution::TracerStatusType::PRODUCER;

        if ( includeTracer )
        {
            sortedTracers.insert( tracerName );
        }
    }
    return sortedTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagnosticsTools::producerTracersInTimeStep( RimFlowDiagSolution* flowSol, int timeStepIndex )
{
    return tracersOfStatusInTimeStep( flowSol, RimFlowDiagSolution::TracerStatusType::PRODUCER, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagnosticsTools::injectorTracersInTimeStep( RimFlowDiagSolution* flowSol, int timeStepIndex )
{
    return tracersOfStatusInTimeStep( flowSol, RimFlowDiagSolution::TracerStatusType::INJECTOR, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagnosticsTools::tracersOfStatusInTimeStep( RimFlowDiagSolution*                  flowSol,
                                                                         RimFlowDiagSolution::TracerStatusType status,
                                                                         int                                   timeStepIndex )
{
    if ( !flowSol || timeStepIndex < 0 ) return {};

    std::vector<QString> tracers;
    for ( const auto& tracer : flowSol->tracerNames() )
    {
        if ( status == flowSol->tracerStatusInTimeStep( tracer, timeStepIndex ) )
        {
            tracers.push_back( tracer );
        }
    }
    return tracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    RimFlowDiagnosticsTools::setOfInjectorTracersFromProducers( RimFlowDiagSolution*        flowSol,
                                                                const std::vector<QString>& producerTracers,
                                                                std::vector<int>            timeStepIndices )
{
    if ( !flowSol ) return {};

    const double                  epsilon    = 1.0e-8;
    const bool                    isInjector = true;
    std::set<QString, TracerComp> communicatingInjectors;
    std::set<QString, TracerComp> injectors = RimFlowDiagnosticsTools::setOfTracersOfType( flowSol, isInjector );
    for ( const QString& producer : producerTracers )
    {
        for ( const QString& injector : injectors )
        {
            for ( const auto& timeStepIndex : timeStepIndices )
            {
                if ( timeStepIndex < 0 ) continue;
                std::pair<double, double> commFluxes =
                    flowSol->flowDiagResults()->injectorProducerPairFluxes( injector.toStdString(), producer.toStdString(), timeStepIndex );
                if ( std::abs( commFluxes.first ) > epsilon || std::abs( commFluxes.second ) > epsilon )
                {
                    communicatingInjectors.insert( injector );
                }
            }
        }
    }
    return communicatingInjectors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    RimFlowDiagnosticsTools::setOfInjectorTracersFromProducers( RimFlowDiagSolution*        flowSol,
                                                                const std::vector<QString>& producerTracers,
                                                                int                         timeStepIndex )
{
    if ( timeStepIndex < 0 ) return {};

    const auto timeStepIndices = std::vector<int>( { timeStepIndex } );
    return setOfInjectorTracersFromProducers( flowSol, producerTracers, timeStepIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    RimFlowDiagnosticsTools::setOfProducerTracersFromInjectors( RimFlowDiagSolution*        flowSol,
                                                                const std::vector<QString>& injectorTracers,
                                                                std::vector<int>            timeStepIndices )
{
    if ( !flowSol ) return {};

    const double                  epsilon    = 1.0e-8;
    const bool                    isInjector = false;
    std::set<QString, TracerComp> communicatingProducers;
    std::set<QString, TracerComp> producers = RimFlowDiagnosticsTools::setOfTracersOfType( flowSol, isInjector );
    for ( const QString& injector : injectorTracers )
    {
        for ( const QString& producer : producers )
        {
            for ( const auto& timeStepIndex : timeStepIndices )
            {
                if ( timeStepIndex < 0 ) continue;
                std::pair<double, double> commFluxes =
                    flowSol->flowDiagResults()->injectorProducerPairFluxes( injector.toStdString(), producer.toStdString(), timeStepIndex );
                if ( std::abs( commFluxes.first ) > epsilon || std::abs( commFluxes.second ) > epsilon )
                {
                    communicatingProducers.insert( producer );
                }
            }
        }
    }
    return communicatingProducers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    RimFlowDiagnosticsTools::setOfProducerTracersFromInjectors( RimFlowDiagSolution*        flowSol,
                                                                const std::vector<QString>& injectorTracers,
                                                                int                         timeStepIndex )
{
    if ( timeStepIndex < 0 ) return {};

    const auto timeStepIndices = std::vector<int>( { timeStepIndex } );
    return setOfProducerTracersFromInjectors( flowSol, injectorTracers, timeStepIndices );
}
