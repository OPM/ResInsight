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

#pragma once

#include <QStringList>

#include <set>
#include <vector>

class RimSummaryCase;
class RifEclipseSummaryAddress;
class RimSummaryCaseCollection;

class QString;

//==================================================================================================
//
//==================================================================================================
class RiaSummaryStringTools
{
public:
    static void splitAddressFiltersInGridAndSummary( RimSummaryCase*    summaryCase,
                                                     const QStringList& addressFilters,
                                                     QStringList*       summaryAddressFilters,
                                                     QStringList*       gridResultAddressFilters );

    static std::pair<QStringList, QStringList> splitIntoAddressAndDataSourceFilters( const QString& filter );

    static std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>> allDataSourcesInProject();
    static std::pair<std::vector<RimSummaryCase*>, std::vector<RimSummaryCaseCollection*>>
        dataSourcesMatchingFilters( const QStringList& dataSourceFilters );

    static QStringList splitIntoWords( const QString& text );

    static QStringList dataSourceNames( const std::vector<RimSummaryCase*>&           summaryCases,
                                        const std::vector<RimSummaryCaseCollection*>& ensembles );

    static std::set<RifEclipseSummaryAddress> computeFilteredAddresses( const QStringList&                        textFilters,
                                                                        const std::set<RifEclipseSummaryAddress>& sourceAddresses,
                                                                        bool                                      includeDiffCurves );

    // Consider private, set public to be able to test
    static void splitUsingDataSourceNames( const QStringList& filters,
                                           const QStringList& dataSourceNames,
                                           QStringList&       addressFilters,
                                           QStringList&       dataSourceFilters );

private:
    static bool hasFilterAnyMatch( const QString& curveFilter, const std::set<RifEclipseSummaryAddress>& summaryAddresses );
};
