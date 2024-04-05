/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigWellLogCsvFile.h"

#include "RiaInterpolationTools.h"
#include "RifCsvUserDataParser.h"
#include "RigWellLogCurveData.h"
#include "RigWellPathGeometryTools.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

#include "RiaLogging.h"
#include "RiaStringEncodingTools.h"

#include "RimWellLogCurve.h"
#include "RimWellPath.h"

#include <QFileInfo>
#include <QString>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogCsvFile::RigWellLogCsvFile()
    : RigWellLogFile()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogCsvFile::~RigWellLogCsvFile()
{
    close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellLogCsvFile::open( const QString& fileName, RigWellPath* wellPath, QString* errorMessage )
{
    m_wellLogChannelNames.clear();
    double                 samplingInterval  = 1.0;
    cvf::cref<RigWellPath> resampledWellPath = resampleWellPath( *wellPath, samplingInterval );

    RifCsvUserDataFileParser parser( fileName, errorMessage );

    AsciiDataParseOptions parseOptions;
    parseOptions.useCustomDateTimeFormat = true;
    parseOptions.dateTimeFormat          = "dd.MM.yyyy hh:mm:ss";
    parseOptions.fallbackDateTimeFormat  = "dd.MM.yyyy";
    parseOptions.cellSeparator           = ";";
    parseOptions.decimalSeparator        = ",";

    if ( !parser.parse( parseOptions ) )
    {
        return false;
    }

    std::map<QString, std::vector<double>> readValues;

    for ( auto s : parser.tableData().columnInfos() )
    {
        if ( s.dataType != Column::NUMERIC ) continue;
        QString columnName = QString::fromStdString( s.columnName() );
        m_wellLogChannelNames.append( columnName );
        readValues[columnName] = s.values;
        m_units[columnName]    = QString::fromStdString( s.unitName );

        if ( columnName.toUpper() == "TVDMSL" || columnName.toUpper().contains( "TVD" ) )
        {
            m_tvdMslLogName = columnName;
        }
    }

    if ( m_tvdMslLogName.isEmpty() )
    {
        QString message = "CSV file does not have TVD values.";
        RiaLogging::error( message );
        if ( errorMessage ) *errorMessage = message;
        return false;
    }

    std::vector<double> readTvds = readValues[m_tvdMslLogName];

    for ( auto [channelName, readValues] : readValues )
    {
        if ( channelName == m_tvdMslLogName )
        {
            // Use TVD from well path.
            m_values[m_tvdMslLogName] = resampledWellPath->trueVerticalDepths();
        }
        else
        {
            CAF_ASSERT( readValues.size() == readTvds.size() );

            auto wellPathMds  = resampledWellPath->measuredDepths();
            auto wellPathTvds = resampledWellPath->trueVerticalDepths();

            // Interpolate values for the well path depths (from TVD).
            // Assumes that the well channel values is dependent on TVD only (MD is not considered).
            std::vector<double> values;
            for ( auto tvd : wellPathTvds )
            {
                double value = RiaInterpolationTools::linear( readTvds, readValues, tvd, RiaInterpolationTools::ExtrapolationMode::TREND );
                values.push_back( value );
            }

            m_values[channelName] = values;
        }
    }

    // Use MD from well path.
    m_depthLogName           = "DEPTH";
    m_values[m_depthLogName] = resampledWellPath->measuredDepths();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCsvFile::close()
{
    m_wellLogChannelNames.clear();
    m_depthLogName.clear();
    m_values.clear();
    m_units.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigWellLogCsvFile::wellLogChannelNames() const
{
    return m_wellLogChannelNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCsvFile::depthValues() const
{
    return values( m_depthLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCsvFile::tvdMslValues() const
{
    return values( m_tvdMslLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCsvFile::tvdRkbValues() const
{
    // Not supported.
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCsvFile::values( const QString& name ) const
{
    if ( auto it = m_values.find( name ); it != m_values.end() ) return it->second;
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCsvFile::depthUnitString() const
{
    return wellLogChannelUnitString( m_tvdMslLogName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCsvFile::wellLogChannelUnitString( const QString& wellLogChannelName ) const
{
    auto unit = m_units.find( wellLogChannelName );
    if ( unit != m_units.end() )
        return unit->second;
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellLogCsvFile::hasTvdMslChannel() const
{
    return !m_tvdMslLogName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellLogCsvFile::hasTvdRkbChannel() const
{
    return !m_tvdRkbLogName.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigWellLogCsvFile::getMissingValue() const
{
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPath* RigWellLogCsvFile::resampleWellPath( const RigWellPath& wellPath, double samplingInterval )
{
    std::vector<double> measuredDepths = resampleMeasuredDepths( wellPath.measuredDepths(), samplingInterval );

    std::vector<cvf::Vec3d> wellPathPoints;
    for ( double md : measuredDepths )
    {
        wellPathPoints.push_back( wellPath.interpolatedPointAlongWellPath( md ) );
    }

    return new RigWellPath( wellPathPoints, measuredDepths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCsvFile::resampleMeasuredDepths( const std::vector<double>& measuredDepths, double samplingInterval )
{
    double firstMd = measuredDepths.front();
    double lastMd  = measuredDepths.back();

    std::vector<double> resampledMds;
    for ( double md = firstMd; md < lastMd; md += samplingInterval )
    {
        resampledMds.push_back( md );
    }
    resampledMds.push_back( lastMd );

    return resampledMds;
}
