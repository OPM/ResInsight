/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RiaSummaryStringTools.h"

#include "RiaLogging.h"
#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryStringTools::splitAddressFiltersInGridAndSummary( RimSummaryCase*    summaryCase,
                                                                 const QStringList& addressFilters,
                                                                 QStringList*       summaryAddressFilters,
                                                                 QStringList*       gridResultAddressFilters )
{
    if ( summaryCase )
    {
        const std::set<RifEclipseSummaryAddress>& addrs = summaryCase->summaryReader()->allResultAddresses();

        QRegularExpression gridAddressPattern( "^[A-Z]+:[0-9]+,[0-9]+,[0-9]+$" );

        for ( int filterIdx = 0; filterIdx < addressFilters.size(); ++filterIdx )
        {
            const QString& address = addressFilters[filterIdx];
            if ( hasFilterAnyMatch( address, addrs ) )
            {
                summaryAddressFilters->push_back( address );
            }
            else
            {
                if ( gridAddressPattern.match( address ).hasMatch() )
                {
                    gridResultAddressFilters->push_back( address );
                }
                else
                {
                    RiaLogging::warning( "No summary or restart vectors matched \"" + address + "\"" );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryStringTools::hasFilterAnyMatch( const QString&                            curveFilter,
                                               const std::set<RifEclipseSummaryAddress>& summaryAddresses )
{
    for ( const auto& addr : summaryAddresses )
    {
        if ( addr.isUiTextMatchingFilterText( curveFilter ) ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Sort filters into curve and data source filters
//--------------------------------------------------------------------------------------------------
void RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( const QStringList& filters,
                                                                  const QStringList& dataSourceNames,
                                                                  QStringList&       addressFilters,
                                                                  QStringList&       dataSourceFilters )
{
    for ( const auto& s : filters )
    {
        // Strip off realization filter from ensemble search string

        QString pureDataSourceCandidate = s.left( s.indexOf( ':' ) );

        bool foundDataSource = false;

        QRegExp searcher( pureDataSourceCandidate, Qt::CaseInsensitive, QRegExp::WildcardUnix );

        for ( const auto& ds : dataSourceNames )
        {
            if ( !foundDataSource && searcher.exactMatch( ds ) )
            {
                dataSourceFilters.push_back( s );
                foundDataSource = true;
            }
        }

        if ( !foundDataSource )
        {
            addressFilters.push_back( s );
        }
    }
}
