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
class RimSummaryCase;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RicSummaryPlotTemplateTools
{
public:
    static RimSummaryPlot* createPlotFromTemplateFile( const QString& fileName );
    static void            appendSummaryPlotToPlotCollection( RimSummaryPlot*                               summaryPlot,
                                                              const std::vector<RimSummaryCase*>&           selectedSummaryCases,
                                                              const std::vector<RimSummaryCaseCollection*>& selectedEnsembles );

    static QString htmlTextFromPlotAndSelection( const RimSummaryPlot*                     templatePlot,
                                                 const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                 const std::vector<caf::PdmObject*>&       selectedSources );

    static QString htmlTextFromCount( const QString& itemText, size_t requiredItemCount, size_t selectionCount );

    static QString selectPlotTemplatePath();

    static std::vector<RimSummaryCase*>           selectedSummaryCases();
    static std::vector<RimSummaryCaseCollection*> selectedSummaryCaseCollections();

    static QString summaryCaseFieldKeyword();
    static QString summaryGroupFieldKeyword();
    static QString placeholderTextForSummaryCase();
    static QString placeholderTextForSummaryGroup();

private:
    static RifEclipseSummaryAddress firstAddressByQuantity( const RifEclipseSummaryAddress&           sourceAddress,
                                                            const std::set<RifEclipseSummaryAddress>& allAddresses );

    static int findValueForKeyword( const QString& keyword, const QString& valueString, bool* ok );
};
