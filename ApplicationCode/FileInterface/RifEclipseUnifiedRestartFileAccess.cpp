/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RifEclipseUnifiedRestartFileAccess.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RiaStringEncodingTools.h"
#include "RifEclipseOutputFileTools.h"
#include "RifReaderSettings.h"

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_nnc_data.h"
#include "ert/ecl/ecl_nnc_geometry.h"

#include "cafUtils.h"

#include <QFileInfo>
// #include "cvfTrace.h"

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::RifEclipseUnifiedRestartFileAccess()
    : RifEclipseRestartDataAccess()
{
    m_ecl_file = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseUnifiedRestartFileAccess::~RifEclipseUnifiedRestartFileAccess()
{
    if ( m_ecl_file )
    {
        ecl_file_close( m_ecl_file );
    }
}

//--------------------------------------------------------------------------------------------------
/// Open file
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::open()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::openFile()
{
    if ( !m_ecl_file )
    {
        QString indexFileName = RifEclipseOutputFileTools::createIndexFileName( m_filename );

        if ( useResultIndexFile() )
        {
            if ( caf::Utils::fileExists( indexFileName ) )
            {
                QFileInfo indexFileInfo( indexFileName );
                QFileInfo resultFileInfo( m_filename );

                if ( resultFileInfo.lastModified() < indexFileInfo.lastModified() )
                {
                    m_ecl_file = ecl_file_fast_open( RiaStringEncodingTools::toNativeEncoded( m_filename ).data(),
                                                     RiaStringEncodingTools::toNativeEncoded( indexFileName ).data(),
                                                     ECL_FILE_CLOSE_STREAM );
                    if ( !m_ecl_file )
                    {
                        RiaLogging::error( QString( "Failed to open file %1 using index file." ).arg( m_filename ) );
                        RiaLogging::info( QString( "Will try to open file without index file." ) );
                    }
                    else
                    {
                        RiaLogging::info( QString( "Imported file %1 using index file." ).arg( m_filename ) );
                    }
                }
            }
        }

        if ( !m_ecl_file )
        {
            m_ecl_file =
                ecl_file_open( RiaStringEncodingTools::toNativeEncoded( m_filename ).data(), ECL_FILE_CLOSE_STREAM );
            if ( !m_ecl_file )
            {
                RiaLogging::error( QString( "Failed to open file %1" ).arg( m_filename ) );
            }
            else
            {
                if ( useResultIndexFile() )
                {
                    QFileInfo fi( indexFileName );
                    QString   resultPath = fi.absolutePath();
                    if ( caf::Utils::isFolderWritable( resultPath ) )
                    {
                        bool success =
                            ecl_file_write_index( m_ecl_file,
                                                  RiaStringEncodingTools::toNativeEncoded( indexFileName ).data() );

                        if ( success )
                        {
                            RiaLogging::info( QString( "Exported index file to %1 " ).arg( indexFileName ) );
                        }
                        else
                        {
                            RiaLogging::info( QString( "Failed to exported index file to %1 " ).arg( indexFileName ) );
                        }
                    }
                }
            }
        }
    }

    if ( !m_ecl_file ) return false;

    m_availablePhases = RifEclipseOutputFileTools::findAvailablePhases( m_ecl_file );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::useResultIndexFile() const
{
    RiaPreferences*          prefs          = RiaApplication::instance()->preferences();
    const RifReaderSettings* readerSettings = prefs->readerSettings();

    return readerSettings->useResultIndexFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::extractTimestepsFromEclipse()
{
    m_timeSteps.clear();
    m_daysSinceSimulationStart.clear();
    m_reportNr.clear();

    if ( openFile() )
    {
        RifEclipseOutputFileTools::timeSteps( m_ecl_file, &m_timeSteps, &m_daysSinceSimulationStart );

        // Taken from well_info_add_UNRST_wells

        int num_blocks = ecl_file_get_num_named_kw( m_ecl_file, SEQNUM_KW );
        int block_nr;
        for ( block_nr = 0; block_nr < num_blocks; block_nr++ )
        {
            ecl_file_push_block( m_ecl_file ); // <-------------------------------------------------------
            { //
                ecl_file_subselect_block( m_ecl_file, SEQNUM_KW, block_nr ); //  Ensure that the status
                { //  is not changed as a side
                    const ecl_kw_type* seqnum_kw = ecl_file_iget_named_kw( m_ecl_file, SEQNUM_KW, 0 ); //  effect.
                    int                report_nr = ecl_kw_iget_int( seqnum_kw, 0 ); //

                    m_reportNr.push_back( report_nr );
                } //
            } //
            ecl_file_pop_block( m_ecl_file ); // <-------------------------------------------------------
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Close file
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::close()
{
    m_timeSteps.clear();
    m_daysSinceSimulationStart.clear();
}

//--------------------------------------------------------------------------------------------------
/// Get the number of time steps
//--------------------------------------------------------------------------------------------------
size_t RifEclipseUnifiedRestartFileAccess::timeStepCount()
{
    if ( m_timeSteps.size() == 0 )
    {
        extractTimestepsFromEclipse();
    }

    return m_timeSteps.size();
}

//--------------------------------------------------------------------------------------------------
/// Get the time steps
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::timeSteps( std::vector<QDateTime>* timeSteps,
                                                    std::vector<double>*    daysSinceSimulationStart )
{
    if ( m_timeSteps.size() == 0 )
    {
        extractTimestepsFromEclipse();
    }

    *timeSteps                = m_timeSteps;
    *daysSinceSimulationStart = m_daysSinceSimulationStart;
}

//--------------------------------------------------------------------------------------------------
/// Get list of result names
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::resultNames( QStringList* resultNames, std::vector<size_t>* resultDataItemCounts )
{
    if ( openFile() )
    {
        std::vector<ecl_file_type*> filesUsedToFindAvailableKeywords;
        filesUsedToFindAvailableKeywords.push_back( m_ecl_file );

        RifEclipseOutputFileTools::findKeywordsAndItemCount( filesUsedToFindAvailableKeywords,
                                                             resultNames,
                                                             resultDataItemCounts );
    }
}

//--------------------------------------------------------------------------------------------------
/// Get result values for given time step
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::results( const QString&       resultName,
                                                  size_t               timeStep,
                                                  size_t               gridCount,
                                                  std::vector<double>* values )
{
    if ( !openFile() )
    {
        return false;
    }

    ecl_file_push_block( m_ecl_file );

    for ( size_t i = 0; i < gridCount; i++ )
    {
        ecl_file_select_block( m_ecl_file, INTEHEAD_KW, static_cast<int>( timeStep * gridCount + i ) );

        int namedKeywordCount = ecl_file_get_num_named_kw( m_ecl_file, resultName.toLatin1().data() );
        for ( int iOcc = 0; iOcc < namedKeywordCount; iOcc++ )
        {
            std::vector<double> partValues;
            RifEclipseOutputFileTools::keywordData( m_ecl_file, resultName, iOcc, &partValues );

            values->insert( values->end(), partValues.begin(), partValues.end() );
        }
    }

    ecl_file_pop_block( m_ecl_file );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseUnifiedRestartFileAccess::dynamicNNCResults( const ecl_grid_type* grid,
                                                            size_t               timeStep,
                                                            std::vector<double>* waterFlux,
                                                            std::vector<double>* oilFlux,
                                                            std::vector<double>* gasFlux )
{
    if ( timeStep > timeStepCount() )
    {
        return false;
    }

    if ( !openFile() )
    {
        return false;
    }

    ecl_file_view_type* summaryView = ecl_file_get_restart_view( m_ecl_file, static_cast<int>( timeStep ), 0, 0, 0 );

    RifEclipseOutputFileTools::transferNncFluxData( grid, summaryView, waterFlux, oilFlux, gasFlux );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::readWellData( well_info_type* well_info, bool importCompleteMswData )
{
    if ( !well_info ) return;

    if ( openFile() )
    {
        // cvf::Trace::show("well_info_add_UNRST_wells Start"); // Use for profiling
        well_info_add_UNRST_wells( well_info, m_ecl_file, importCompleteMswData );
        // cvf::Trace::show("well_info_add_UNRST_wells End");   // Use for profiling
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseUnifiedRestartFileAccess::setRestartFiles( const QStringList& fileSet )
{
    m_filename = fileSet[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseUnifiedRestartFileAccess::readUnitsType()
{
    openFile();

    return RifEclipseOutputFileTools::readUnitsType( m_ecl_file );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifEclipseUnifiedRestartFileAccess::availablePhases() const
{
    return m_availablePhases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifEclipseUnifiedRestartFileAccess::reportNumbers()
{
    if ( m_timeSteps.size() == 0 )
    {
        extractTimestepsFromEclipse();
    }

    return m_reportNr;
}
