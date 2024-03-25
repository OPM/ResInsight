/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RifHdf5SummaryReader.h"

#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"
#include "RifHdf5Reader.h"

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif
#include "H5Cpp.h"

#include <QDateTime>
#include <QString>

#include <chrono>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifHdf5SummaryReader::RifHdf5SummaryReader( const QString& fileName )
{
    m_fileName = fileName.toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifHdf5SummaryReader::~RifHdf5SummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifHdf5SummaryReader::vectorNames()
{
    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        auto h5File = H5::H5File( m_fileName.c_str(), H5F_ACC_RDONLY );

        std::vector<std::string> names;

        auto groupNames = RifHdf5ReaderTools::getSubGroupNames( &h5File, "summary_vectors" );
        names.reserve( groupNames.size() );
        for ( const auto& name : groupNames )
        {
            names.push_back( name );
        }

        return names;
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifHdf5SummaryReader::timeSteps() const
{
    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        auto h5File = H5::H5File( m_fileName.c_str(), H5F_ACC_RDONLY );

        std::vector<time_t> times;

        // TODO: This function uses code taken from ESmry::dates(). There is conversion from time_t to
        // chrono::system_clock::time_points, and then back to time_t again. Consider using one representation

        double time_unit = 24 * 3600;

        using namespace std::chrono;
        using DoubSec = duration<double, seconds::period>;
        using TP      = time_point<system_clock, DoubSec>;

        auto timeDeltasInDays = values( "TIME" );

        // Add custom method to convert from time_point to time_t. The usual implementation of
        // chrono::system_clock::to_time_t() uses nanoseconds which will overflow on data with
        // long time spans.
        auto convertTimePointToTimeT = []( const TP& value )
        { return std::chrono::duration_cast<std::chrono::seconds>( value.time_since_epoch() ).count(); };

        auto startDat = std::chrono::system_clock::from_time_t( startDate() );

        for ( const auto& t : timeDeltasInDays )
        {
            TP          timePoint   = startDat + duration_cast<TP::duration>( DoubSec( t * time_unit ) );
            std::time_t timeAsTimeT = convertTimePointToTimeT( timePoint );
            times.push_back( timeAsTimeT );
        }

        return times;
    }

    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifHdf5SummaryReader::values( const std::string& vectorName, int smspecKeywordIndex ) const
{
    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        auto h5File = H5::H5File( m_fileName.c_str(), H5F_ACC_RDONLY );

        std::string idText    = std::to_string( smspecKeywordIndex );
        std::string groupPath = "summary_vectors/" + vectorName + "/" + idText;

        {
            H5::Group   generalGroup = h5File.openGroup( groupPath.c_str() );
            H5::DataSet dataset      = H5::DataSet( generalGroup.openDataSet( "values" ) );

            hsize_t       dims[2];
            H5::DataSpace dataspace = dataset.getSpace();
            dataspace.getSimpleExtentDims( dims, nullptr );

            std::vector<double> values;
            values.resize( dims[0] );
            dataset.read( values.data(), H5::PredType::NATIVE_DOUBLE );

            return values;
        }
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifHdf5SummaryReader::values( const std::string& vectorName ) const
{
    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        auto        h5File    = H5::H5File( m_fileName.c_str(), H5F_ACC_RDONLY );
        std::string groupPath = "summary_vectors/" + vectorName;

        auto groupNames = RifHdf5ReaderTools::getSubGroupNames( &h5File, groupPath );
        if ( !groupNames.empty() )
        {
            groupPath = groupPath + "/" + groupNames[0];

            H5::Group   generalGroup = h5File.openGroup( groupPath.c_str() );
            H5::DataSet dataset      = H5::DataSet( generalGroup.openDataSet( "values" ) );

            hsize_t       dims[2];
            H5::DataSpace dataspace = dataset.getSpace();
            dataspace.getSimpleExtentDims( dims, nullptr );

            std::vector<double> values;
            values.resize( dims[0] );
            dataset.read( values.data(), H5::PredType::NATIVE_DOUBLE );

            return values;
        }
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RifHdf5SummaryReader::startDate() const
{
    try
    {
        H5::Exception::dontPrint(); // Turn off auto-printing of failures to handle the errors appropriately

        auto h5File = H5::H5File( m_fileName.c_str(), H5F_ACC_RDONLY );

        QString groupPath = QString( "general" );

        H5::Group   GridFunction_00002 = h5File.openGroup( groupPath.toStdString().c_str() );
        H5::DataSet dataset            = H5::DataSet( GridFunction_00002.openDataSet( "start_date" ) );

        hsize_t       dims[2];
        H5::DataSpace dataspace = dataset.getSpace();
        dataspace.getSimpleExtentDims( dims, nullptr );

        std::vector<int> values;
        values.resize( dims[0] );
        dataset.read( values.data(), H5::PredType::NATIVE_INT );

        int day    = values[0];
        int month  = values[1];
        int year   = values[2];
        int hour   = values[3];
        int minute = values[4];
        int second = values[5];

        QDate date( year, month, day );
        QTime time( hour, minute, second );

        QDateTime dt( date, time );

        QDateTime reportDateTime = RiaQDateTimeTools::createUtcDateTime( dt );

        time_t myTime = RiaTimeTTools::fromQDateTime( reportDateTime );
        return myTime;
    }
    catch ( ... )
    {
    }
    return {};
}
