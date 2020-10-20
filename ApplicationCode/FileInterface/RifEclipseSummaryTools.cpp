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
#include "RiaSummaryCurveAnalyzer.h"
#include "RifReaderEclipseSummary.h"

#include "cafAppEnum.h"

#include "ert/ecl/ecl_util.h"

#include <QDir>
#include <QString>
#include <QStringList>

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
    std::set<RifEclipseSummaryAddress> addresses = readerEclipseSummary->allResultAddresses();

    for ( int category = 0; category < RifEclipseSummaryAddress::SUMMARY_BLOCK_LGR; category++ )
    {
        RifEclipseSummaryAddress::SummaryVarCategory categoryEnum =
            RifEclipseSummaryAddress::SummaryVarCategory( category );

        std::vector<RifEclipseSummaryAddress> catAddresses =
            RiaSummaryCurveAnalyzer::addressesForCategory( addresses, categoryEnum );

        if ( catAddresses.size() > 0 )
        {
            std::cout << caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>::uiText( categoryEnum ).toStdString()
                      << " count : " << catAddresses.size() << std::endl;

            for ( size_t i = 0; i < catAddresses.size(); i++ )
            {
                std::cout << catAddresses[i].quantityName() << " " << catAddresses[i].regionNumber() << " "
                          << catAddresses[i].regionNumber2() << " " << catAddresses[i].wellGroupName() << " "
                          << catAddresses[i].wellName() << " " << catAddresses[i].wellSegmentNumber() << " "
                          << catAddresses[i].lgrName() << " " << catAddresses[i].cellI() << " "
                          << catAddresses[i].cellJ() << " " << catAddresses[i].cellK() << std::endl;
            }

            std::cout << std::endl;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseSummaryTools::findSummaryHeaderFileInfo( const QString& inputFile,
                                                        QString*       headerFile,
                                                        QString*       path,
                                                        QString*       base,
                                                        bool*          isFormatted )
{
    char* myPath        = nullptr;
    char* myBase        = nullptr;
    bool  formattedFile = true;

    util_alloc_file_components( RiaStringEncodingTools::toNativeEncoded( QDir::toNativeSeparators( inputFile ) ).data(),
                                &myPath,
                                &myBase,
                                nullptr );

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
