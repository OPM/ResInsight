/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"
#include "RifProjectSummaryDataWriter.h"
#include "RifSummaryReaderInterface.h"

#include "opm/io/eclipse/ExtESmry.hpp"

#include <QDir>
#include <QTemporaryDir>

#include <chrono>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Reader providing only the TIME vector, given as days or hours since simulation start
//--------------------------------------------------------------------------------------------------
class StubTimeVectorReader : public RifSummaryReaderInterface
{
public:
    StubTimeVectorReader( const std::vector<time_t>& timeSteps, const std::vector<double>& timeSinceStart, const std::string& timeUnit )
        : m_timeSteps( timeSteps )
        , m_timeSinceStart( timeSinceStart )
        , m_timeUnit( timeUnit )
    {
        m_allResultAddresses.insert( RifEclipseSummaryAddress::miscAddress( "TIME" ) );
    }

    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override { return m_timeSteps; }
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override
    {
        return { true, m_timeSinceStart };
    }
    std::string                   unitName( const RifEclipseSummaryAddress& resultAddress ) const override { return m_timeUnit; }
    RiaDefines::EclipseUnitSystem unitSystem() const override { return RiaDefines::EclipseUnitSystem::UNITS_METRIC; }
    size_t                        keywordCount() const override { return m_allResultAddresses.size(); }

private:
    std::vector<time_t> m_timeSteps;
    std::vector<double> m_timeSinceStart;
    std::string         m_timeUnit;
};

//--------------------------------------------------------------------------------------------------
/// Time step computed from start date and TIME as done by the summary readers
//--------------------------------------------------------------------------------------------------
time_t timeStepFromDays( time_t startOfSimulation, float days )
{
    const double secondsInOneDay = 24 * 3600;
    return static_cast<time_t>( startOfSimulation + days * secondsInOneDay );
}

//--------------------------------------------------------------------------------------------------
/// Write an additional summary file based on the source reader, and read it back using opm-common
//--------------------------------------------------------------------------------------------------
void writeAndVerify( const std::vector<float>& timeSinceStart, const std::string& timeUnit = "DAYS" )
{
    const double secondsPerTimeUnit = timeUnit == "HOURS" ? 3600.0 : 24.0 * 3600.0;

    // 2020-01-01 00:00:00 UTC
    const time_t startOfSimulation = 1577836800;

    std::vector<time_t> timeSteps;
    std::vector<double> timeValues;
    for ( auto t : timeSinceStart )
    {
        timeSteps.push_back( static_cast<time_t>( startOfSimulation + t * secondsPerTimeUnit ) );
        timeValues.push_back( t );
    }

    StubTimeVectorReader reader( timeSteps, timeValues, timeUnit );

    RifProjectSummaryDataWriter writer;
    writer.importFromSourceSummaryReader( &reader );
    writer.setData( { "FOPT_1" }, { "SM3" }, { std::vector<float>( timeSinceStart.size(), 1.0f ) } );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    const std::string fileName = QDir( tempDir.path() ).absoluteFilePath( "additional.ESMRY" ).toStdString();

    writer.writeDataToFile( fileName );
    ASSERT_TRUE( writer.errorMessages().empty() );

    Opm::EclIO::ExtESmry esmry( fileName );

    const time_t startDate = std::chrono::system_clock::to_time_t( esmry.startdate() );
    EXPECT_EQ( startOfSimulation, startDate );

    // TIME is always written in days
    EXPECT_EQ( "DAYS", esmry.get_unit( "TIME" ) );

    const auto& writtenTime = esmry.get( "TIME" );
    ASSERT_EQ( timeSteps.size(), writtenTime.size() );
    for ( size_t i = 0; i < writtenTime.size(); i++ )
    {
        EXPECT_EQ( timeSteps[i], timeStepFromDays( startDate, writtenTime[i] ) );
    }

    const auto dates = esmry.dates();
    ASSERT_EQ( timeSteps.size(), dates.size() );
    for ( size_t i = 0; i < dates.size(); i++ )
    {
        EXPECT_EQ( timeSteps[i], std::chrono::system_clock::to_time_t( dates[i] ) );
    }
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// The first report step coincides with the start of the simulation
//--------------------------------------------------------------------------------------------------
TEST( RifProjectSummaryDataWriter, StartDateWhenTimeStartsAtZero )
{
    writeAndVerify( { 0.0f, 1.0f, 2.0f } );
}

//--------------------------------------------------------------------------------------------------
/// Restart case imported without history: the first report step is after the start of the simulation,
/// see https://github.com/OPM/ResInsight/issues/11139
//--------------------------------------------------------------------------------------------------
TEST( RifProjectSummaryDataWriter, StartDateWhenTimeStartsAfterZero )
{
    writeAndVerify( { 100.0f, 101.0f, 102.0f } );
}

//--------------------------------------------------------------------------------------------------
/// First report step is not a whole number of seconds after the start of the simulation
//--------------------------------------------------------------------------------------------------
TEST( RifProjectSummaryDataWriter, StartDateWhenTimeIsFractional )
{
    writeAndVerify( { 100.3f, 101.3f, 250.7f } );
}

//--------------------------------------------------------------------------------------------------
/// TIME in hours, starting at zero
//--------------------------------------------------------------------------------------------------
TEST( RifProjectSummaryDataWriter, StartDateWhenTimeInHoursStartsAtZero )
{
    writeAndVerify( { 0.0f, 12.0f, 36.0f }, "HOURS" );
}

//--------------------------------------------------------------------------------------------------
/// TIME in hours, first report step one day after the start of the simulation
//--------------------------------------------------------------------------------------------------
TEST( RifProjectSummaryDataWriter, StartDateWhenTimeInHoursStartsAfterZero )
{
    writeAndVerify( { 24.0f, 36.0f, 72.0f }, "HOURS" );
}
