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

#include "RifEclipseOutputFileTools.h"

#include "RiaQDateTimeTools.h"
#include "RiaStringEncodingTools.h"
#include "RifEclipseRestartFilesetAccess.h"
#include "RifEclipseUnifiedRestartFileAccess.h"

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_grid.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_nnc_data.h"
#include "ert/ecl/ecl_nnc_geometry.h"

#include "cafProgressInfo.h"

#include "cvfMath.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFileInfo>

#include <algorithm>
#include <cassert>

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::RifEclipseOutputFileTools()
{
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseOutputFileTools::~RifEclipseOutputFileTools()
{
}

struct KeywordItemCounter
{
    KeywordItemCounter( const std::string& keyword, size_t aggregatedItemCount )
        : m_keyword( keyword )
        , m_aggregatedItemCount( aggregatedItemCount )
        , m_reportStepCount( 1 )
    {
    }

    bool operator==( const std::string& rhs ) const { return this->m_keyword == rhs; }

    std::string m_keyword;
    size_t      m_aggregatedItemCount;
    size_t      m_reportStepCount;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::findKeywordsAndItemCount( std::vector<ecl_file_type*> ecl_files,
                                                          QStringList*                resultNames,
                                                          std::vector<size_t>*        resultDataItemCounts )
{
    std::vector<RifRestartReportStep> reportSteps;
    RifEclipseOutputFileTools::createReportStepsMetaData( ecl_files, &reportSteps );

    std::vector<KeywordItemCounter> foundKeywords;

    for ( auto reportStep : reportSteps )
    {
        for ( auto keywordItemCount : reportStep.m_keywords.keywordsWithAggregatedItemCount() )
        {
            auto it = std::find( foundKeywords.begin(), foundKeywords.end(), keywordItemCount.first );
            if ( it == foundKeywords.end() )
            {
                foundKeywords.push_back( KeywordItemCounter( keywordItemCount.first, keywordItemCount.second ) );
            }
            else
            {
                it->m_aggregatedItemCount += keywordItemCount.second;
                it->m_reportStepCount++;
            }
        }
    }

    for ( auto stdKeyword : foundKeywords )
    {
        resultNames->push_back( QString::fromStdString( stdKeyword.m_keyword ) );
        resultDataItemCounts->push_back( stdKeyword.m_aggregatedItemCount );
    }
}

void getDayMonthYear( const ecl_kw_type* intehead_kw, int* day, int* month, int* year )
{
    assert( day && month && year );

    *day   = ecl_kw_iget_int( intehead_kw, INTEHEAD_DAY_INDEX );
    *month = ecl_kw_iget_int( intehead_kw, INTEHEAD_MONTH_INDEX );
    *year  = ecl_kw_iget_int( intehead_kw, INTEHEAD_YEAR_INDEX );
}

//--------------------------------------------------------------------------------------------------
/// Get list of time step texts (dates)
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::timeSteps( const ecl_file_type*    ecl_file,
                                           std::vector<QDateTime>* timeSteps,
                                           std::vector<double>*    daysSinceSimulationStart,
                                           size_t*                 perTimeStepHeaderKeywordCount )
{
    if ( !ecl_file ) return;

    CVF_ASSERT( timeSteps && daysSinceSimulationStart );

    timeSteps->clear();
    daysSinceSimulationStart->clear();

    // Get the number of occurrences of the INTEHEAD keyword
    int numINTEHEAD = ecl_file_get_num_named_kw( ecl_file, INTEHEAD_KW );

    // Get the number of occurrences of the DOUBHEAD keyword
    int numDOUBHEAD = ecl_file_get_num_named_kw( ecl_file, DOUBHEAD_KW );

    std::vector<double> dayValues( numINTEHEAD, 0.0 ); // Init fraction to zero

    // Read out fraction of day if number of keywords are identical
    if ( numINTEHEAD == numDOUBHEAD )
    {
        for ( int i = 0; i < numDOUBHEAD; i++ )
        {
            ecl_kw_type* kwDOUBHEAD = ecl_file_iget_named_kw( ecl_file, DOUBHEAD_KW, i );
            if ( kwDOUBHEAD )
            {
                dayValues[i] = ecl_kw_iget_double( kwDOUBHEAD, DOUBHEAD_DAYS_INDEX );
            }
        }
    }

    bool useStartOfSimulationDate = true;
    {
        // See https://github.com/OPM/ResInsight/issues/4770

        std::set<std::tuple<int, int, int>> uniqueDays;

        for ( int i = 0; i < numINTEHEAD; i++ )
        {
            ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, i );
            CVF_ASSERT( kwINTEHEAD );
            int day   = 0;
            int month = 0;
            int year  = 0;
            getDayMonthYear( kwINTEHEAD, &day, &month, &year );

            uniqueDays.insert( std::make_tuple( day, month, year ) );
        }

        std::set<double> uniqueDayValues;
        for ( auto dayValue : dayValues )
        {
            uniqueDayValues.insert( dayValue );
        }

        if ( uniqueDays.size() == 1 && uniqueDayValues.size() == 1 )
        {
            int day   = 0;
            int month = 0;
            int year  = 0;

            std::tie( day, month, year ) = *( uniqueDays.begin() );

            QDateTime reportDateTime = RiaQDateTimeTools::createUtcDateTime( QDate( year, month, day ) );

            timeSteps->push_back( reportDateTime );
            daysSinceSimulationStart->push_back( *uniqueDayValues.begin() );

            // Early return, we have only one unique time step
            // This state has been seen for cases with one file for each time step
            // See https://github.com/OPM/ResInsight/issues/4904

            return;
        }

        // Some simulations might end up with wrong data reported for day/month/year. If this situation is detected,
        // base all time step on double values and use start of simulation as first date
        // See https://github.com/OPM/ResInsight/issues/4850
        if ( uniqueDays.size() == dayValues.size() ) useStartOfSimulationDate = false;
    }

    std::set<QDateTime> existingTimesteps;
    for ( int i = 0; i < numINTEHEAD; i++ )
    {
        ecl_kw_type* kwINTEHEAD = nullptr;

        if ( useStartOfSimulationDate )
        {
            kwINTEHEAD = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, 0 );
        }
        else
        {
            kwINTEHEAD = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, i );
        }

        CVF_ASSERT( kwINTEHEAD );
        int day   = 0;
        int month = 0;
        int year  = 0;
        getDayMonthYear( kwINTEHEAD, &day, &month, &year );

        QDateTime reportDateTime = RiaQDateTimeTools::createUtcDateTime( QDate( year, month, day ) );
        CVF_ASSERT( reportDateTime.isValid() );

        double dayDoubleValue = dayValues[i];
        int    dayValue       = cvf::Math::floor( dayDoubleValue );
        if ( useStartOfSimulationDate )
        {
            reportDateTime = reportDateTime.addDays( dayValue );
        }

        double dayFraction  = dayDoubleValue - dayValue;
        double milliseconds = dayFraction * 24.0 * 60.0 * 60.0 * 1000.0;

        reportDateTime = reportDateTime.addMSecs( milliseconds );

        if ( existingTimesteps.insert( reportDateTime ).second )
        {
            timeSteps->push_back( reportDateTime );
            daysSinceSimulationStart->push_back( dayDoubleValue );
        }
    }

    if ( perTimeStepHeaderKeywordCount )
    {
        // 6X simulator can report more then one keyword block per time step. Report the total number of keywords per
        // time steps to be able to read data from correct index
        //
        // See https://github.com/OPM/ResInsight/issues/5763
        //
        *perTimeStepHeaderKeywordCount = numINTEHEAD / timeSteps->size();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordData( const ecl_file_type* ecl_file,
                                             const QString&       keyword,
                                             size_t               fileKeywordOccurrence,
                                             std::vector<double>* values )
{
    bool result = false;

#pragma omp critical( critical_section_keywordData_double )
    {
        ecl_kw_type* kwData = ecl_file_iget_named_kw( ecl_file,
                                                      RiaStringEncodingTools::toNativeEncoded( keyword ).data(),
                                                      static_cast<int>( fileKeywordOccurrence ) );
        if ( kwData )
        {
            size_t numValues = ecl_kw_get_size( kwData );

            std::vector<double> doubleData;
            doubleData.resize( numValues );

            ecl_kw_get_data_as_double( kwData, doubleData.data() );
            values->insert( values->end(), doubleData.begin(), doubleData.end() );

            result = true;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::keywordData( const ecl_file_type* ecl_file,
                                             const QString&       keyword,
                                             size_t               fileKeywordOccurrence,
                                             std::vector<int>*    values )
{
    bool result = false;

#pragma omp critical( critical_section_keywordData_int )
    {
        ecl_kw_type* kwData = ecl_file_iget_named_kw( ecl_file,
                                                      RiaStringEncodingTools::toNativeEncoded( keyword ).data(),
                                                      static_cast<int>( fileKeywordOccurrence ) );
        if ( kwData )
        {
            size_t numValues = ecl_kw_get_size( kwData );

            std::vector<int> integerData;
            integerData.resize( numValues );

            ecl_kw_get_memcpy_int_data( kwData, integerData.data() );
            values->insert( values->end(), integerData.begin(), integerData.end() );

            result = true;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Get first occurrence of file of given type in given list of filenames, as filename or nullptr if not found
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputFileTools::firstFileNameOfType( const QStringList& fileSet, ecl_file_enum fileType )
{
    int i;
    for ( i = 0; i < fileSet.count(); i++ )
    {
        bool formatted    = false;
        int  reportNumber = -1;
        if ( ecl_util_get_file_type( RiaStringEncodingTools::toNativeEncoded( fileSet.at( i ) ).data(),
                                     &formatted,
                                     &reportNumber ) == fileType )
        {
            return fileSet.at( i );
        }
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
/// Get all files of the given type from the provided list of filenames
//--------------------------------------------------------------------------------------------------
QStringList RifEclipseOutputFileTools::filterFileNamesOfType( const QStringList& fileSet, ecl_file_enum fileType )
{
    QStringList fileNames;

    int i;
    for ( i = 0; i < fileSet.count(); i++ )
    {
        bool formatted    = false;
        int  reportNumber = -1;
        if ( ecl_util_get_file_type( RiaStringEncodingTools::toNativeEncoded( fileSet.at( i ) ).data(),
                                     &formatted,
                                     &reportNumber ) == fileType )
        {
            fileNames.append( fileSet.at( i ) );
        }
    }

    return fileNames;
}

//-------------------------------------------------------------------------------------------------------
/// Check if libecl accepts the file name. libecl refuses to open files with mixed case in the file name.
//-------------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::isValidEclipseFileName( const QString& fileName )
{
    QString fileNameBase = QFileInfo( fileName ).completeBaseName();
    return ecl_util_valid_basename( RiaStringEncodingTools::toNativeEncoded( fileNameBase ).data() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QByteArray RifEclipseOutputFileTools::md5sum( const QString& fileName )
{
    QFile file( fileName );
    if ( file.open( QFile::ReadOnly ) )
    {
        QCryptographicHash hash( QCryptographicHash::Md5 );
        QByteArray         fileContent = file.readAll();
        if ( !fileContent.isEmpty() )
        {
            hash.addData( fileContent );
            return hash.result();
        }
    }
    return QByteArray();
}

//--------------------------------------------------------------------------------------------------
/// Get set of Eclipse files based on an input file and its path
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( const QString& fullPathFileName,
                                                                  QStringList*   baseNameFiles )
{
    CVF_ASSERT( baseNameFiles );
    baseNameFiles->clear();

    QString filePath     = QFileInfo( fullPathFileName ).absoluteFilePath();
    filePath             = QFileInfo( filePath ).path();
    QString fileNameBase = QFileInfo( fullPathFileName ).completeBaseName();

    stringlist_type* eclipseFiles = stringlist_alloc_new();
    ecl_util_select_filelist( RiaStringEncodingTools::toNativeEncoded( filePath ).data(),
                              RiaStringEncodingTools::toNativeEncoded( fileNameBase ).data(),
                              ECL_OTHER_FILE,
                              false,
                              eclipseFiles );

    int i;
    for ( i = 0; i < stringlist_get_size( eclipseFiles ); i++ )
    {
        baseNameFiles->append( RiaStringEncodingTools::fromNativeEncoded( stringlist_safe_iget( eclipseFiles, i ) ) );
    }

    stringlist_free( eclipseFiles );

    return baseNameFiles->count() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::readGridDimensions( const QString&                 gridFileName,
                                                    std::vector<std::vector<int>>& gridDimensions )
{
    ecl_grid_type* grid = ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( gridFileName ).data() ); // bootstrap
                                                                                                            // ecl_grid
                                                                                                            // instance
    stringlist_type* lgr_names = ecl_grid_alloc_lgr_name_list( grid ); // get a list of all the lgr names.

    // printf("grid:%s has %d a total of %d lgr's \n", grid_filename , stringlist_get_size( lgr_names ));
    for ( int lgr_nr = 0; lgr_nr < stringlist_get_size( lgr_names ); lgr_nr++ )
    {
        ecl_grid_type* lgr_grid = ecl_grid_get_lgr( grid,
                                                    stringlist_iget( lgr_names,
                                                                     lgr_nr ) ); // get the ecl_grid instance of the lgr
                                                                                 // - by name.

        int nx, ny, nz, active_size;
        ecl_grid_get_dims( lgr_grid, &nx, &ny, &nz, &active_size ); // get some size info from this lgr.

        std::vector<int> values;
        values.push_back( nx );
        values.push_back( ny );
        values.push_back( nz );
        values.push_back( active_size );

        gridDimensions.push_back( values );
    }

    ecl_grid_free( grid );
    stringlist_free( lgr_names );
}

//--------------------------------------------------------------------------------------------------
/// Returns the following integer values from the first INTEHEAD keyword found
///  1  : METRIC
///  2  : FIELD
///  3  : LAB
///  -1 : No INTEHEAD keyword found
//--------------------------------------------------------------------------------------------------
int RifEclipseOutputFileTools::readUnitsType( const ecl_file_type* ecl_file )
{
    int unitsType = -1;

    if ( ecl_file )
    {
        ecl_kw_type* kwINTEHEAD = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, 0 );
        if ( kwINTEHEAD )
        {
            unitsType = ecl_kw_iget_int( kwINTEHEAD, INTEHEAD_UNIT_INDEX );
        }
    }

    return unitsType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifEclipseRestartDataAccess> RifEclipseOutputFileTools::createDynamicResultAccess( const QString& fileName )
{
    QStringList filesWithSameBaseName;
    RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &filesWithSameBaseName );

    cvf::ref<RifEclipseRestartDataAccess> resultsAccess;

    // Look for unified restart file
    QString unrstFileName =
        RifEclipseOutputFileTools::firstFileNameOfType( filesWithSameBaseName, ECL_UNIFIED_RESTART_FILE );
    if ( unrstFileName.size() > 0 )
    {
        resultsAccess = new RifEclipseUnifiedRestartFileAccess();
        resultsAccess->setRestartFiles( QStringList( unrstFileName ) );
    }
    else
    {
        // Look for set of restart files (one file per time step)
        QStringList restartFiles =
            RifEclipseOutputFileTools::filterFileNamesOfType( filesWithSameBaseName, ECL_RESTART_FILE );
        if ( restartFiles.size() > 0 )
        {
            resultsAccess = new RifEclipseRestartFilesetAccess();
            resultsAccess->setRestartFiles( restartFiles );
        }
    }

    return resultsAccess;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseOutputFileTools::createIndexFileName( const QString& resultFileName )
{
    QFileInfo fi( resultFileName );

    QString indexFileName = fi.path() + "/" + fi.completeBaseName() + ".RESINSIGHT_IDX";

    return indexFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifEclipseOutputFileTools::findAvailablePhases( const ecl_file_type* ecl_file )
{
    std::set<RiaDefines::PhaseType> phaseTypes;

    if ( ecl_file )
    {
        const ecl_kw_type* intehead = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, 0 );
        if ( intehead )
        {
            int phases = ecl_kw_iget_int( intehead, INTEHEAD_PHASE_INDEX );

            if ( phases & ECL_OIL_PHASE )
            {
                phaseTypes.insert( RiaDefines::PhaseType::OIL_PHASE );
            }

            if ( phases & ECL_GAS_PHASE )
            {
                phaseTypes.insert( RiaDefines::PhaseType::GAS_PHASE );
            }

            if ( phases & ECL_WATER_PHASE )
            {
                phaseTypes.insert( RiaDefines::PhaseType::WATER_PHASE );
            }
        }
    }

    return phaseTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::transferNncFluxData( const ecl_grid_type*      grid,
                                                     const ecl_file_view_type* summaryView,
                                                     std::vector<double>*      waterFlux,
                                                     std::vector<double>*      oilFlux,
                                                     std::vector<double>*      gasFlux )
{
    ecl_nnc_geometry_type* nnc_geo = ecl_nnc_geometry_alloc( grid );
    if ( nnc_geo )
    {
        ecl_nnc_data_type* waterFluxData = ecl_nnc_data_alloc_wat_flux( grid, nnc_geo, summaryView );
        if ( waterFluxData )
        {
            const double* waterFluxValues = ecl_nnc_data_get_values( waterFluxData );
            waterFlux->insert( waterFlux->end(),
                               &waterFluxValues[0],
                               &waterFluxValues[ecl_nnc_data_get_size( waterFluxData )] );
            ecl_nnc_data_free( waterFluxData );
        }

        ecl_nnc_data_type* oilFluxData = ecl_nnc_data_alloc_oil_flux( grid, nnc_geo, summaryView );
        if ( oilFluxData )
        {
            const double* oilFluxValues = ecl_nnc_data_get_values( oilFluxData );
            oilFlux->insert( oilFlux->end(), &oilFluxValues[0], &oilFluxValues[ecl_nnc_data_get_size( oilFluxData )] );
            ecl_nnc_data_free( oilFluxData );
        }

        ecl_nnc_data_type* gasFluxData = ecl_nnc_data_alloc_gas_flux( grid, nnc_geo, summaryView );
        if ( gasFluxData )
        {
            const double* gasFluxValues = ecl_nnc_data_get_values( gasFluxData );
            gasFlux->insert( gasFlux->end(), &gasFluxValues[0], &gasFluxValues[ecl_nnc_data_get_size( gasFluxData )] );
            ecl_nnc_data_free( gasFluxData );
        }

        ecl_nnc_geometry_free( nnc_geo );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseOutputFileTools::isExportedFromIntersect( const ecl_file_type* ecl_file )
{
    // This code is taken from ecl_file_get_ecl_version() in ecl_file.cpp

    ecl_kw_type* intehead_kw = ecl_file_iget_named_kw( ecl_file, INTEHEAD_KW, 0 );
    if ( !intehead_kw ) return false;

    int int_value = ecl_kw_iget_int( intehead_kw, INTEHEAD_IPROG_INDEX );
    if ( int_value == INTEHEAD_INTERSECT_VALUE )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Convenience method to hide C fopen calls in #pragma declarations to avoid warnings on Windows
//--------------------------------------------------------------------------------------------------
FILE* RifEclipseOutputFileTools::fopen( const QString& filePath, const QString& mode )
{
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif

    FILE* filePtr = std::fopen( RiaStringEncodingTools::toNativeEncoded( filePath ).data(),
                                RiaStringEncodingTools::toNativeEncoded( mode ).data() );

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    return filePtr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseOutputFileTools::createReportStepsMetaData( std::vector<ecl_file_type*>        ecl_files,
                                                           std::vector<RifRestartReportStep>* reportSteps )
{
    if ( !reportSteps ) return;

    for ( auto ecl_file : ecl_files )
    {
        if ( !ecl_file ) continue;

        int reportStepCount = ecl_file_get_num_named_kw( ecl_file, INTEHEAD_KW );
        for ( int reportStepIndex = 0; reportStepIndex < reportStepCount; reportStepIndex++ )
        {
            ecl_file_view_type* rst_view = ecl_file_get_global_view( ecl_file );
            if ( !rst_view ) continue;

            ecl_rsthead_type* restart_header = ecl_rsthead_alloc( rst_view, reportStepIndex );
            if ( restart_header )
            {
                ecl_file_push_block( ecl_file );

                {
                    ecl_file_select_block( ecl_file, INTEHEAD_KW, reportStepIndex );

                    RifRestartReportStep reportStep;

                    // Set Date
                    {
                        QDateTime reportDateTime(
                            QDate( restart_header->year, restart_header->month, restart_header->day ) );
                        reportStep.dateTime = reportDateTime;
                    }

                    // Find number of keywords within this report step
                    int numKeywords = ecl_file_get_num_distinct_kw( ecl_file );
                    for ( int iKey = 0; iKey < numKeywords; iKey++ )
                    {
                        const char* kw = ecl_file_iget_distinct_kw( ecl_file, iKey );

                        int namedKeywordCount = ecl_file_get_num_named_kw( ecl_file, kw );
                        for ( int iOcc = 0; iOcc < namedKeywordCount; iOcc++ )
                        {
                            ecl_data_type dataType     = ecl_file_iget_named_data_type( ecl_file, kw, iOcc );
                            ecl_type_enum dataTypeEmum = ecl_type_get_type( dataType );
                            if ( dataTypeEmum != ECL_DOUBLE_TYPE && dataTypeEmum != ECL_FLOAT_TYPE &&
                                 dataTypeEmum != ECL_INT_TYPE )
                            {
                                continue;
                            }

                            int itemCount = ecl_file_iget_named_size( ecl_file, kw, iOcc );
                            reportStep.m_keywords.appendKeyword( kw, itemCount, iOcc );
                        }
                    }

                    reportSteps->push_back( reportStep );
                }

                ecl_file_pop_block( ecl_file );

                ecl_rsthead_free( restart_header );
            }
        }
    }
}
