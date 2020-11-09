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
#include "RiaWellNameComparer.h"

#include "RifReaderEclipseRft.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimObservedDataCollection.h"
#include "RimObservedFmuRftData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogRftCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include <regex>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::set<QString> RimWellPlotTools::PRESSURE_DATA_NAMES = { "PRESSURE", "PRES_FORM" };

const std::set<QString> RimWellPlotTools::OIL_CHANNEL_NAMES   = { "QOZT", "QOIL", "^.*\\D_QOIL" };
const std::set<QString> RimWellPlotTools::GAS_CHANNEL_NAMES   = { "QGZT", "QGAS", "^.*\\D_QGAS" };
const std::set<QString> RimWellPlotTools::WATER_CHANNEL_NAMES = { "QWZT", "QWAT", "^.*\\D_QWAT" };
const std::set<QString> RimWellPlotTools::TOTAL_CHANNEL_NAMES = { "QTZT", "QTOT", "^.*\\D_QTOT" };

std::set<QString> RimWellPlotTools::FLOW_DATA_NAMES = {};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class StaticFieldsInitializer
{
public:
    StaticFieldsInitializer()
    {
        // Init static list
        RimWellPlotTools::FLOW_DATA_NAMES.insert( RimWellPlotTools::OIL_CHANNEL_NAMES.begin(),
                                                  RimWellPlotTools::OIL_CHANNEL_NAMES.end() );
        RimWellPlotTools::FLOW_DATA_NAMES.insert( RimWellPlotTools::GAS_CHANNEL_NAMES.begin(),
                                                  RimWellPlotTools::GAS_CHANNEL_NAMES.end() );
        RimWellPlotTools::FLOW_DATA_NAMES.insert( RimWellPlotTools::WATER_CHANNEL_NAMES.begin(),
                                                  RimWellPlotTools::WATER_CHANNEL_NAMES.end() );
        RimWellPlotTools::FLOW_DATA_NAMES.insert( RimWellPlotTools::TOTAL_CHANNEL_NAMES.begin(),
                                                  RimWellPlotTools::TOTAL_CHANNEL_NAMES.end() );
    }
} staticFieldsInitializer;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasPressureData( const RimWellLogFile* wellLogFile )
{
    for ( RimWellLogFileChannel* const wellLogChannel : wellLogFile->wellLogChannels() )
    {
        if ( isPressureChannel( wellLogChannel ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasPressureData( RimWellPath* wellPath )
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
std::pair<RigEclipseResultAddress, QString>
    RimWellPlotTools::pressureResultDataInfo( const RigEclipseCaseData* eclipseCaseData )
{
    if ( eclipseCaseData != nullptr )
    {
        for ( const auto& pressureDataName : PRESSURE_DATA_NAMES )
        {
            if ( eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                     ->hasResultEntry(
                         RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, pressureDataName ) ) )
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
bool RimWellPlotTools::isPressureChannel( RimWellLogFileChannel* channel )
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
bool RimWellPlotTools::hasPressureData( RimEclipseResultCase* gridCase )
{
    return pressureResultDataInfo( gridCase->eclipseCaseData() ).first.isValid();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData( const RimWellLogFile* wellLogFile )
{
    for ( RimWellLogFileChannel* const wellLogChannel : wellLogFile->wellLogChannels() )
    {
        if ( isFlowChannel( wellLogChannel ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData( const RimWellPath* wellPath )
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
bool RimWellPlotTools::hasAssociatedWellPath( const QString& wellName )
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( wellName );

    return wellPath != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isFlowChannel( RimWellLogFileChannel* channel )
{
    return tryMatchChannelName( FLOW_DATA_NAMES, channel->name() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isOilFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( OIL_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isGasFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( GAS_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isWaterFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( WATER_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::isTotalFlowChannel( const QString& channelName )
{
    return tryMatchChannelName( TOTAL_CHANNEL_NAMES, channelName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::hasFlowData( RimEclipseResultCase* gridCase )
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
FlowPhase RimWellPlotTools::flowPhaseFromChannelName( const QString& channelName )
{
    if ( tryMatchChannelName( OIL_CHANNEL_NAMES, channelName ) ) return FLOW_PHASE_OIL;
    if ( tryMatchChannelName( GAS_CHANNEL_NAMES, channelName ) ) return FLOW_PHASE_GAS;
    if ( tryMatchChannelName( WATER_CHANNEL_NAMES, channelName ) ) return FLOW_PHASE_WATER;
    if ( tryMatchChannelName( TOTAL_CHANNEL_NAMES, channelName ) ) return FLOW_PHASE_TOTAL;
    return FLOW_PHASE_NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPlotTools::addTimeStepsToMap( std::map<QDateTime, std::set<RifDataSourceForRftPlt>>&       destMap,
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
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingPressure( const QString& wellPathNameOrSimWellName )
{
    std::vector<RimWellLogFile*> wellLogFiles;
    const RimProject* const      project   = RimProject::current();
    std::vector<RimWellPath*>    wellPaths = project->allWellPaths();

    for ( auto wellPath : wellPaths )
    {
        if ( !wellPathNameOrSimWellName.isEmpty() &&
             ( wellPathNameOrSimWellName == wellPath->associatedSimulationWellName() ||
               wellPathNameOrSimWellName == wellPath->name() ) )
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
RimWellLogFileChannel* RimWellPlotTools::getPressureChannelFromWellFile( const RimWellLogFile* wellLogFile )
{
    if ( wellLogFile != nullptr )
    {
        for ( RimWellLogFileChannel* const channel : wellLogFile->wellLogChannels() )
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
std::vector<RimWellLogFile*> RimWellPlotTools::wellLogFilesContainingFlow( const QString& wellPathName )
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
RimWellPath* RimWellPlotTools::wellPathFromWellLogFile( const RimWellLogFile* wellLogFile )
{
    RimProject* const project = RimProject::current();
    for ( const auto& oilField : project->oilFields )
    {
        auto wellPaths = std::vector<RimWellPath*>( oilField->wellPathCollection()->wellPaths.begin(),
                                                    oilField->wellPathCollection()->wellPaths.end() );

        for ( const auto& wellPath : wellPaths )
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
std::vector<RimEclipseResultCase*> RimWellPlotTools::gridCasesForWell( const QString& simWellName )
{
    std::vector<RimEclipseResultCase*> cases;
    const RimProject*                  project = RimProject::current();

    for ( RimEclipseCase* eclCase : project->eclipseCases() )
    {
        RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>( eclCase );
        if ( resultCase != nullptr )
        {
            if ( eclCase->eclipseCaseData()->findSimWellData( simWellName ) )
            {
                cases.push_back( resultCase );
                break;
            }
        }
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseResultCase*> RimWellPlotTools::rftCasesForWell( const QString& simWellName )
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
std::vector<RimSummaryCaseCollection*> RimWellPlotTools::rftEnsemblesForWell( const QString& simWellName )
{
    const RimProject* project = RimProject::current();

    std::vector<RimSummaryCaseCollection*> allSummaryCaseCollections = project->summaryGroups();

    std::vector<RimSummaryCaseCollection*> rftEnsembles;

    for ( RimSummaryCaseCollection* summaryCaseColl : allSummaryCaseCollections )
    {
        if ( summaryCaseColl && summaryCaseColl->isEnsemble() &&
             !summaryCaseColl->rftTimeStepsForWell( simWellName ).empty() )
        {
            rftEnsembles.push_back( summaryCaseColl );
        }
    }
    return rftEnsembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCaseCollection*> RimWellPlotTools::rftEnsembles()
{
    const RimProject* project = RimProject::current();

    std::vector<RimSummaryCaseCollection*> allSummaryCaseCollections = project->summaryGroups();

    std::vector<RimSummaryCaseCollection*> rftEnsembles;

    for ( RimSummaryCaseCollection* summaryCaseColl : allSummaryCaseCollections )
    {
        if ( summaryCaseColl && summaryCaseColl->isEnsemble() && !summaryCaseColl->wellsWithRftData().empty() )
        {
            rftEnsembles.push_back( summaryCaseColl );
        }
    }
    return rftEnsembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimObservedFmuRftData*> RimWellPlotTools::observedFmuRftDataForWell( const QString& simWellName )
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
std::vector<RimObservedFmuRftData*> RimWellPlotTools::observedFmuRftData()
{
    const RimProject*          project = RimProject::current();
    RimObservedDataCollection* observedDataCollection =
        project->activeOilField() ? project->activeOilField()->observedDataCollection() : nullptr;

    if ( observedDataCollection )
    {
        return observedDataCollection->allObservedFmuRftData();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> RimWellPlotTools::timeStepsMapFromGridCase( RimEclipseCase* gridCase )
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
            timeStepsMap[timeStep].insert( RifDataSourceForRftPlt( RifDataSourceForRftPlt::GRID, gridCase ) );
        }
    }
    return timeStepsMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellPlotTools::findMatchingOrAdjacentTimeSteps( const std::set<QDateTime>& baseTimeLine,
                                                                       const std::set<QDateTime>& availableTimeSteps )
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

    if ( !resultTimeSteps.size() && baseTimeLine.size() && availableTimeSteps.size() )
    {
        resultTimeSteps.insert( *availableTimeSteps.rbegin() );
    }

    return resultTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellPlotTools::availableSimWellTimesteps( RimEclipseCase* eclCase,
                                                                 const QString&  simWellName,
                                                                 bool            addFirstReportTimestep )
{
    std::set<QDateTime> availebleTimeSteps;

    if ( eclCase && eclCase->eclipseCaseData() )
    {
        std::vector<QDateTime> allTimeSteps =
            eclCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->timeStepDates();
        const RigSimWellData* simWell = eclCase->eclipseCaseData()->findSimWellData( simWellName );

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
RiaRftPltCurveDefinition RimWellPlotTools::curveDefFromCurve( const RimWellLogCurve* curve )
{
    const RimWellLogRftCurve*        rftCurve         = dynamic_cast<const RimWellLogRftCurve*>( curve );
    const RimWellLogExtractionCurve* gridCurve        = dynamic_cast<const RimWellLogExtractionCurve*>( curve );
    const RimWellLogFileCurve*       wellLogFileCurve = dynamic_cast<const RimWellLogFileCurve*>( curve );

    if ( rftCurve != nullptr )
    {
        RimEclipseResultCase*     rftCase        = dynamic_cast<RimEclipseResultCase*>( rftCurve->eclipseResultCase() );
        RimSummaryCase*           rftSummaryCase = rftCurve->summaryCase();
        RimSummaryCaseCollection* rftEnsemble    = rftCurve->ensemble();
        RimObservedFmuRftData*    rftFmuData     = rftCurve->observedFmuRftData();

        const RifEclipseRftAddress rftAddress = rftCurve->rftAddress();
        const QString&             wellName   = rftAddress.wellName();
        const QDateTime&           timeStep   = rftAddress.timeStep();

        if ( rftCase != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::RFT, rftCase ),
                                             wellName,
                                             timeStep );
        }
        else if ( rftSummaryCase != nullptr )
        {
            rftSummaryCase->firstAncestorOrThisOfTypeAsserted( rftEnsemble );
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::SUMMARY_RFT,
                                                                     rftSummaryCase,
                                                                     rftEnsemble ),
                                             wellName,
                                             timeStep );
        }
        else if ( rftEnsemble != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::ENSEMBLE_RFT, rftEnsemble ),
                                             wellName,
                                             timeStep );
        }
        else if ( rftFmuData != nullptr )
        {
            return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED_FMU_RFT, rftFmuData ),
                                             wellName,
                                             timeStep );
        }
    }
    else if ( gridCurve != nullptr )
    {
        RimEclipseResultCase* gridCase = dynamic_cast<RimEclipseResultCase*>( gridCurve->rimCase() );
        if ( gridCase != nullptr )
        {
            size_t                                                       timeStepIndex = gridCurve->currentTimeStep();
            const std::map<QDateTime, std::set<RifDataSourceForRftPlt>>& timeStepsMap =
                timeStepsMapFromGridCase( gridCase );
            auto timeStepsVector =
                std::vector<std::pair<QDateTime, std::set<RifDataSourceForRftPlt>>>( timeStepsMap.begin(),
                                                                                     timeStepsMap.end() );
            if ( timeStepIndex < timeStepsMap.size() )
            {
                return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::GRID, gridCase ),
                                                 gridCurve->wellName(),
                                                 timeStepsVector[timeStepIndex].first );
            }
        }
    }
    else if ( wellLogFileCurve != nullptr )
    {
        RimWellLogFile* const wellLogFile = wellLogFileCurve->wellLogFile();

        if ( wellLogFile != nullptr )
        {
            const QDateTime date = wellLogFile->date();

            if ( date.isValid() )
            {
                return RiaRftPltCurveDefinition( RifDataSourceForRftPlt( RifDataSourceForRftPlt::OBSERVED, wellLogFile ),
                                                 wellLogFile->wellName(),
                                                 date );
            }
        }
    }
    return RiaRftPltCurveDefinition( RifDataSourceForRftPlt(), QString(), QDateTime() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellPlotTools::wellPathByWellPathNameOrSimWellName( const QString& wellPathNameOrSimwellName )
{
    RimProject*  proj     = RimProject::current();
    RimWellPath* wellPath = proj->wellPathByName( wellPathNameOrSimwellName );

    return wellPath != nullptr ? wellPath : proj->wellPathFromSimWellName( wellPathNameOrSimwellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPlotTools::simWellName( const QString& wellPathNameOrSimWellName )
{
    RimWellPath* wellPath = wellPathByWellPathNameOrSimWellName( wellPathNameOrSimWellName );
    return wellPath != nullptr ? wellPath->associatedSimulationWellName() : wellPathNameOrSimWellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPlotTools::tryMatchChannelName( const std::set<QString>& channelNames, const QString& channelNameToMatch )
{
    auto itr = std::find_if( channelNames.begin(), channelNames.end(), [&]( const QString& channelName ) {
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
std::set<RiaRftPltCurveDefinition>
    RimWellPlotTools::curveDefsFromTimesteps( const QString&                             wellPathNameOrSimWellName,
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
        if ( addr.sourceType() == RifDataSourceForRftPlt::RFT && addr.rftReader() )
        {
            std::set<QDateTime> rftTimes = addr.rftReader()->availableTimeSteps( simWellName, interestingRFTResults );
            for ( const QDateTime& time : rftTimes )
            {
                if ( selectedTimeStepSet.count( time ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, time ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::GRID && addr.eclCase() )
        {
            std::set<QDateTime> timeSteps =
                RimWellPlotTools::availableSimWellTimesteps( addr.eclCase(), simWellName, firstSimWellTimeStepIsValid );

            for ( const QDateTime& time : timeSteps )
            {
                if ( selectedTimeStepSet.count( time ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, time ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::OBSERVED )
        {
            if ( addr.wellLogFile() )
            {
                if ( selectedTimeStepSet.count( addr.wellLogFile()->date() ) )
                {
                    curveDefs.insert( RiaRftPltCurveDefinition( addr, simWellName, addr.wellLogFile()->date() ) );
                }
            }
        }
        else if ( addr.sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT )
        {
            RimObservedFmuRftData* observedFmuRftData = addr.observedFmuRftData();
            if ( observedFmuRftData && observedFmuRftData->rftReader() )
            {
                std::set<QDateTime> timeSteps =
                    observedFmuRftData->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
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
                    RifDataSourceForRftPlt summaryAddr( RifDataSourceForRftPlt::SUMMARY_RFT, summaryCase, addr.ensemble() );

                    std::set<QDateTime> timeSteps =
                        summaryCase->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
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
            if ( addr.sourceType() == RifDataSourceForRftPlt::ENSEMBLE_RFT )
            {
                std::set<QDateTime> statTimeSteps = addr.ensemble()->rftTimeStepsForWell( wellPathNameOrSimWellName );
                for ( const QDateTime& time : statTimeSteps )
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
QString RimWellPlotTools::flowPlotAxisTitle( RimWellLogFile::WellFlowCondition condition,
                                             RiaEclipseUnitTools::UnitSystem   unitSystem )
{
    QString axisTitle;

    if ( condition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR )
    {
        QString unitText = RimWellPlotTools::flowUnitText( condition, unitSystem );

        axisTitle = "Reservoir Flow Rate " + unitText;
    }
    else
    {
        QString unitText = RimWellPlotTools::flowUnitText( condition, unitSystem );

        axisTitle = "Surface Flow Rate " + unitText;
    }

    return axisTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString flowConditionReservoirUnitText( RiaEclipseUnitTools::UnitSystem unitSystem )
{
    QString unitText;

    switch ( unitSystem )
    {
        case RiaEclipseUnitTools::UnitSystem::UNITS_METRIC:
            unitText = "[m<sup>3</sup>/day]";
            break;
        case RiaEclipseUnitTools::UnitSystem::UNITS_FIELD:
            unitText = "[Brl/day]";
            break;
        case RiaEclipseUnitTools::UnitSystem::UNITS_LAB:
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
QString RimWellPlotTools::flowUnitText( RimWellLogFile::WellFlowCondition condition,
                                        RiaEclipseUnitTools::UnitSystem   unitSystem )
{
    QString unitText;

    if ( condition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR )
    {
        unitText = flowConditionReservoirUnitText( unitSystem );
    }
    else
    {
        switch ( unitSystem )
        {
            case RiaEclipseUnitTools::UnitSystem::UNITS_METRIC:
                unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
                break;
            case RiaEclipseUnitTools::UnitSystem::UNITS_FIELD:
                unitText = "[Liquid BBL/day], [Gas BOE/day]";
                break;
            case RiaEclipseUnitTools::UnitSystem::UNITS_LAB:
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
QString RimWellPlotTools::curveUnitText( RimWellLogFile::WellFlowCondition condition,
                                         RiaEclipseUnitTools::UnitSystem   unitSystem,
                                         FlowPhase                         flowPhase )
{
    QString unitText;

    if ( condition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR )
    {
        unitText = flowConditionReservoirUnitText( unitSystem );
    }
    else
    {
        switch ( unitSystem )
        {
            case RiaEclipseUnitTools::UnitSystem::UNITS_METRIC:
                switch ( flowPhase )
                {
                    case FLOW_PHASE_GAS:
                        unitText = "[kSm<sup>3</sup>/day]";
                        break;
                    case FLOW_PHASE_WATER: // Intentionally fall through, water and oil have same unit
                    case FLOW_PHASE_OIL:
                        unitText = "[Sm<sup>3</sup>/day]";
                        break;
                    default:
                        unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
                        break;
                }
                break;

            case RiaEclipseUnitTools::UnitSystem::UNITS_FIELD:
                switch ( flowPhase )
                {
                    case FLOW_PHASE_GAS:
                        unitText = "[BOE/day]";
                        break;
                    case FLOW_PHASE_WATER: // Intentionally fall through, water and oil have same unit
                    case FLOW_PHASE_OIL:
                        unitText = "[BBL/day]";
                        break;
                    default:
                        unitText = "[Liquid BBL/day], [Gas BOE/day]";
                        break;
                }
                break;
            case RiaEclipseUnitTools::UnitSystem::UNITS_LAB:
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
std::map<QDateTime, std::set<RifDataSourceForRftPlt>> RimWellPlotTools::calculateRelevantTimeStepsFromCases(
    const QString&                                               wellPathNameOrSimWellName,
    const std::vector<RifDataSourceForRftPlt>&                   selSources,
    const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults )
{
    bool addFirstTimestep = ( interestingRFTResults.count( RifEclipseRftAddress::PRESSURE ) == 1 );

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
            case RifDataSourceForRftPlt::RFT:
                hasRftData = true;
                break;
            case RifDataSourceForRftPlt::GRID:
                hasGridData = true;
                break;
            case RifDataSourceForRftPlt::OBSERVED:
            case RifDataSourceForRftPlt::OBSERVED_FMU_RFT:
                hasObservedData = true;
                break;
            case RifDataSourceForRftPlt::SUMMARY_RFT:
                hasSummaryRftData = true;
                break;
            case RifDataSourceForRftPlt::ENSEMBLE_RFT:
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
            if ( source.sourceType() == RifDataSourceForRftPlt::OBSERVED && source.wellLogFile() )
            {
                observedTimeStepsWithSources[source.wellLogFile()->date()].insert( source );
            }
            else if ( source.sourceType() == RifDataSourceForRftPlt::OBSERVED_FMU_RFT && source.observedFmuRftData() )
            {
                std::set<QDateTime> rftFmuTimes =
                    source.observedFmuRftData()->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );
                for ( const QDateTime& date : rftFmuTimes )
                {
                    observedTimeStepsWithSources[date].insert( source );
                }
            }
        }
    }

    if ( hasRftData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::RFT && source.rftReader() )
            {
                std::set<QDateTime> rftTimes = source.rftReader()->availableTimeSteps( simWellName, interestingRFTResults );
                for ( const QDateTime& date : rftTimes )
                {
                    rftTimeStepsWithSources[date].insert( source );
                }
            }
        }
    }

    if ( hasGridData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::GRID && source.eclCase() )
            {
                std::set<QDateTime> wellTimeSteps =
                    RimWellPlotTools::availableSimWellTimesteps( source.eclCase(), simWellName, addFirstTimestep );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    gridTimestepsWithSources[date].insert( source );
                }
            }
        }
    }

    if ( hasSummaryRftData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::SUMMARY_RFT && source.summaryCase() &&
                 source.summaryCase()->rftReader() )
            {
                std::set<QDateTime> wellTimeSteps =
                    source.summaryCase()->rftReader()->availableTimeSteps( wellPathNameOrSimWellName );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    summaryRftTimeStepsWithSources[date].insert( source );
                }
            }
        }
    }

    if ( hasEnsembleData )
    {
        for ( const auto& source : selSources )
        {
            if ( source.sourceType() == RifDataSourceForRftPlt::ENSEMBLE_RFT && source.ensemble() )
            {
                std::set<QDateTime> wellTimeSteps = source.ensemble()->rftTimeStepsForWell( wellPathNameOrSimWellName );

                for ( const QDateTime& date : wellTimeSteps )
                {
                    ensembleTimeStepsWithSources[date].insert( source );
                }
            }
        }
    }

    // If we have a time baseline add the equal or adjacent grid timesteps

    std::map<QDateTime, std::set<RifDataSourceForRftPlt>>  timestepsToShowWithSources;
    std::map<QDateTime, std::set<RifDataSourceForRftPlt>>* timeBaseline = nullptr;

    if ( hasObservedData )
    {
        timeBaseline = &observedTimeStepsWithSources;
    }
    else if ( hasRftData )
    {
        timeBaseline = &rftTimeStepsWithSources;
    }
    else if ( hasSummaryRftData )
    {
        timeBaseline = &summaryRftTimeStepsWithSources;
    }
    else if ( hasEnsembleData )
    {
        timeBaseline = &ensembleTimeStepsWithSources;
    }

    if ( timeBaseline )
    {
        std::set<QDateTime> baseTimeSteps;
        for ( const auto& dateSourceSetPair : *timeBaseline )
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

        std::set<QDateTime> filteredRftTimeSteps =
            RimWellPlotTools::findMatchingOrAdjacentTimeSteps( baseTimeSteps, rftTimeSteps );
        std::set<QDateTime> filteredGridTimeSteps =
            RimWellPlotTools::findMatchingOrAdjacentTimeSteps( baseTimeSteps, gridTimeSteps );
        std::set<QDateTime> filteredEnsembleRftTimeSteps =
            RimWellPlotTools::findMatchingOrAdjacentTimeSteps( baseTimeSteps, ensembleRftTimeSteps );

        if ( addFirstTimestep && gridTimeSteps.size() )
        {
            filteredGridTimeSteps.insert( *gridTimeSteps.begin() );
        }

        // Fill final map
        timestepsToShowWithSources = observedTimeStepsWithSources;

        std::set<QDateTime>& allFilteredTimesteps = filteredRftTimeSteps;
        allFilteredTimesteps.insert( filteredEnsembleRftTimeSteps.begin(), filteredEnsembleRftTimeSteps.end() );
        allFilteredTimesteps.insert( filteredGridTimeSteps.begin(), filteredGridTimeSteps.end() );

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
void RimWellPlotTools::calculateValueOptionsForTimeSteps(
    const QString&                                               wellPathNameOrSimWellName,
    const std::vector<RifDataSourceForRftPlt>&                   selSources,
    const std::set<RifEclipseRftAddress::RftWellLogChannelType>& interestingRFTResults,
    QList<caf::PdmOptionItemInfo>&                               options )
{
    auto timestepsToShowWithSources =
        calculateRelevantTimeStepsFromCases( wellPathNameOrSimWellName, selSources, interestingRFTResults );

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
                case RifDataSourceForRftPlt::OBSERVED:
                case RifDataSourceForRftPlt::OBSERVED_FMU_RFT:
                    hasObs = true;
                    break;
                case RifDataSourceForRftPlt::RFT:
                    hasRft = true;
                    break;
                case RifDataSourceForRftPlt::GRID:
                    hasGrid = true;
                    break;
                case RifDataSourceForRftPlt::ENSEMBLE_RFT:
                    hasEnsemble = true;
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
