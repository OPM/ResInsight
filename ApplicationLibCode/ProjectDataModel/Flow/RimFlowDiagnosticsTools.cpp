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

    if ( lhs.endsWith( "-XF" ) && !rhs.endsWith( "-XF" ) )
    {
        return false;
    }

    return lhs < rhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFlowDiagnosticsTools::calcOptionsForSelectedTracerField( FDS* flowSol, bool isInjector )
{
    if ( !flowSol ) return {};

    QList<caf::PdmOptionItemInfo> options;
    std::set<QString, TracerComp> sortedTracers = setOfTracersOfType( flowSol, isInjector );
    for ( const QString& tracerName : sortedTracers )
    {
        QString               postfix;
        FDS::TracerStatusType status = flowSol->tracerStatusOverall( tracerName );
        if ( status == FDS::TracerStatusType::VARYING )
        {
            postfix = " [I/P]";
        }
        else if ( status == FDS::TracerStatusType::UNDEFINED )
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
std::set<QString, RimFlowDiagnosticsTools::TracerComp> RimFlowDiagnosticsTools::setOfTracersOfType( const FDS* flowSol, bool isInjector )
{
    if ( !flowSol ) return {};

    std::set<QString, TracerComp> sortedTracers;
    std::vector<QString>          tracerNames = flowSol->tracerNames();
    for ( const QString& tracerName : tracerNames )
    {
        FDS::TracerStatusType status        = flowSol->tracerStatusOverall( tracerName );
        bool                  includeTracer = status == FDS::TracerStatusType::VARYING || status == FDS::TracerStatusType::UNDEFINED;
        includeTracer |= isInjector && status == FDS::TracerStatusType::INJECTOR;
        includeTracer |= !isInjector && status == FDS::TracerStatusType::PRODUCER;

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
std::vector<QString> RimFlowDiagnosticsTools::producerTracersInTimeStep( const FDS* flowSol, int timeStepIndex )
{
    return tracersOfStatusInTimeStep( flowSol, FDS::TracerStatusType::PRODUCER, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagnosticsTools::injectorTracersInTimeStep( const FDS* flowSol, int timeStepIndex )
{
    return tracersOfStatusInTimeStep( flowSol, FDS::TracerStatusType::INJECTOR, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFlowDiagnosticsTools::tracersOfStatusInTimeStep( const FDS* flowSol, FDS::TracerStatusType status, int timeStepIndex )
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
    RimFlowDiagnosticsTools::setOfInjectorTracersFromProducers( FDS*                        flowSol,
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
    RimFlowDiagnosticsTools::setOfInjectorTracersFromProducers( FDS* flowSol, const std::vector<QString>& producerTracers, int timeStepIndex )
{
    if ( timeStepIndex < 0 ) return {};

    const auto timeStepIndices = std::vector<int>( { timeStepIndex } );
    return setOfInjectorTracersFromProducers( flowSol, producerTracers, timeStepIndices );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    RimFlowDiagnosticsTools::setOfProducerTracersFromInjectors( FDS*                        flowSol,
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
    RimFlowDiagnosticsTools::setOfProducerTracersFromInjectors( FDS* flowSol, const std::vector<QString>& injectorTracers, int timeStepIndex )
{
    if ( timeStepIndex < 0 ) return {};

    const auto timeStepIndices = std::vector<int>( { timeStepIndex } );
    return setOfProducerTracersFromInjectors( flowSol, injectorTracers, timeStepIndices );
}
