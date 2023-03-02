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

#include <QFile>
#include <QTextStream>
#include <Qt>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSummaryReader::RifRevealCsvSummaryReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRevealCsvSummaryReader::~RifRevealCsvSummaryReader()
{
}

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
    QStringList parts        = fileContents.split( ",Date", QString::SkipEmptyParts );

    // Parse each section separately
    bool isFirst = true;
    for ( auto p : parts )
    {
        p.prepend( "Name,Date" );
        cvf::ref<RifRevealCsvSectionSummaryReader> sectionReader = new RifRevealCsvSectionSummaryReader;

        // The first part is field data, and the rest is well data
        auto defaultCategory = isFirst ? RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_FIELD
                                       : RifEclipseSummaryAddress::SummaryVarCategory::SUMMARY_WELL;

        QString errorMessage;
        if ( !sectionReader->parse( p, defaultCategory, &errorMessage ) )
        {
            return std::make_pair( false, "" );
        }

        addReader( sectionReader.p() );

        isFirst = false;
    }

    return std::make_pair( true, caseName );
}
