/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleWellLogStatistics.h"

#include "RiaCurveMerger.h"
#include "RiaDefines.h"
#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"
#include "RiaWellLogUnitTools.h"

#include "RigStatisticsMath.h"
#include "RigWellLogFile.h"

#include "RimWellLogFile.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RimEnsembleWellLogStatistics::StatisticsType>::setUp()
{
    addItem( RimEnsembleWellLogStatistics::StatisticsType::P10, "P10", "P10" );
    addItem( RimEnsembleWellLogStatistics::StatisticsType::P50, "P50", "P50" );
    addItem( RimEnsembleWellLogStatistics::StatisticsType::P90, "P90", "P90" );
    addItem( RimEnsembleWellLogStatistics::StatisticsType::MEAN, "MEAN", "Mean" );

    setDefault( RimEnsembleWellLogStatistics::StatisticsType::MEAN );
}
}; // namespace caf

RimEnsembleWellLogStatistics::RimEnsembleWellLogStatistics()
{
    m_depthUnit            = RiaDefines::DepthUnitType::UNIT_NONE;
    m_logChannelUnitString = RiaWellLogUnitTools<double>::noUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatistics::calculate( const std::vector<RimWellLogFile*>& wellLogFiles,
                                              const QString&                      wellLogChannelName )
{
    RiaCurveMerger<double> curveMerger;

    RiaWeightedMeanCalculator<size_t> dataSetSizeCalc;

    for ( RimWellLogFile* wellLogFile : wellLogFiles )
    {
        QString errorMessage;
        if ( wellLogFile->readFile( &errorMessage ) )
        {
            RigWellLogFile*           fileData        = wellLogFile->wellLogFileData();
            RiaDefines::DepthUnitType depthUnitInFile = fileData->depthUnit();
            if ( m_depthUnit != RiaDefines::DepthUnitType::UNIT_NONE && m_depthUnit != depthUnitInFile )
            {
                RiaLogging::error( QString( "Unexpected depth unit in file %1." ).arg( wellLogFile->fileName() ) );
            }
            m_depthUnit = depthUnitInFile;

            QString logChannelUnitString = fileData->wellLogChannelUnitString( wellLogChannelName );
            if ( m_logChannelUnitString != RiaWellLogUnitTools<double>::noUnitString() &&
                 m_logChannelUnitString != logChannelUnitString )
            {
                RiaLogging::error( QString( "Unexpected unit in file %1." ).arg( wellLogFile->fileName() ) );
            }
            m_logChannelUnitString = logChannelUnitString;

            std::vector<double> depths = fileData->depthValues();
            std::vector<double> values = fileData->values( wellLogChannelName );
            if ( !depths.empty() && !values.empty() )
            {
                dataSetSizeCalc.addValueAndWeight( depths.size(), 1.0 );
                curveMerger.addCurveData( depths, values );
            }
        }
        else
        {
            RiaLogging::error( errorMessage );
        }
    }
    curveMerger.computeInterpolatedValues( true );

    clearData();

    const std::vector<double>& allDepths = curveMerger.allXValues();
    if ( !allDepths.empty() )
    {
        // Make sure we end up with approximately the same amount of points as originally
        // Since allDepths contain *valid* values, it can potentially be smaller than the mean.
        // Thus we need to ensure sizeMultiplier is at least 1.
        size_t sizeMultiplier = std::max( (size_t)1, allDepths.size() / dataSetSizeCalc.weightedMean() );
        for ( size_t depthIdx = 0; depthIdx < allDepths.size(); depthIdx += sizeMultiplier )
        {
            std::vector<double> valuesAtDepth;
            valuesAtDepth.reserve( curveMerger.curveCount() );
            for ( size_t curveIdx = 0; curveIdx < curveMerger.curveCount(); ++curveIdx )
            {
                const std::vector<double>& curveValues = curveMerger.interpolatedYValuesForAllXValues( curveIdx );
                valuesAtDepth.push_back( curveValues[depthIdx] );
            }
            double p10, p50, p90, mean;
            RigStatisticsMath::calculateStatisticsCurves( valuesAtDepth, &p10, &p50, &p90, &mean );

            m_measuredDepths.push_back( allDepths[depthIdx] );
            m_p10Data.push_back( p10 );
            m_p50Data.push_back( p50 );
            m_p90Data.push_back( p90 );
            m_meanData.push_back( mean );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::measuredDepths() const
{
    return m_measuredDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::tvDepths() const
{
    return m_tvDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::p10() const
{
    return m_p10Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::p50() const
{
    return m_p50Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::p90() const
{
    return m_p90Data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimEnsembleWellLogStatistics::mean() const
{
    return m_meanData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogStatistics::hasP10Data() const
{
    return !m_p10Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogStatistics::hasP50Data() const
{
    return !m_p50Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogStatistics::hasP90Data() const
{
    return !m_p90Data.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleWellLogStatistics::hasMeanData() const
{
    return !m_meanData.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogStatistics::clearData()
{
    m_measuredDepths.clear();
    m_tvDepths.clear();
    m_p10Data.clear();
    m_p50Data.clear();
    m_p90Data.clear();
    m_meanData.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RimEnsembleWellLogStatistics::depthUnitType() const
{
    return m_depthUnit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleWellLogStatistics::logChannelUnitString() const
{
    return m_logChannelUnitString;
}
