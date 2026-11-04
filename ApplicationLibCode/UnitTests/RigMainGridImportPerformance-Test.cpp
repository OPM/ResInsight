/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RifEclipseInputFileTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RimEclipseResultCase.h"

#include <QString>
#include <QTemporaryDir>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
// Must be included after windows.h
#include <psapi.h>
#pragma comment( lib, "psapi.lib" )
#endif

namespace
{
//--------------------------------------------------------------------------------------------------
/// Current resident memory (working set) of this process in bytes. Returns 0 if unavailable.
//--------------------------------------------------------------------------------------------------
size_t currentResidentBytes()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) ) ) return pmc.WorkingSetSize;
    return 0;
#else
    std::ifstream statm( "/proc/self/statm" );
    size_t        totalPages    = 0;
    size_t        residentPages = 0;
    if ( statm >> totalPages >> residentPages ) return residentPages * 4096;
    return 0;
#endif
}

//--------------------------------------------------------------------------------------------------
/// Polls resident memory on a background thread and records the peak value
//--------------------------------------------------------------------------------------------------
class PeakMemorySampler
{
public:
    PeakMemorySampler()
        : m_peak( currentResidentBytes() )
        , m_stop( false )
        , m_thread(
              [this]()
              {
                  while ( !m_stop )
                  {
                      update();
                      std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
                  }
              } )
    {
    }

    ~PeakMemorySampler() { stop(); }

    size_t stop()
    {
        if ( !m_stop.exchange( true ) ) m_thread.join();
        update();
        return m_peak;
    }

private:
    void update()
    {
        size_t current = currentResidentBytes();
        size_t peak    = m_peak;
        while ( current > peak && !m_peak.compare_exchange_weak( peak, current ) )
        {
        }
    }

    std::atomic<size_t> m_peak;
    std::atomic<bool>   m_stop;
    std::thread         m_thread;
};

size_t envSize( const char* name, size_t defaultValue )
{
    const char* value = std::getenv( name );
    if ( !value ) return defaultValue;
    long long parsed = std::atoll( value );
    return parsed > 0 ? static_cast<size_t>( parsed ) : defaultValue;
}

double toMiB( size_t bytes )
{
    return static_cast<double>( bytes ) / ( 1024.0 * 1024.0 );
}

