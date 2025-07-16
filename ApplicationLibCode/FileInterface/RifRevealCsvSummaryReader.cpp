/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RifRevealCsvSummaryReader.h"

#include "RifCsvUserDataParser.h"
#include "RifRevealCsvSectionSummaryReader.h"

#include "RiaTextStringTools.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSummaryReader::RifRevealCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSummaryReader::~RifRevealCsvSummaryReader() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, QString> RifRevealCsvSummaryReader::parse( const QString& fileName, QString* errorText )
{
    QFile file( fileName );

    if ( !file.open( QFile::ReadOnly | QFile::Text ) ) return std::make_pair( false, "" );

    QTextStream in( &file );

    // Skip first line
    QString caseName = in.readLine().trimmed();

    // Split files on strange header line (starts with ",Date").
    QString     fileContents = in.readAll();
    QStringList parts        = RiaTextStringTools::splitSkipEmptyParts( fileContents, ",Date" );

    // Parse each section separately
    bool isFirst = true;
    for ( auto p : parts )
    {
        p.prepend( "Name,Date" );
        auto sectionReader = std::make_unique<RifRevealCsvSectionSummaryReader>();

        // The first part is field data, and the rest is well data
        auto defaultCategory = isFirst ? RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_FIELD
                                       : RifEclipseSummaryAddressDefines::SummaryCategory::SUMMARY_WELL;

        QString errorMessage;
        if ( !sectionReader->parse( p, defaultCategory, &errorMessage ) )
        {
            return std::make_pair( false, "" );
        }

        addReader( std::move( sectionReader ) );

        isFirst = false;
    }

    // Build metadata to populate the result addresses based on the readers
    buildMetaData();

    return std::make_pair( true, caseName );
}
