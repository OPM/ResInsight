////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include <QString>

#include <set>
#include <vector>

namespace caf
{
class PdmObject;
}

class RimSummaryPlot;
class RimSummaryPlot;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RicSummaryPlotTemplateTools
{
public:
    static RimSummaryPlot* createPlotFromTemplateFile( const QString& fileName );
    static QString         htmlTextFromPlotAndSelection( const RimSummaryPlot*                     templatePlot,
                                                         const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                         const std::vector<caf::PdmObject*>&       selectedSources );

    static QString htmlTextFromCount( const QString& itemText, size_t requiredItemCount, size_t selectionCount );
};