//--------------------------------------------------------------------------------------------------
/// Writes a regular box grid as a GRDECL file (SPECGRID, COORD, ZCORN, ACTNUM)
//--------------------------------------------------------------------------------------------------
void writeSyntheticGrdecl( const std::string& fileName, size_t ni, size_t nj, size_t nk )
{
    std::ofstream out( fileName, std::ios::out | std::ios::trunc );
    out << "SPECGRID\n" << ni << " " << nj << " " << nk << " 1 F /\n";

    const int dxy = 10;
    const int dz  = 1;

    out << "COORD\n";
    for ( size_t j = 0; j <= nj; j++ )
    {
        for ( size_t i = 0; i <= ni; i++ )
        {
            const size_t x = i * dxy;
            const size_t y = j * dxy;
            out << x << " " << y << " 0 " << x << " " << y << " " << nk * dz << "\n";
        }
    }
    out << "/\n";

    // Corner point ordering: for each layer top and bottom, for each j two rows (south/north edge),
    // for each i two values (west/east edge)
    out << "ZCORN\n";
    for ( size_t k = 0; k < nk; k++ )
    {
        for ( size_t layerEdge = 0; layerEdge < 2; layerEdge++ )
        {
            const size_t z = ( k + layerEdge ) * dz;
            for ( size_t j = 0; j < nj; j++ )
            {
                for ( size_t row = 0; row < 2; row++ )
                {
                    for ( size_t i = 0; i < ni; i++ )
                    {
                        out << z << " " << z << " ";
                    }
                    out << "\n";
                }
            }
        }
    }
    out << "/\n";

    out << "ACTNUM\n";
    const size_t cellCount = ni * nj * nk;
    for ( size_t n = 0; n < cellCount; n++ )
    {
        out << "1 ";
        if ( n % 100 == 99 ) out << "\n";
    }
    out << "/\n";
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Times import of a large synthetic grid into a RigMainGrid and reports memory use.
///
/// Only uses APIs available on both dev and feature branches, so the same test file can be dropped
/// into either to compare results. Grid size is controlled by environment variables:
///   RESINSIGHT_PERF_GRID_NI, RESINSIGHT_PERF_GRID_NJ, RESINSIGHT_PERF_GRID_NK (default 350 x 350 x 80)
///
/// Disabled by default. Run with --gtest_also_run_disabled_tests --gtest_filter=RigMainGridImportPerformance.*
//--------------------------------------------------------------------------------------------------
TEST( RigMainGridImportPerformance, DISABLED_SyntheticGridImportTimeAndMemory )
{
    const size_t ni = envSize( "RESINSIGHT_PERF_GRID_NI", 350 );
    const size_t nj = envSize( "RESINSIGHT_PERF_GRID_NJ", 350 );
    const size_t nk = envSize( "RESINSIGHT_PERF_GRID_NK", 80 );

    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );
    const QString filePath = tempDir.filePath( "synthetic_grid.grdecl" );

    using Clock  = std::chrono::steady_clock;
    auto seconds = []( Clock::time_point a, Clock::time_point b ) { return std::chrono::duration<double>( b - a ).count(); };

    auto tGenStart = Clock::now();
    writeSyntheticGrdecl( filePath.toStdString(), ni, nj, nk );
    auto tGenEnd = Clock::now();

    std::unique_ptr<RimEclipseResultCase> resultCase( new RimEclipseResultCase );
    cvf::ref<RigEclipseCaseData>          caseData = new RigEclipseCaseData( resultCase.get() );

    const size_t memBefore = currentResidentBytes();

    double importSeconds  = 0.0;
    double cacheSeconds   = 0.0;
    size_t peakImport     = 0;
    size_t memAfterImport = 0;
    size_t peakCache      = 0;
    size_t memAfterCache  = 0;

    {
        PeakMemorySampler sampler;
        auto              t0 = Clock::now();

        QString errorMessages;
        bool    ok = RifEclipseInputFileTools::openGridFile( filePath, caseData.p(), false, &errorMessages );

        auto t1        = Clock::now();
        importSeconds  = seconds( t0, t1 );
        peakImport     = sampler.stop();
        memAfterImport = currentResidentBytes();
        ASSERT_TRUE( ok ) << errorMessages.toStdString();
    }

    RigMainGrid* mainGrid = caseData->mainGrid();
    ASSERT_NE( mainGrid, nullptr );

    {
        PeakMemorySampler sampler;
        auto              t0 = Clock::now();
        mainGrid->computeCachedData();
        auto t1       = Clock::now();
        cacheSeconds  = seconds( t0, t1 );
        peakCache     = sampler.stop();
        memAfterCache = currentResidentBytes();
    }

    EXPECT_EQ( ni, mainGrid->cellCountI() );
    EXPECT_EQ( nj, mainGrid->cellCountJ() );
    EXPECT_EQ( nk, mainGrid->cellCountK() );

    std::cout << "[PERF] Grid size            : " << ni << " x " << nj << " x " << nk << " = " << ni * nj * nk << " cells\n"
              << "[PERF] File generation      : " << seconds( tGenStart, tGenEnd ) << " s (not part of measurement)\n"
              << "[PERF] openGridFile         : " << importSeconds << " s\n"
              << "[PERF] computeCachedData    : " << cacheSeconds << " s\n"
              << "[PERF] Memory before        : " << toMiB( memBefore ) << " MiB\n"
              << "[PERF] Peak during import   : " << toMiB( peakImport ) << " MiB (+"
              << toMiB( peakImport - std::min( peakImport, memBefore ) ) << " MiB)\n"
              << "[PERF] After import         : " << toMiB( memAfterImport ) << " MiB\n"
              << "[PERF] Peak during cache    : " << toMiB( peakCache ) << " MiB\n"
              << "[PERF] After cache          : " << toMiB( memAfterCache ) << " MiB (+"
              << toMiB( memAfterCache - std::min( memAfterCache, memBefore ) ) << " MiB retained)\n"
              << std::flush;
}
