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
#include "RiaStdStringTools.h"
#include "RiaStringEncodingTools.h"
#include "Summary/RiaSummaryAddressAnalyzer.h"

#include "RifSummaryReaderInterface.h"

#include "cafAppEnum.h"

#include "ert/ecl/ecl_file.h"
#include "ert/ecl/ecl_kw.h"
#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_sum.h"
#include "ert/ecl/ecl_util.h"
#include "ert/ecl/smspec_node.hpp"

#include "opm/io/eclipse/EclFile.hpp"

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
std::vector<QString> RifEclipseSummaryTools::getRestartFileNames( const QString& headerFileName, std::vector<QString>& warnings )
{
    const bool useOpmReader = false;
    return getRestartFileNames( headerFileName, useOpmReader, warnings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString>
    RifEclipseSummaryTools::getRestartFileNames( const QString& headerFileName, bool useOpmReader, std::vector<QString>& warnings )
{
    std::vector<QString> restartFiles;

    std::set<QString> restartFilesOpened;

    QString currentFileName = headerFileName;
    while ( !currentFileName.isEmpty() )
    {
        // Due to a weakness in resdata regarding restart summary header file selection,
        // do some extra checking
        {
            QString formattedHeaderExtension    = ".FSMSPEC";
            QString nonformattedHeaderExtension = ".SMSPEC";
            QString formattedDataFileExtension  = ".FUNSMRY";

            if ( currentFileName.endsWith( nonformattedHeaderExtension, Qt::CaseInsensitive ) )
            {
                QString formattedHeaderFile = currentFileName;
                formattedHeaderFile.replace( nonformattedHeaderExtension, formattedHeaderExtension, Qt::CaseInsensitive );
                QString formattedDateFile = currentFileName;
                formattedDateFile.replace( nonformattedHeaderExtension, formattedDataFileExtension, Qt::CaseInsensitive );

                QFileInfo nonformattedHeaderFileInfo = QFileInfo( currentFileName );
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

            QString relativeFilePathFromFile;
            if ( useOpmReader )
            {
                relativeFilePathFromFile = getRestartRelativeFilePathOpm( currentFileName );
            }
            else
            {
                relativeFilePathFromFile = getRestartRelativeFilePathResdata( currentFileName );
            }

            QString prevFileName = currentFileName;
            if ( !relativeFilePathFromFile.isEmpty() )
            {
                currentFileName = getRestartFilePath( currentFileName, relativeFilePathFromFile );
            }
            else
            {
                currentFileName.clear();
            }

            // Fix to stop potential infinite loop
            if ( currentFileName == prevFileName )
            {
                warnings.push_back( "RifReaderEclipseSummary: Restart file reference loop detected" );
                break;
            }
            if ( restartFilesOpened.count( currentFileName ) != 0u )
            {
                warnings.push_back( "RifReaderEclipseSummary: Same restart file being opened multiple times" );
            }
            restartFilesOpened.insert( currentFileName );
        }

        if ( !currentFileName.isEmpty() ) restartFiles.push_back( currentFileName );
    }
    return restartFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseSummaryTools::getRestartFilePath( const QString& headerFileName, const QString& restartCaseNameFromFile )
{
    QFileInfo sourceFileInfo( headerFileName );
    QString   suffix = sourceFileInfo.suffix();

    QString filePath = sourceFileInfo.absolutePath() + RiaFilePathTools::separator() + restartCaseNameFromFile + "." + suffix;

    QFileInfo restartFileInfo( filePath );
    QString   restartFileName = RiaFilePathTools::toInternalSeparator( restartFileInfo.absoluteFilePath() );

    return restartFileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRestartFileInfo RifEclipseSummaryTools::getFileInfoAndTimeSteps( const QString& headerFileName )
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
std::vector<QString> RifEclipseSummaryTools::getRestartFileNamesOpm( const QString& headerFileName, std::vector<QString>& warnings )
{
    return getRestartFileNames( headerFileName, true, warnings );
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
QString RifEclipseSummaryTools::getRestartRelativeFilePathResdata( const QString& headerFileName )
{
    QString restartCaseNameFromFile;
    if ( ecl_file_type* header = ecl_file_open( headerFileName.toStdString().data(), 0 ) )
    {
        // Code taken from ecl_smspec_load_restart() in ecl_smspec.cpp
        if ( ecl_file_has_kw( header, RESTART_KW ) )
        {
            if ( const ecl_kw_type* restart_kw = ecl_file_iget_named_kw( header, RESTART_KW, 0 ) )
            {
                char tmp_base[137];

                tmp_base[0] = '\0';
                for ( int i = 0; i < ecl_kw_get_size( restart_kw ); i++ )
                    strcat( tmp_base, (const char*)ecl_kw_iget_ptr( restart_kw, i ) );

                // The case name is a relative path without the .SMSPEC extension
                restartCaseNameFromFile = tmp_base;
                restartCaseNameFromFile = restartCaseNameFromFile.trimmed();
            }
        }

        ecl_file_close( header );
    }

    return restartCaseNameFromFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseSummaryTools::getRestartRelativeFilePathOpm( const QString& headerFileName )
{
    try
    {
        Opm::EclIO::EclFile eclFile( headerFileName.toStdString() );
        eclFile.loadData( "RESTART" );

        std::string restartCaseNameFromFile;

        auto restartData = eclFile.get<std::string>( "RESTART" );
        for ( const auto& string : restartData )
        {
            restartCaseNameFromFile += string;
        }

        return QString::fromStdString( restartCaseNameFromFile );
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> RifEclipseSummaryTools::splitVectorNameAndSuffix( const std::string& vectorName )
{
    std::string vectorNameToUse = vectorName;
    std::string suffix;
    if ( RiaStdStringTools::endsWith( vectorNameToUse, RifEclipseSummaryAddressDefines::differenceIdentifier() ) )
    {
        suffix = RifEclipseSummaryAddressDefines::differenceIdentifier();
        vectorNameToUse = vectorNameToUse.substr( 0, vectorNameToUse.size() - RifEclipseSummaryAddressDefines::differenceIdentifier().size() );
    }
    else if ( RiaStdStringTools::endsWith( vectorNameToUse, RifEclipseSummaryAddressDefines::historyIdentifier() ) )
    {
        suffix          = RifEclipseSummaryAddressDefines::historyIdentifier();
        vectorNameToUse = vectorNameToUse.substr( 0, vectorNameToUse.size() - RifEclipseSummaryAddressDefines::historyIdentifier().size() );
    }
    return { vectorNameToUse, suffix };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifEclipseSummaryTools::nativeVectorNames( const std::vector<std::string>& vectorNames )
{
    std::vector<std::string> nativeNames;

    for ( const auto& s : vectorNames )
    {
        const auto [vectorName, suffix] = RifEclipseSummaryTools::splitVectorNameAndSuffix( s );
        if ( std::find( nativeNames.begin(), nativeNames.end(), vectorName ) == nativeNames.end() )
        {
            nativeNames.push_back( vectorName );
        }
    }

    return nativeNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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
