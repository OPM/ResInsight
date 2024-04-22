/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifEclipseSummaryTools.h"

#include "RiaFilePathTools.h"
#include "RiaStringEncodingTools.h"
#include "RiaSummaryAddressAnalyzer.h"
#include "RifSummaryReaderInterface.h"

#include "cafAppEnum.h"

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_sum.h"
#include "ert/ecl/ecl_util.h"
#include "ert/ecl/smspec_node.hpp"

#include <QDateTime>
#include <QDir>
#include <QString>
#include <QStringList>

#include <cassert>
#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryHeaderFile( const QString& inputFile, QString* headerFile, bool* isFormatted )
{
    findSummaryHeaderFileInfo( inputFile, headerFile, nullptr, nullptr, isFormatted );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryFiles( const QString& inputFile, QString* headerFile, QStringList* dataFiles )
{
    dataFiles->clear();
    headerFile->clear();

    char* myPath      = nullptr;
    char* myBase      = nullptr;
    char* myExtension = nullptr;

    util_alloc_file_components( RiaStringEncodingTools::toNativeEncoded( inputFile ).data(), &myPath, &myBase, &myExtension );

    QString path;
    if ( myPath ) path = RiaStringEncodingTools::fromNativeEncoded( myPath );
    QString base;
    if ( myBase ) base = RiaStringEncodingTools::fromNativeEncoded( myBase );
    std::string extension;
    if ( myExtension ) extension = myExtension;

    free( myExtension );
    free( myBase );
    free( myPath );

    if ( path.isEmpty() || base.isEmpty() ) return;

    char*            myHeaderFile      = nullptr;
    stringlist_type* summary_file_list = stringlist_alloc_new();

    ecl_util_alloc_summary_files( RiaStringEncodingTools::toNativeEncoded( path ).data(),
                                  RiaStringEncodingTools::toNativeEncoded( base ).data(),
                                  extension.data(),
                                  &myHeaderFile,
                                  summary_file_list );
    if ( myHeaderFile )
    {
        ( *headerFile ) = RiaStringEncodingTools::fromNativeEncoded( myHeaderFile );
        free( myHeaderFile );
    }

    if ( stringlist_get_size( summary_file_list ) > 0 )
    {
        for ( int i = 0; i < stringlist_get_size( summary_file_list ); i++ )
        {
            dataFiles->push_back( RiaStringEncodingTools::fromNativeEncoded( stringlist_iget( summary_file_list, i ) ) );
        }
    }
    stringlist_free( summary_file_list );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile( const QString& summaryHeaderFile )
{
    char* myPath = nullptr;
    char* myBase = nullptr;

    util_alloc_file_components( RiaStringEncodingTools::toNativeEncoded( QDir::toNativeSeparators( summaryHeaderFile ) ).data(),
                                &myPath,
                                &myBase,
                                nullptr );

    char* caseFile = ecl_util_alloc_exfilename( myPath, myBase, ECL_EGRID_FILE, true, -1 );
    if ( !caseFile )
    {
        caseFile = ecl_util_alloc_exfilename( myPath, myBase, ECL_EGRID_FILE, false, -1 );
    }

    QString gridCaseFile;

    if ( caseFile ) gridCaseFile = caseFile;

    free( caseFile );
    free( myBase );
    free( myPath );

    return RiaFilePathTools::toInternalSeparator( gridCaseFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::dumpMetaData( RifSummaryReaderInterface* readerEclipseSummary )
{
    const auto addresses = readerEclipseSummary->allResultAddresses();

    for ( const auto& catAddresse : addresses )
    {
        std::cout << catAddresse.vectorName() << " " << catAddresse.regionNumber() << " " << catAddresse.regionNumber2() << " "
                  << catAddresse.groupName() << " " << catAddresse.wellName() << " " << catAddresse.wellSegmentNumber() << " "
                  << catAddresse.lgrName() << " " << catAddresse.cellI() << " " << catAddresse.cellJ() << " " << catAddresse.cellK()
                  << std::endl;
    }

    std::cout << std::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifRestartFileInfo> RifEclipseSummaryTools::getRestartFiles( const QString& headerFileName, std::vector<QString>& warnings )
{
    std::vector<RifRestartFileInfo> restartFiles;

    std::set<QString> restartFilesOpened;

    RifRestartFileInfo currFile;
    currFile.fileName = headerFileName;
    while ( !currFile.fileName.isEmpty() )
    {
        // Due to a weakness in resdata regarding restart summary header file selection,
        // do some extra checking
        {
            QString formattedHeaderExtension    = ".FSMSPEC";
            QString nonformattedHeaderExtension = ".SMSPEC";
            QString formattedDataFileExtension  = ".FUNSMRY";

            if ( currFile.fileName.endsWith( nonformattedHeaderExtension, Qt::CaseInsensitive ) )
            {
                QString formattedHeaderFile = currFile.fileName;
                formattedHeaderFile.replace( nonformattedHeaderExtension, formattedHeaderExtension, Qt::CaseInsensitive );
                QString formattedDateFile = currFile.fileName;
                formattedDateFile.replace( nonformattedHeaderExtension, formattedDataFileExtension, Qt::CaseInsensitive );

                QFileInfo nonformattedHeaderFileInfo = QFileInfo( currFile.fileName );
                QFileInfo formattedHeaderFileInfo    = QFileInfo( formattedHeaderFile );
                QFileInfo formattedDateFileInfo      = QFileInfo( formattedDateFile );
                if ( formattedHeaderFileInfo.lastModified() < nonformattedHeaderFileInfo.lastModified() &&
                     formattedHeaderFileInfo.exists() && !formattedDateFileInfo.exists() )
                {
                    warnings.push_back( QString( "RifReaderEclipseSummary: Formatted summary header file without an\n" ) +
                                        QString( "associated data file detected.\n" ) +
                                        QString( "This may cause a failure reading summary origin data.\n" ) +
                                        QString( "To avoid this problem, please delete or rename the.FSMSPEC file." ) );
                    break;
                }
            }
            QString prevFile = currFile.fileName;
            currFile         = getRestartFile( currFile.fileName );

            // Fix to stop potential infinite loop
            if ( currFile.fileName == prevFile )
            {
                warnings.push_back( "RifReaderEclipseSummary: Restart file reference loop detected" );
                break;
            }
            if ( restartFilesOpened.count( currFile.fileName ) != 0u )
            {
                warnings.push_back( "RifReaderEclipseSummary: Same restart file being opened multiple times" );
            }
            restartFilesOpened.insert( currFile.fileName );
        }

        if ( !currFile.fileName.isEmpty() ) restartFiles.push_back( currFile );
    }
    return restartFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RifEclipseSummaryTools::getFileInfo( const QString& headerFileName )
{
    RifRestartFileInfo  fileInfo;
    ecl_sum_type*       ecl_sum   = openEclSum( headerFileName, false );
    std::vector<time_t> timeSteps = getTimeSteps( ecl_sum );
    if ( !timeSteps.empty() )
    {
        fileInfo.fileName  = headerFileName;
        fileInfo.startDate = timeSteps.front();
        fileInfo.endDate   = timeSteps.back();
    }
    closeEclSum( ecl_sum );

    return fileInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryHeaderFileInfo( const QString& inputFile, QString* headerFile, QString* path, QString* base, bool* isFormatted )
{
    char* myPath        = nullptr;
    char* myBase        = nullptr;
    bool  formattedFile = true;

    util_alloc_file_components( RiaStringEncodingTools::toNativeEncoded( QDir::toNativeSeparators( inputFile ) ).data(), &myPath, &myBase, nullptr );

    char* myHeaderFile = ecl_util_alloc_exfilename( myPath, myBase, ECL_SUMMARY_HEADER_FILE, true, -1 );
    if ( !myHeaderFile )
    {
        myHeaderFile = ecl_util_alloc_exfilename( myPath, myBase, ECL_SUMMARY_HEADER_FILE, false, -1 );
        if ( myHeaderFile )
        {
            formattedFile = false;
        }
    }

    if ( myHeaderFile && headerFile ) *headerFile = RiaFilePathTools::toInternalSeparator( myHeaderFile );
    if ( myPath && path ) *path = RiaFilePathTools::toInternalSeparator( myPath );
    if ( myBase && base ) *base = RiaFilePathTools::toInternalSeparator( myBase );
    if ( isFormatted ) *isFormatted = formattedFile;

    free( myHeaderFile );
    free( myBase );
    free( myPath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RifEclipseSummaryTools::getRestartFile( const QString& headerFileName )
{
    ecl_sum_type* ecl_sum = openEclSum( headerFileName, true );

    const ecl_smspec_type* smspec  = ecl_sum ? ecl_sum_get_smspec( ecl_sum ) : nullptr;
    const char*            rstCase = smspec ? ecl_smspec_get_restart_case( smspec ) : nullptr;
    QString restartCase            = rstCase ? RiaFilePathTools::canonicalPath( RiaStringEncodingTools::fromNativeEncoded( rstCase ) ) : "";
    closeEclSum( ecl_sum );

    if ( !restartCase.isEmpty() )
    {
        QString path        = QFileInfo( restartCase ).dir().path();
        QString restartBase = QDir( restartCase ).dirName();

        char*   smspec_header   = ecl_util_alloc_exfilename( path.toStdString().data(),
                                                         restartBase.toStdString().data(),
                                                         ECL_SUMMARY_HEADER_FILE,
                                                         false /*unformatted*/,
                                                         0 );
        QString restartFileName = RiaFilePathTools::toInternalSeparator( RiaStringEncodingTools::fromNativeEncoded( smspec_header ) );
        free( smspec_header );

        return getFileInfo( restartFileName );
    }
    return RifRestartFileInfo();
}

std::vector<time_t> RifEclipseSummaryTools::getTimeSteps( ecl_sum_type* ecl_sum )
{
    std::vector<time_t> timeSteps;

    if ( ecl_sum )
    {
        time_t_vector_type* steps = ecl_sum_alloc_time_vector( ecl_sum, false );

        if ( steps )
        {
            for ( int i = 0; i < time_t_vector_size( steps ); i++ )
            {
                timeSteps.push_back( time_t_vector_iget( steps, i ) );
            }

            time_t_vector_free( steps );
        }
    }
    return timeSteps;
}

RiaDefines::EclipseUnitSystem RifEclipseSummaryTools::readUnitSystem( ecl_sum_type* ecl_sum )
{
    ert_ecl_unit_enum eclUnitEnum = ecl_sum_get_unit_system( ecl_sum );
    switch ( eclUnitEnum )
    {
        case ECL_METRIC_UNITS:
            return RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        case ECL_FIELD_UNITS:
            return RiaDefines::EclipseUnitSystem::UNITS_FIELD;
        case ECL_LAB_UNITS:
            return RiaDefines::EclipseUnitSystem::UNITS_LAB;
        default:
            return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ecl_sum_type* RifEclipseSummaryTools::openEclSum( const QString& inHeaderFileName, bool includeRestartFiles )
{
    QString     headerFileName;
    QStringList dataFileNames;
    QString     nativeHeaderFileName = QDir::toNativeSeparators( inHeaderFileName );
    RifEclipseSummaryTools::findSummaryFiles( nativeHeaderFileName, &headerFileName, &dataFileNames );

    if ( headerFileName.isEmpty() || dataFileNames.isEmpty() ) return nullptr;

    stringlist_type* dataFiles = stringlist_alloc_new();
    for ( int i = 0; i < dataFileNames.size(); i++ )
    {
        stringlist_append_copy( dataFiles, RiaStringEncodingTools::toNativeEncoded( dataFileNames[i] ).data() );
    }

    bool        lazyLoad                     = true;
    std::string itemSeparatorInVariableNames = ":";

    ecl_sum_type* ecl_sum = nullptr;
    try
    {
        ecl_sum = ecl_sum_fread_alloc( RiaStringEncodingTools::toNativeEncoded( headerFileName ).data(),
                                       dataFiles,
                                       itemSeparatorInVariableNames.data(),
                                       includeRestartFiles,
                                       lazyLoad,
                                       ECL_FILE_CLOSE_STREAM );
    }
    catch ( ... )
    {
        ecl_sum = nullptr;
    }

    stringlist_free( dataFiles );

    return ecl_sum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::closeEclSum( ecl_sum_type* ecl_sum )
{
    if ( ecl_sum ) ecl_sum_free( ecl_sum );
}
