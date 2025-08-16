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

#include "Summary/RiaSummaryStringTools.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RifEclipseSummaryAddress.h"
#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RimMainPlotCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"

#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include <QRegularExpression>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryStringTools::splitAddressFiltersInGridAndSummary( RimSummaryCase*    summaryCase,
                                                                 const QStringList& addressFilters,
                                                                 QStringList*       summaryAddressFilters,
                                                                 QStringList*       gridResultAddressFilters )
{
    if ( summaryCase && summaryCase->summaryReader() )
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
std::pair<QStringList, QStringList> RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( const QString& filter )
{
    auto words                     = RiaSummaryStringTools::splitIntoWords( filter );
    auto [summaryCases, ensembles] = RiaSummaryStringTools::allDataSourcesInProject();
    auto dataSourceNames           = RiaSummaryStringTools::dataSourceNames( summaryCases, ensembles );

    QStringList addressFilters;
    QStringList dataSourceFilters;

    RiaSummaryStringTools::splitUsingDataSourceNames( words, dataSourceNames, addressFilters, dataSourceFilters );

    // If no filter on data source is specified, use wildcard to match all
    if ( dataSourceFilters.empty() ) dataSourceFilters.push_back( "*" );

    return { addressFilters, dataSourceFilters };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaSummaryStringTools::hasFilterAnyMatch( const QString& curveFilter, const std::set<RifEclipseSummaryAddress>& summaryAddresses )
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
void RiaSummaryStringTools::splitUsingDataSourceNames( const QStringList& filters,
                                                       const QStringList& dataSourceNames,
                                                       QStringList&       addressFilters,
                                                       QStringList&       dataSourceFilters )
{
    for ( const auto& s : filters )
    {
        // Strip off realization filter from ensemble search string

        QString pureDataSourceCandidate = s.left( s.indexOf( ':' ) );

        bool foundDataSource = false;

        QString            regexPattern = QRegularExpression::wildcardToRegularExpression( pureDataSourceCandidate );
        QRegularExpression searcher( regexPattern, QRegularExpression::CaseInsensitiveOption );

        for ( const auto& ds : dataSourceNames )
        {
            if ( !foundDataSource && searcher.match( ds ).hasMatch() )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryEnsemble*>> RiaSummaryStringTools::allDataSourcesInProject()
{
    auto sumCaseMainColl = RiaSummaryTools::summaryCaseMainCollection();
    if ( !sumCaseMainColl ) return { {}, {} };

    auto summaryCases = sumCaseMainColl->topLevelSummaryCases();
    auto ensembles    = sumCaseMainColl->summaryEnsembles();

    return { summaryCases, ensembles };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryEnsemble*>>
    RiaSummaryStringTools::dataSourcesMatchingFilters( const QStringList& dataSourceFilters )
{
    std::vector<RimSummaryCase*>     matchingSummaryCases;
    std::vector<RimSummaryEnsemble*> matchingEnsembles;

    auto [allSummaryCases, allEnsembles] = allDataSourcesInProject();

    for ( const auto& dsFilter : dataSourceFilters )
    {
        QString            searchString = dsFilter.left( dsFilter.indexOf( ':' ) );
        QString            regexPattern = QRegularExpression::wildcardToRegularExpression( searchString );
        QRegularExpression searcher( regexPattern, QRegularExpression::CaseInsensitiveOption );

        for ( const auto& ensemble : allEnsembles )
        {
            auto ensembleName = ensemble->name();
            if ( searcher.match( ensembleName ).hasMatch() )
            {
                if ( searchString == dsFilter )
                {
                    // Match on ensemble name without realization filter

                    matchingEnsembles.push_back( ensemble );
                }
                else
                {
                    // Match on subset of realisations in ensemble

                    QString            realizationSearchString = dsFilter.right( dsFilter.size() - dsFilter.indexOf( ':' ) - 1 );
                    QString            regexPattern            = QRegularExpression::wildcardToRegularExpression( realizationSearchString );
                    QRegularExpression realizationSearcher( regexPattern, QRegularExpression::CaseInsensitiveOption );

                    for ( const auto& summaryCase : ensemble->allSummaryCases() )
                    {
                        auto realizationName = summaryCase->displayCaseName();
                        if ( realizationSearcher.match( realizationName ).hasMatch() )
                        {
                            matchingSummaryCases.push_back( summaryCase );
                        }
                    }
                }
            }
        }

        for ( const auto& summaryCase : allSummaryCases )
        {
            auto summaryCaseName = summaryCase->displayCaseName();
            if ( searcher.match( summaryCaseName ).hasMatch() )
            {
                matchingSummaryCases.push_back( summaryCase );
            }
        }
    }

    return { matchingSummaryCases, matchingEnsembles };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaSummaryStringTools::splitIntoWords( const QString& text )
{
    return RiaTextStringTools::splitSkipEmptyParts( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaSummaryStringTools::dataSourceNames( const std::vector<RimSummaryCase*>&     summaryCases,
                                                    const std::vector<RimSummaryEnsemble*>& ensembles )
{
    QStringList names;
    for ( const auto& summaryCase : summaryCases )
    {
        names.push_back( summaryCase->displayCaseName() );
    }

    for ( const auto& ensemble : ensembles )
    {
        names.push_back( ensemble->name() );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RiaSummaryStringTools::computeFilteredAddresses( const QStringList& textFilters,
                                                                                    const std::set<RifEclipseSummaryAddress>& sourceAddresses,
                                                                                    bool includeDiffCurves )
{
    std::set<RifEclipseSummaryAddress> addresses;

    std::vector<bool> usedFilters;
    RicSummaryPlotFeatureImpl::insertFilteredAddressesInSet( textFilters, sourceAddresses, &addresses, &usedFilters );

    if ( includeDiffCurves ) return addresses;

    const auto diffText = RifEclipseSummaryAddressDefines::differenceIdentifier();

    std::set<RifEclipseSummaryAddress> addressesWithoutDiffVectors;
    for ( const auto& adr : addresses )
    {
        if ( RiaStdStringTools::endsWith( adr.vectorName(), diffText ) ) continue;

        addressesWithoutDiffVectors.insert( adr );
    }

    return addressesWithoutDiffVectors;
}
