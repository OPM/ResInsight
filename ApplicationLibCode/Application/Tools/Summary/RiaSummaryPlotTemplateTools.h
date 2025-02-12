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
class RimSummaryEnsemble;
class RifEclipseSummaryAddress;
class RimSummaryMultiPlot;
class RimSummaryAddressCollection;
class RimPlotTemplateFileItem;

//==================================================================================================
///
//==================================================================================================
class RicSummaryPlotTemplateTools
{
public:
    static RimSummaryMultiPlot* create( const QString& fileName );

    static RimSummaryMultiPlot*
        create( const QString& fileName, const std::vector<RimSummaryCase*>& cases, const std::vector<RimSummaryEnsemble*>& ensembles );

    static QString              selectPlotTemplatePath();
    static std::vector<QString> selectDefaultPlotTemplates( std::vector<QString> currentSelection );

    static QString summaryCaseFieldKeyword();
    static QString summaryCaseXFieldKeyword();
    static QString summaryGroupFieldKeyword();

    static QString placeholderTextForSummaryCase();
    static QString placeholderTextForSummaryCaseX();
    static QString placeholderTextForSummaryGroup();
    static QString placeholderTextForWell();
    static QString placeholderTextForGroup();

private:
    static RimSummaryMultiPlot* createMultiPlotFromTemplateFile( const QString& fileName );

    static std::vector<RimSummaryCase*>              selectedSummaryCases();
    static std::vector<RimSummaryEnsemble*>          selectedSummaryEnsembles();
    static std::vector<RimSummaryAddressCollection*> selectedSummaryAddressCollections();

    static QString htmlTextFromPlotAndSelection( const RimSummaryPlot* templatePlot,

                                                 const std::set<RifEclipseSummaryAddress>& selectedSummaryAddresses,
                                                 const std::vector<caf::PdmObject*>&       selectedSources );

    static QString htmlTextFromCount( const QString& itemText, size_t requiredItemCount, size_t selectionCount );

    static void setValuesForPlaceholders( RimSummaryMultiPlot*                    summaryMultiPlot,
                                          const std::vector<RimSummaryCase*>&     selectedSummaryCases,
                                          const std::vector<RimSummaryEnsemble*>& selectedEnsembles,
                                          const std::vector<QString>&             wellNames,
                                          const std::vector<QString>&             groupNames,
                                          const std::vector<QString>&             regions );

    static void setValuesForPlaceholders( RimSummaryPlot*                         summaryPlot,
                                          const std::vector<RimSummaryCase*>&     selectedSummaryCases,
                                          const std::vector<RimSummaryEnsemble*>& selectedEnsembles,
                                          const std::vector<QString>&             wellNames,
                                          const std::vector<QString>&             groupNames,
                                          const std::vector<QString>&             regions );

    static RifEclipseSummaryAddress firstAddressByQuantity( const RifEclipseSummaryAddress&           sourceAddress,
                                                            const std::set<RifEclipseSummaryAddress>& allAddresses );

    static int findValueForKeyword( const QString& keyword, const QString& valueString, bool* ok );

    static void setPlaceholderWellName( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& wellNames );
    static void setPlaceholderGroupName( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& groupNames );
    static void setPlaceholderRegion( RifEclipseSummaryAddress* summaryAddress, const std::vector<QString>& regions );
};
