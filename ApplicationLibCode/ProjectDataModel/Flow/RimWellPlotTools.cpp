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

#include "RimWellPlotTools.h"

#include "RiaQDateTimeTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "Well/RigSimWellData.h"

#include "RimDepthTrackPlot.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimOilField.h"
#include "RimPressureDepthData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimWellLogChannel.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurve.h"
#include "RimWellLogRftCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellRftPlot.h"

#include <regex>

namespace RimWellPlotTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::set<QString> PRESSURE_DATA_NAMES = { "PRESSURE", "PRES_FORM" };

const std::set<QString> OIL_CHANNEL_NAMES   = { "QOZT", "QOIL", "^.*\\D_QOIL" };
const std::set<QString> GAS_CHANNEL_NAMES   = { "QGZT", "QGAS", "^.*\\D_QGAS" };
const std::set<QString> WATER_CHANNEL_NAMES = { "QWZT", "QWAT", "^.*\\D_QWAT" };
const std::set<QString> TOTAL_CHANNEL_NAMES = { "QTZT", "QTOT", "^.*\\D_QTOT" };

std::set<QString> FLOW_DATA_NAMES = {};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool tryMatchChannelName( const std::set<QString>& channelNames, const QString& channelNameToMatch )
{
    auto itr = std::find_if( channelNames.begin(),
                             channelNames.end(),
                             [&]( const QString& channelName )
                             {
                                 if ( channelName.startsWith( '^' ) )
                                 {
                                     std::regex pattern( channelName.toStdString() );
                                     return std::regex_match( channelNameToMatch.toStdString(), pattern );
                                 }
                                 else
                                 {
                                     return (bool)channelName.contains( channelNameToMatch, Qt::CaseInsensitive );
                                 }
                             } );
    return itr != channelNames.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class StaticFieldsInitializer
{
public:
    StaticFieldsInitializer()
    {
        // Init static list
        FLOW_DATA_NAMES.insert( OIL_CHANNEL_NAMES.begin(), OIL_CHANNEL_NAMES.end() );
        FLOW_DATA_NAMES.insert( GAS_CHANNEL_NAMES.begin(), GAS_CHANNEL_NAMES.end() );
        FLOW_DATA_NAMES.insert( WATER_CHANNEL_NAMES.begin(), WATER_CHANNEL_NAMES.end() );
        FLOW_DATA_NAMES.insert( TOTAL_CHANNEL_NAMES.begin(), TOTAL_CHANNEL_NAMES.end() );
    }
} staticFieldsInitializer;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isPressureChannel( RimWellLogChannel* channel )
{
    for ( const auto& pressureDataName : PRESSURE_DATA_NAMES )
    {
        if ( QString::compare( channel->name(), pressureDataName, Qt::CaseInsensitive ) == 0 ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasPressureData( const RimWellLogFile* wellLogFile )
{
    for ( RimWellLogChannel* const wellLogChannel : wellLogFile->wellLogChannels() )
    {
        if ( isPressureChannel( wellLogChannel ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasPressureData( RimWellPath* wellPath )
{
    for ( RimWellLogFile* const wellLogFile : wellPath->wellLogFiles() )
    {
        if ( hasPressureData( wellLogFile ) )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<RigEclipseResultAddress, QString> pressureResultDataInfo( const RigEclipseCaseData* eclipseCaseData )
{
    if ( eclipseCaseData != nullptr )
    {
        for ( const auto& pressureDataName : PRESSURE_DATA_NAMES )
        {
            if ( eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                     ->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, pressureDataName ) ) )
            {
                return std::make_pair( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, pressureDataName ),
                                       pressureDataName );
            }
        }
    }
    return std::make_pair( RigEclipseResultAddress(), "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasPressureData( RimEclipseResultCase* gridCase )
{
    return pressureResultDataInfo( gridCase->eclipseCaseData() ).first.isValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isFlowChannel( RimWellLogChannel* channel )
{
    return tryMatchChannelName( FLOW_DATA_NAMES, channel->name() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasFlowData( const RimWellLog* wellLog )
{
    for ( RimWellLogChannel* const wellLogChannel : wellLog->wellLogChannels() )
    {
        if ( isFlowChannel( wellLogChannel ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasFlowData( const RimWellPath* wellPath )
{
    for ( RimWellLogFile* const wellLogFile : wellPath->wellLogFiles() )
    {
        if ( hasFlowData( wellLogFile ) )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasAssociatedWellPath( const QString& wellName )
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( wellName );

    return wellPath != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isOilFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( OIL_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isGasFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( GAS_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isWaterFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( WATER_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isTotalFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( TOTAL_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasFlowData( RimEclipseResultCase* gridCase )
{
    const RigEclipseCaseData* const eclipseCaseData = gridCase->eclipseCaseData();

    for ( const QString& channelName : FLOW_DATA_NAMES )
    {
        if ( eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                 ->hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, channelName ) ) )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FlowPhase flowPhaseFromChannelName( const QString& channelName )
{
    if ( tryMatchChannelName( OIL_CHANNEL_NAMES, channelName ) ) return FlowPhase::FLOW_PHASE_OIL;
    if ( tryMatchChannelName( GAS_CHANNEL_NAMES, channelName ) ) return FlowPhase::FLOW_PHASE_GAS;
    if ( tryMatchChannelName( WATER_CHANNEL_NAMES, channelName ) ) return FlowPhase::FLOW_PHASE_WATER;
    if ( tryMatchChannelName( TOTAL_CHANNEL_NAMES, channelName ) ) return FlowPhase::FLOW_PHASE_TOTAL;
    return FlowPhase::FLOW_PHASE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void addTimeStepsToMap( std::map<QDateTime, std::set<RifDataSourceForRftPlt>>&       destMap,
                        const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsToAdd )
{
    for ( const auto& timeStepPair : timeStepsToAdd )
    {
        if ( timeStepPair.first.isValid() )
        {
            auto addresses = timeStepPair.second;
            destMap[timeStepPair.first].insert( addresses.begin(), addresses.end() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> wellLogFilesContainingPressure( const QString& wellPathNameOrSimWellName )
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const      project   = RimProject::current();
    std::vector<RimWellPath*>    wellPaths = project->allWellPaths();

    for ( auto wellPath : wellPaths )
    {
        if ( !wellPathNameOrSimWellName.isEmpty() &&
             ( wellPathNameOrSimWellName == wellPath->associatedSimulationWellName() || wellPathNameOrSimWellName == wellPath->name() ) )
        {
            const std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for ( RimWellLogFile* file : files )
            {
                if ( hasPressureData( file ) )
                {
                    wellLogFiles.push_back( file );
                }
            }
        }
    }

    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogChannel* getPressureChannelFromWellFile( const RimWellLogFile* wellLogFile )
{
    if ( wellLogFile != nullptr )
    {
        for ( RimWellLogChannel* const channel : wellLogFile->wellLogChannels() )
        {
            if ( isPressureChannel( channel ) )
            {
                return channel;
            }
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> wellLogFilesContainingFlow( const QString& wellPathName )
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const      project   = RimProject::current();
    std::vector<RimWellPath*>    wellPaths = project->allWellPaths();

    for ( auto wellPath : wellPaths )
    {
        if ( wellPath->name() == wellPathName )
        {
            std::vector<RimWellLogFile*> files = wellPath->wellLogFiles();

            for ( RimWellLogFile* file : files )
            {
                if ( hasFlowData( file ) )
                {
                    wellLogFiles.push_back( file );
                }
            }
        }
    }
    return wellLogFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* wellPathFromWellLogFile( const RimWellLogFile* wellLogFile )
{
    RimProject* const project = RimProject::current();
    for ( const auto& oilField : project->oilFields )
    {
        for ( const auto& wellPath : oilField->wellPathCollection()->allWellPaths() )
        {
            for ( RimWellLogFile* const file : wellPath->wellLogFiles() )
            {
                if ( file == wellLogFile )
                {
                    return wellPath;
                }
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> gridCasesForWell( const QString& simWellName )
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject*                  project = RimProject::current();

    for ( RimEclipseCase* eclCase : project->eclipseCases() )
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>( eclCase );
        if ( resultCase && resultCase->eclipseCaseData() )
        {
            if ( eclCase->eclipseCaseData()->findSimWellData( simWellName ) )
            {
                cases.push_back( resultCase );
            }
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> rftCasesForWell( const QString& simWellName )
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject*                  project = RimProject::current();

    for ( RimEclipseCase* eclCase : project->eclipseCases() )
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>( eclCase );

        if ( resultCase && resultCase->rftReader() && resultCase->rftReader()->wellNames().count( simWellName ) )
        {
            cases.push_back( resultCase );
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> rftTimeStepsForWell( const std::vector<RimSummaryCase*>& summaryCases, const QString& wellName )
{
    std::set<QDateTime> allTimeSteps;
    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( reader )
        {
            std::set<QDateTime> timeStep = reader->availableTimeSteps( wellName );
            allTimeSteps.insert( timeStep.begin(), timeStep.end() );
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> rftEnsemblesForWell( const QString& simWellName )
{
    const RimProject* project = RimProject::current();

    std::vector<RimSummaryEnsemble*> ensembles = project->summaryEnsembles();

    std::vector<RimSummaryEnsemble*> rftEnsembles;

    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        if ( ensemble && ensemble->isEnsemble() &&
             !rftTimeStepsForWell( ensemble->allSummaryCases(), simWellName ).empty() )
        {
            rftEnsembles.push_back( ensemble );
        }
    }
    return rftEnsembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> rftEnsembles()
{
    const RimProject* project = RimProject::current();

    std::vector<RimSummaryEnsemble*> ensembles = project->summaryEnsembles();

    std::vector<RimSummaryEnsemble*> rftEnsembles;

    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        if ( ensemble && ensemble->isEnsemble() &&
             !RimSummaryEnsembleTools::wellsWithRftData( ensemble->allSummaryCases() ).empty() )
        {
            rftEnsembles.push_back( ensemble );
        }
    }
    return rftEnsembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedFmuRftData*> observedFmuRftDataForWell( const QString& simWellName )
{
    std::vector<RimObservedFmuRftData*> observedDataForWell;
    std::vector<RimObservedFmuRftData*> allObservedData = observedFmuRftData();
    for ( RimObservedFmuRftData* observedData : allObservedData )
    {
        if ( observedData->hasWell( simWellName ) )
        {
            observedDataForWell.push_back( observedData );
        }
    }
    return observedDataForWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedFmuRftData*> observedFmuRftData()
{
    const RimProject*          project                = RimProject::current();
    RimObservedDataCollection* observedDataCollection = project->activeOilField() ? project->activeOilField()->observedDataCollection()
                                                                                  : nullptr;

    if ( observedDataCollection )
    {
        return observedDataCollection->allObservedFmuRftData();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPressureDepthData*> pressureDepthDataForWell( const QString& simWellName )
{
    std::vector<RimPressureDepthData*> observedDataForWell;
    std::vector<RimPressureDepthData*> allObservedData = pressureDepthData();
    for ( RimPressureDepthData* observedData : allObservedData )
    {
        if ( observedData->hasWell( simWellName ) )
        {
            observedDataForWell.push_back( observedData );
        }
    }
    return observedDataForWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void loadDataAndUpdateDepthTrackPlots( const std::set<RimDepthTrackPlot*>& depthTrackPlots )
{
    for ( auto plot : depthTrackPlots )
    {
        plot->updateConnectedEditors();

        if ( auto rftPlot = dynamic_cast<RimWellRftPlot*>( plot ) )
        {
            rftPlot->rebuildCurves();
        }

        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPressureDepthData*> pressureDepthData()
{
    const RimProject*          project                = RimProject::current();
    RimObservedDataCollection* observedDataCollection = project->activeOilField() ? project->activeOilField()->observedDataCollection()
                                                                                  : nullptr;

    if ( observedDataCollection )
    {
        return observedDataCollection->allPressureDepthData();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMapFromGridCase( RimEclipseCase* gridCase )
{
    const RigEclipseCaseData* const             eclipseCaseData = gridCase->eclipseCaseData();
    std::pair<RigEclipseResultAddress, QString> resultDataInfo  = pressureResultDataInfo( eclipseCaseData );

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeStepsMap;
    if ( resultDataInfo.first.isValid() )
    {
        for ( const QDateTime& timeStep :
              eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates( resultDataInfo.first ) )
        {
            if ( timeStepsMap.count( timeStep ) == 0 )
            {
                timeStepsMap.insert( std::make_pair( timeStep, std::set<RifDataSourceForRftPlt>() ) );
            }
            timeStepsMap[timeStep].insert( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, gridCase ) );
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> findMatchingOrAdjacentTimeSteps( const std::set<QDateTime>& baseTimeLine, const std::set<QDateTime>& availableTimeSteps )
{
    std::set<QDateTime> resultTimeSteps;
    for ( const QDateTime& baseTimeStep : baseTimeLine )
    {
        auto itToEqualOrLargerTime = availableTimeSteps.lower_bound( baseTimeStep );
        if ( itToEqualOrLargerTime != availableTimeSteps.end() )
        {
            resultTimeSteps.insert( *itToEqualOrLargerTime );
            if ( *itToEqualOrLargerTime != baseTimeStep && itToEqualOrLargerTime != availableTimeSteps.begin() )
            {
                // Found a larger time, then add the timestep before it as the adjacent timestep before the base timestep
                itToEqualOrLargerTime--;
                resultTimeSteps.insert( *itToEqualOrLargerTime );
            }
        }
    }

    // The above will only work if there are at least one available timestep equal or after any of the basetimeline
    // times. If no timesteps matched but we have some, add the last available because the above code missed it.

    if ( resultTimeSteps.empty() && !baseTimeLine.empty() && !availableTimeSteps.empty() )
    {
        resultTimeSteps.insert( *availableTimeSteps.rbegin() );
    }

    return resultTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> availableSimWellTimesteps( RimEclipseCase* eclCase, const QString& simWellName, bool addFirstReportTimestep )
{
    if ( !eclCase || !eclCase->eclipseCaseData() ) return {};

    std::set<QDateTime> availebleTimeSteps;

    const RigSimWellData* simWell = eclCase->eclipseCaseData()->findSimWellData( simWellName );

    if ( simWell )
    {
        std::vector<QDateTime> allTimeSteps =
            eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();

        for ( size_t tsIdx = 0; tsIdx < allTimeSteps.size(); ++tsIdx )
        {
            if ( simWell->hasWellResult( tsIdx ) || ( addFirstReportTimestep && tsIdx == 0 ) )
            {
                availebleTimeSteps.insert( allTimeSteps[tsIdx] );
            }
        }
    }

    return availebleTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* rftReaderInterface( RimEclipseCase* eclipseCase )
{
    auto eclResCase = dynamic_cast<RimEclipseResultCase*>( eclipseCase );

    if ( eclResCase ) return eclResCase->rftReader();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRftPltCurveDefinition curveDefFromCurve( const RimWellLogCurve* curve )
{
    const RimWellLogRftCurve*        rftCurve         = dynamic_cast<const RimWellLogRftCurve*>( curve );
    const RimWellLogExtractionCurve* gridCurve        = dynamic_cast<const RimWellLogExtractionCurve*>( curve );
    const RimWellLogLasFileCurve*    wellLogFileCurve = dynamic_cast<const RimWellLogLasFileCurve*>( curve );

    if ( rftCurve != nullptr )
    {
        RimEclipseResultCase*  rftCase           = dynamic_cast<RimEclipseResultCase*>( rftCurve->eclipseCase() );
        RimSummaryCase*        rftSummaryCase    = rftCurve->summaryCase();
        RimSummaryEnsemble*    rftEnsemble       = rftCurve->ensemble();
        RimObservedFmuRftData* rftFmuData        = rftCurve->observedFmuRftData();
        RimPressureDepthData*  pressureDepthData = rftCurve->pressureDepthData();

        const RifEclipseRftAddress rftAddress = rftCurve->rftAddress();
        const QString&             wellName   = rftAddress.wellName();
        const QDateTime&           timeStep   = rftAddress.timeStep();

        if ( rftSummaryCase != nullptr )
        {
            // Presens of rftSummaryCase must be tested before rftCase because a rftSummaryCase can have a rftCase
            // The rftCase is used to extract TVD/MD from the grid model

            RimSummaryEnsemble* parentEnsemble = rftSummaryCase->firstAncestorOrThisOfType<RimSummaryEnsemble>();
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( rftSummaryCase, parentEnsemble, rftCase ), wellName, timeStep );
        }
        else if ( rftCase != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA, rftCase ),
                                             wellName,
                                             timeStep );
        }
        else if ( rftEnsemble != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( rftEnsemble ), wellName, timeStep );
        }
        else if ( rftFmuData != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( rftFmuData ), wellName, timeStep );
        }
        else if ( pressureDepthData != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( pressureDepthData ), wellName, timeStep );
        }
    }
    else if ( gridCurve != nullptr )
    {
        RimEclipseResultCase* gridCase = dynamic_cast<RimEclipseResultCase*>( gridCurve->rimCase() );
        if ( gridCase != nullptr )
        {
            size_t                                                       timeStepIndex = gridCurve->currentTimeStep();
            const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsMap  = timeStepsMapFromGridCase( gridCase );
            auto                                                         timeStepsVector =
                std::vector<std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>>( timeStepsMap.begin(), timeStepsMap.end() );
            if ( timeStepIndex < timeStepsMap.size() )
            {
                return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA, gridCase ),
                                                 gridCurve->wellName(),
                                                 timeStepsVector[timeStepIndex].first );
            }
        }
    }
    else if ( wellLogFileCurve != nullptr )
    {
        RimWellLog* const wellLog = wellLogFileCurve->wellLog();

        if ( wellLog != nullptr )
        {
            const QDateTime date = wellLog->date();
            if ( date.isValid() )
            {
                if ( auto wellLogLasFile = dynamic_cast<RimWellLogLasFile*>( wellLog ) )
                {
                    return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( wellLogLasFile ), wellLog->wellName(), date );
                }
            }
        }
    }
    return RiaRftPltCurveDefinition( RifDataSourceForRftPlt(), QString(), QDateTime() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* wellPathByWellPathNameOrSimWellName( const QString& wellPathNameOrSimwellName )
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( wellPathNameOrSimwellName );

    return wellPath != nullptr ? wellPath : proj->wellPathFromSimWellName( wellPathNameOrSimwellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> wellPathsContainingFlow()
{
    std::vector<RimWellPath*> wellPaths;
    for ( RimWellPath* wellPath : RimProject::current()->allWellPaths() )
    {
        if ( wellPath->wellPathGeometry() || hasFlowData( wellPath ) ) wellPaths.push_back( wellPath );
    }

    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString simWellName( const QString& wellPathNameOrSimWellName )
{
    RimWellPath* wellPath = wellPathByWellPathNameOrSimWellName( wellPathNameOrSimWellName );
    return wellPath != nullptr ? wellPath->associatedSimulationWellName() : wellPathNameOrSimWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaRftPltCurveDefinition> curveDefsFromTimesteps( const QString&                             wellPathNameOrSimWellName,
                                                           const std::vector<QDateTime>&              selectedTimeSteps,
                                                           bool                                       firstSimWellTimeStepIsValid,
                                                           const std::vector<RifDataSourceForRftPlt>& selectedSourcesExpanded,
                                                           const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults )
{
    std::set<RiaRftPltCurveDefinition> curveDefs;

    std::set<QDateTime> selectedTimeStepSet( selectedTimeSteps.begin(), selectedTimeSteps.end() );

    const QString simWellName = RimWellPlotTools::simWellName( wellPathNameOrSimWellName );

    for ( const RifDataSourceForRftPlt& addr : selectedSourcesExpanded )
    {
        auto rftReader = rftReaderInterface( addr.eclCase() );
        if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA && rftReader )
        {
            std::set<QDateTime> rftTimes = rftReader->availableTimeSteps( simWellName, interestingRFTResults );
            for ( const QDateTime& time : rftTimes )
            {
                if ( selectedTimeStepSet.count( time ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, time ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA && addr.eclCase() )
        {
            std::set<QDateTime> timeSteps = availableSimWellTimesteps( addr.eclCase(), simWellName, firstSimWellTimeStepIsValid );

            for ( const QDateTime& time : timeSteps )
            {
                if ( selectedTimeStepSet.count( time ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, time ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE )
        {
            if ( addr.wellLogFile() )
            {
                if ( selectedTimeStepSet.count( addr.wellLogFile()->date() ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, addr.wellLogFile()->date() ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT )
        {
            RimObservedFmuRftData* observedFmuRftData = addr.observedFmuRftData();
            if ( observedFmuRftData && observedFmuRftData->rftReader() )
            {
                std::set<QDateTime> timeSteps = observedFmuRftData->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                for ( const QDateTime& time : timeSteps )
                {
                    if ( selectedTimeStepSet.count( time ) )
                    {
                        curveDefs.insert( RiaRftPltCurveDefinition( addr, wellPathNameOrSimWellName, time ) );
                    }
                }
            }

            RimPressureDepthData* pressureDepthData = addr.pressureDepthData();
            if ( pressureDepthData && pressureDepthData->rftReader() )
            {
                std::set<QDateTime> timeSteps = pressureDepthData->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                for ( const QDateTime& time : timeSteps )
                {
                    if ( selectedTimeStepSet.count( time ) )
                    {
                        curveDefs.insert( RiaRftPltCurveDefinition( addr, wellPathNameOrSimWellName, time ) );
                    }
                }
            }
        }
        else if ( addr.ensemble() )
        {
            // Add individual summary curves
            for ( RimSummaryCase* summaryCase : addr.ensemble()->allSummaryCases() )
            {
                if ( summaryCase && summaryCase->rftReader() )
                {
                    RifDataSourceForRftPlt summaryAddr( summaryCase, addr.ensemble(), addr.eclCase() );

                    std::set<QDateTime> timeSteps = summaryCase->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                    for ( const QDateTime& time : timeSteps )
                    {
                        if ( selectedTimeStepSet.count( time ) )
                        {
                            curveDefs.insert( RiaRftPltCurveDefinition( summaryAddr, wellPathNameOrSimWellName, time ) );
                        }
                    }
                }
            }
            // Add statistics curves
            if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT )
            {
                std::set<QDateTime> statTimeSteps = rftTimeStepsForWell( addr.ensemble()->allSummaryCases(), wellPathNameOrSimWellName );
                for ( const QDateTime& time : statTimeSteps )
                {
                    if ( selectedTimeStepSet.count( time ) )
                    {
                        curveDefs.insert( RiaRftPltCurveDefinition( addr, wellPathNameOrSimWellName, time ) );
                    }
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT )
        {
            auto summaryCase = addr.summaryCase();
            if ( summaryCase && summaryCase->rftReader() )
            {
                std::set<QDateTime> timeSteps = summaryCase->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                for ( const QDateTime& time : timeSteps )
                {
                    if ( selectedTimeStepSet.count( time ) )
                    {
                        curveDefs.insert( RiaRftPltCurveDefinition( addr, wellPathNameOrSimWellName, time ) );
                    }
                }
            }
        }
    }

    return curveDefs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowPlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem )
{
    QString axisTitle;

    if ( condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR )
    {
        QString unitText = flowUnitText( condition, unitSystem );

        axisTitle = "Reservoir Flow Rate " + unitText;
    }
    else
    {
        QString unitText = flowUnitText( condition, unitSystem );

        axisTitle = "Surface Flow Rate " + unitText;
    }

    return axisTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowConditionReservoirUnitText( RiaDefines::EclipseUnitSystem unitSystem )
{
    QString unitText;

    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            unitText = "[m<sup>3</sup>/day]";
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            unitText = "[Brl/day]";
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_LAB:
            unitText = "[cm<sup>3</sup>/hr]";
            break;
        default:
            break;
    }

    return unitText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem )
{
    QString unitText;

    if ( condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR )
    {
        unitText = flowConditionReservoirUnitText( unitSystem );
    }
    else
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
                break;
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                unitText = "[Liquid BBL/day], [Gas BOE/day]";
                break;
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                unitText = "[cm<sup>3</sup>/hr]";
                break;
            default:
                break;
        }
    }

    return unitText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowVolumePlotAxisTitle( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem )
{
    QString axisTitle;

    if ( condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR )
    {
        QString unitText = flowVolumeUnitText( condition, unitSystem );

        axisTitle = "Reservoir Flow Volume " + unitText;
    }
    else
    {
        QString unitText = flowVolumeUnitText( condition, unitSystem );

        axisTitle = "Surface Flow Volume " + unitText;
    }

    return axisTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowVolumeConditionReservoirUnitText( RiaDefines::EclipseUnitSystem unitSystem )
{
    QString unitText;

    switch ( unitSystem )
    {
        case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
            unitText = "[m<sup>3</sup>]";
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
            unitText = "[Brl]";
            break;
        case RiaDefines::EclipseUnitSystem::UNITS_LAB:
            unitText = "[cm<sup>3</sup>]";
            break;
        default:
            break;
    }

    return unitText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowVolumeUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem )
{
    QString unitText;

    if ( condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR )
    {
        unitText = flowVolumeConditionReservoirUnitText( unitSystem );
    }
    else
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                unitText = "[Liquid Sm<sup>3</sup>], [Gas kSm<sup>3</sup>]";
                break;
            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                unitText = "[Liquid BBL], [Gas BOE]";
                break;
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                unitText = "[cm<sup>3</sup>]";
                break;
            default:
                break;
        }
    }

    return unitText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString curveUnitText( RimWellLogLasFile::WellFlowCondition condition, RiaDefines::EclipseUnitSystem unitSystem, FlowPhase flowPhase )
{
    QString unitText;

    if ( condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR )
    {
        unitText = flowConditionReservoirUnitText( unitSystem );
    }
    else
    {
        switch ( unitSystem )
        {
            case RiaDefines::EclipseUnitSystem::UNITS_METRIC:
                switch ( flowPhase )
                {
                    case FlowPhase::FLOW_PHASE_GAS:
                        unitText = "[kSm<sup>3</sup>/day]";
                        break;
                    case FlowPhase::FLOW_PHASE_WATER: // Intentionally fall through, water and oil have same unit
                    case FlowPhase::FLOW_PHASE_OIL:
                        unitText = "[Sm<sup>3</sup>/day]";
                        break;
                    default:
                        unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
                        break;
                }
                break;

            case RiaDefines::EclipseUnitSystem::UNITS_FIELD:
                switch ( flowPhase )
                {
                    case FlowPhase::FLOW_PHASE_GAS:
                        unitText = "[BOE/day]";
                        break;
                    case FlowPhase::FLOW_PHASE_WATER: // Intentionally fall through, water and oil have same unit
                    case FlowPhase::FLOW_PHASE_OIL:
                        unitText = "[BBL/day]";
                        break;
                    default:
                        unitText = "[Liquid BBL/day], [Gas BOE/day]";
                        break;
                }
                break;
            case RiaDefines::EclipseUnitSystem::UNITS_LAB:
                unitText = "[cm<sup>3</sup>/hr]";
                break;
            default:
                break;
        }
    }

    return unitText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>>
    calculateRelevantTimeStepsFromCases( const QString&                                               wellPathNameOrSimWellName,
                                         const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults )
{
    bool addFirstTimestep = ( interestingRFTResults.count( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE ) == 1 );

    const QString simWellName = RimWellPlotTools::simWellName( wellPathNameOrSimWellName );

    bool hasObservedData   = false;
    bool hasRftData        = false;
    bool hasGridData       = false;
    bool hasEnsembleData   = false;
    bool hasSummaryRftData = false;

    for ( const auto& source : selSources )
    {
        switch ( source.sourceType() )
        {
            case RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA:
                hasRftData = true;
                break;
            case RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA:
                hasGridData = true;
                break;
            case RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE:
            case RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT:
                hasObservedData = true;
                break;
            case RifDataSourceForRftPlt::SourceType::SUMMARY_RFT:
                hasSummaryRftData = true;
                break;
            case RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT:
                hasEnsembleData = true;
                break;
        }
    }

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> observedTimeStepsWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> rftTimeStepsWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> gridTimestepsWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> summaryRftTimeStepsWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> ensembleTimeStepsWithSources;

    if ( hasObservedData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE && source.wellLogFile() )
            {
                observedTimeStepsWithSources[source.wellLogFile()->date()].insert( source );
            }
            else if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT )
            {
                {
                    if ( source.observedFmuRftData() )
                    {
                        std::set<QDateTime> rftFmuTimes =
                            source.observedFmuRftData()->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                        for ( const QDateTime& date : rftFmuTimes )
                        {
                            observedTimeStepsWithSources[date].insert( source );
                        }
                    }
                    else if ( source.pressureDepthData() )
                    {
                        std::set<QDateTime> rftFmuTimes =
                            source.pressureDepthData()->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                        for ( const QDateTime& date : rftFmuTimes )
                        {
                            observedTimeStepsWithSources[date].insert( source );
                        }
                    }
                }
            }
        }

        hasObservedData = !observedTimeStepsWithSources.empty();
    }

    if ( hasRftData )
    {
        for ( const auto& source : selSources )
        {
            auto rftReader = rftReaderInterface( source.eclCase() );
            if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA && rftReader )
            {
                std::set<QDateTime> rftTimes = rftReader->availableTimeSteps( simWellName, interestingRFTResults );
                for ( const QDateTime& date : rftTimes )
                {
                    rftTimeStepsWithSources[date].insert( source );
                }
            }
        }

        hasRftData = !rftTimeStepsWithSources.empty();
    }

    if ( hasGridData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA && source.eclCase() )
            {
                std::set<QDateTime> wellTimeSteps = availableSimWellTimesteps( source.eclCase(), simWellName, addFirstTimestep );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    gridTimestepsWithSources[date].insert( source );
                }
            }
        }

        hasGridData = !gridTimestepsWithSources.empty();
    }

    if ( hasSummaryRftData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::SUMMARY_RFT && source.summaryCase() &&
                 source.summaryCase()->rftReader() )
            {
                std::set<QDateTime> wellTimeSteps = source.summaryCase()->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    summaryRftTimeStepsWithSources[date].insert( source );
                }
            }
        }

        hasSummaryRftData = !summaryRftTimeStepsWithSources.empty();
    }

    if ( hasEnsembleData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT && source.ensemble() )
            {
                std::set<QDateTime> wellTimeSteps = rftTimeStepsForWell( source.ensemble()->allSummaryCases(), wellPathNameOrSimWellName );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    ensembleTimeStepsWithSources[date].insert( source );
                }
            }
        }

        hasEnsembleData = !ensembleTimeStepsWithSources.empty();
    }

    // If we have a time baseline add the equal or adjacent grid timesteps

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timestepsToShowWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>> timeBaseline;

    if ( hasObservedData )
    {
        timeBaseline = observedTimeStepsWithSources;
    }
    else if ( hasRftData )
    {
        timeBaseline = rftTimeStepsWithSources;
    }
    else if ( hasSummaryRftData )
    {
        timeBaseline = summaryRftTimeStepsWithSources;
    }
    else if ( hasEnsembleData )
    {
        timeBaseline = ensembleTimeStepsWithSources;
    }

    if ( !timeBaseline.empty() )
    {
        std::set<QDateTime> baseTimeSteps;
        for ( const auto& dateSourceSetPair : timeBaseline )
            baseTimeSteps.insert( dateSourceSetPair.first );

        std::set<QDateTime> rftTimeSteps;
        for ( const auto& dateSourceSetPair : rftTimeStepsWithSources )
            rftTimeSteps.insert( dateSourceSetPair.first );

        std::set<QDateTime> gridTimeSteps;
        for ( const auto& dateSourceSetPair : gridTimestepsWithSources )
            gridTimeSteps.insert( dateSourceSetPair.first );

        std::set<QDateTime> summaryRftTimeSteps;
        for ( const auto& dateSourceSetPair : summaryRftTimeStepsWithSources )
            summaryRftTimeSteps.insert( dateSourceSetPair.first );

        std::set<QDateTime> ensembleRftTimeSteps;
        for ( const auto& dateSourceSetPair : ensembleTimeStepsWithSources )
            ensembleRftTimeSteps.insert( dateSourceSetPair.first );

        std::set<QDateTime> filteredRftTimeSteps         = findMatchingOrAdjacentTimeSteps( baseTimeSteps, rftTimeSteps );
        std::set<QDateTime> filteredGridTimeSteps        = findMatchingOrAdjacentTimeSteps( baseTimeSteps, gridTimeSteps );
        std::set<QDateTime> filteredEnsembleRftTimeSteps = findMatchingOrAdjacentTimeSteps( baseTimeSteps, ensembleRftTimeSteps );
        std::set<QDateTime> filteredSummaryRftTimeSteps  = findMatchingOrAdjacentTimeSteps( baseTimeSteps, summaryRftTimeSteps );

        if ( addFirstTimestep && !gridTimeSteps.empty() )
        {
            filteredGridTimeSteps.insert( *gridTimeSteps.begin() );
        }

        // Fill final map
        timestepsToShowWithSources = observedTimeStepsWithSources;

        std::set<QDateTime>& allFilteredTimesteps = filteredRftTimeSteps;
        allFilteredTimesteps.insert( filteredEnsembleRftTimeSteps.begin(), filteredEnsembleRftTimeSteps.end() );
        allFilteredTimesteps.insert( filteredGridTimeSteps.begin(), filteredGridTimeSteps.end() );
        allFilteredTimesteps.insert( filteredSummaryRftTimeSteps.begin(), filteredSummaryRftTimeSteps.end() );

        for ( const QDateTime& time : allFilteredTimesteps )
        {
            auto rftTimeSourceSetIt = rftTimeStepsWithSources.find( time );
            if ( rftTimeSourceSetIt != rftTimeStepsWithSources.end() )
            {
                std::set<RifDataSourceForRftPlt>& sourceSet = rftTimeSourceSetIt->second;
                timestepsToShowWithSources[time].insert( sourceSet.begin(), sourceSet.end() );
            }

            auto gridTimeSourceSetIt = gridTimestepsWithSources.find( time );
            if ( gridTimeSourceSetIt != gridTimestepsWithSources.end() )
            {
                std::set<RifDataSourceForRftPlt>& sourceSet = gridTimeSourceSetIt->second;
                timestepsToShowWithSources[time].insert( sourceSet.begin(), sourceSet.end() );
            }

            auto summaryRftTimeSourceSetIt = summaryRftTimeStepsWithSources.find( time );
            if ( summaryRftTimeSourceSetIt != summaryRftTimeStepsWithSources.end() )
            {
                std::set<RifDataSourceForRftPlt>& sourceSet = summaryRftTimeSourceSetIt->second;
                timestepsToShowWithSources[time].insert( sourceSet.begin(), sourceSet.end() );
            }

            auto ensembleRftTimeSourceSetIt = ensembleTimeStepsWithSources.find( time );
            if ( ensembleRftTimeSourceSetIt != ensembleTimeStepsWithSources.end() )
            {
                std::set<RifDataSourceForRftPlt>& sourceSet = ensembleRftTimeSourceSetIt->second;
                timestepsToShowWithSources[time].insert( sourceSet.begin(), sourceSet.end() );
            }
        }
    }
    else
    {
        timestepsToShowWithSources = gridTimestepsWithSources;
    }

    return timestepsToShowWithSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void calculateValueOptionsForTimeSteps( const QString&                                               wellPathNameOrSimWellName,
                                        const std::vector<RifDataSourceForRftPlt>&                   selSources,
                                        const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults,
                                        QList<caf::PdmOptionItemInfo>&                               options )
{
    auto timestepsToShowWithSources = calculateRelevantTimeStepsFromCases( wellPathNameOrSimWellName, selSources, interestingRFTResults );

    // Create formatted options of all the time steps
    QString dateFormatString;
    {
        std::vector<QDateTime> allTimeSteps;
        for ( const std::pair<const QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepPair : timestepsToShowWithSources )
        {
            allTimeSteps.push_back( timeStepPair.first );
        }
        dateFormatString = RiaQDateTimeTools::createTimeFormatStringFromDates( allTimeSteps );
    }

    for ( const std::pair<const QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepPair : timestepsToShowWithSources )
    {
        QString optionText = RiaQDateTimeTools::toStringUsingApplicationLocale( timeStepPair.first, dateFormatString );

        bool hasObs      = false;
        bool hasRft      = false;
        bool hasGrid     = false;
        bool hasEnsemble = false;

        for ( const auto& source : timeStepPair.second )
        {
            switch ( source.sourceType() )
            {
                case RifDataSourceForRftPlt::SourceType::OBSERVED_LAS_FILE:
                case RifDataSourceForRftPlt::SourceType::OBSERVED_FMU_RFT:
                    hasObs = true;
                    break;
                case RifDataSourceForRftPlt::SourceType::RFT_SIM_WELL_DATA:
                    hasRft = true;
                    break;
                case RifDataSourceForRftPlt::SourceType::GRID_MODEL_CELL_DATA:
                    hasGrid = true;
                    break;
                case RifDataSourceForRftPlt::SourceType::ENSEMBLE_RFT:
                    hasEnsemble = true;
                    break;
                case RifDataSourceForRftPlt::SourceType::SUMMARY_RFT:
                    hasRft = true;
                    break;
            }
        }

        QStringList optionTags;
        if ( hasObs ) optionTags << "O";
        if ( hasRft ) optionTags << "R";
        if ( hasGrid ) optionTags << "G";
        if ( hasEnsemble ) optionTags << "E";

        optionText += QString( " \t[%1]" ).arg( optionTags.join( ", " ) );
        options.push_back( caf::PdmOptionItemInfo( optionText, timeStepPair.first ) );
    }
}

} // namespace RimWellPlotTools
