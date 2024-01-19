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

#include "RifCsvUserDataParser.h"
#include "RigWellLogCurveData.h"
#include "RigWellPathGeometryTools.h"

#include "SummaryPlotCommands/RicPasteAsciiDataToSummaryPlotFeatureUi.h"

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
    m_values.clear();
    m_wellLogChannelNames.clear();

    RifCsvUserDataFileParser parser( fileName );

    AsciiDataParseOptions parseOptions;
    parseOptions.useCustomDateTimeFormat = true;
    parseOptions.dateTimeFormat          = "dd.MM.yyyy hh:mm:ss";
    parseOptions.fallbackDateTimeFormat  = "dd.MM.yyyy";
    parseOptions.cellSeparator           = ";";
    parseOptions.decimalSeparator        = ",";

    if ( parser.parse( parseOptions ) )
    {
        for ( auto s : parser.tableData().columnInfos() )
        {
            if ( s.dataType != Column::NUMERIC ) continue;
            QString columnName = QString::fromStdString( s.columnName() );
            printf( "Column name: [%s] %s %zu\n", columnName.toStdString().c_str(), s.unitName.c_str(), s.values.size() );
            m_wellLogChannelNames.append( columnName );
            m_values[columnName] = s.values;
            m_units[columnName]  = QString::fromStdString( s.unitName );

            if ( columnName.toUpper() == "DEPT" || columnName.toUpper() == "DEPTH" || columnName.toUpper() == "MD" )
            {
                m_depthLogName = columnName;
            }
            else if ( columnName.toUpper() == "TVDMSL" || columnName.toUpper().contains( "TVD" ) )
            {
                m_tvdMslLogName = columnName;
            }
            else if ( columnName.toUpper() == "TVDRKB" )
            {
                m_tvdRkbLogName = columnName;
            }
        }

        if ( m_depthLogName.isEmpty() )
        {
            auto wellPathMd  = wellPath->measuredDepths();
            auto wellPathTvd = wellPath->trueVerticalDepths();

            // Estimate measured depth for cells that do not have measured depth
            std::vector<double> tvdValuesToEstimate = tvdMslValues();
            auto estimatedMeasuredDepth = RigWellPathGeometryTools::interpolateMdFromTvd( wellPathMd, wellPathTvd, tvdValuesToEstimate );
            m_depthLogName              = "DEPTH";
            m_wellLogChannelNames.append( m_depthLogName );
            m_values[m_depthLogName] = estimatedMeasuredDepth;

            printf( "depth: %zu %zu\n", tvdValuesToEstimate.size(), estimatedMeasuredDepth.size() );
        }

        return true;
    }

    return false;
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
QString RigWellLogCsvFile::wellName() const
{
    // CVF_ASSERT( m_wellLogFile );
    // return RiaStringEncodingTools::fromNativeEncoded( m_wellLogFile->GetWellName().data() );
    return "TODO";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCsvFile::date() const
{
    // CVF_ASSERT( m_wellLogFile );
    // return RiaStringEncodingTools::fromNativeEncoded( m_wellLogFile->GetWellName().data() );
    return "TODO";
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
    return values( m_tvdRkbLogName );
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
    QString unit;

    // NRLib::CsvWell* lasWell = dynamic_cast<NRLib::CsvWell*>( m_wellLogFile );
    // if ( lasWell )
    // {
    //     unit = QString::fromStdString( lasWell->depthUnit() );
    // }

    return unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCsvFile::wellLogChannelUnitString( const QString& wellLogChannelName, RiaDefines::DepthUnitType displayDepthUnit ) const
{
    QString unit;

    // NRLib::CsvWell* lasWell = dynamic_cast<NRLib::CsvWell*>( m_wellLogFile );
    // if ( lasWell )
    // {
    //     unit = QString::fromStdString( lasWell->unitName( wellLogChannelName.toStdString() ) );
    // }

    // if ( unit == depthUnitString() )
    // {
    //     if ( displayDepthUnit != depthUnit() )
    //     {
    //         if ( displayDepthUnit == RiaDefines::DepthUnitType::UNIT_METER )
    //         {
    //             return "M";
    //         }
    //         else if ( displayDepthUnit == RiaDefines::DepthUnitType::UNIT_FEET )
    //         {
    //             return "FT";
    //         }
    //         else if ( displayDepthUnit == RiaDefines::DepthUnitType::UNIT_NONE )
    //         {
    //             CVF_ASSERT( false );
    //             return "";
    //         }
    //     }
    // }

    return unit;
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
RiaDefines::DepthUnitType RigWellLogCsvFile::depthUnit() const
{
    RiaDefines::DepthUnitType unitType = RiaDefines::DepthUnitType::UNIT_METER;

    if ( depthUnitString().toUpper() == "F" || depthUnitString().toUpper() == "FT" )
    {
        unitType = RiaDefines::DepthUnitType::UNIT_FEET;
    }

    return unitType;
}
