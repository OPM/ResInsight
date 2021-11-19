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

#include <set>

class RimSummaryCase;
class RifEclipseSummaryAddress;

class QString;
class QStringList;

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

    static bool hasFilterAnyMatch( const QString& curveFilter, const std::set<RifEclipseSummaryAddress>& summaryAddresses );

    static void splitIntoAddressAndDataSourceFilters( const QStringList& filters,
                                                      const QStringList& dataSourceNames,
                                                      QStringList&       addressFilters,
                                                      QStringList&       dataSourceFilters );
};
