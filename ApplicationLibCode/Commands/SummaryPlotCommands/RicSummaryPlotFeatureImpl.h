/////////////////////////////////////////////////////////////////////////////////
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
#include <QStringList>

#include <set>
#include <vector>

class RifEclipseSummaryAddress;

namespace caf
{
class PdmObject;
}

class RimSummaryCurve;
class RimSummaryPlot;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimEnsembleCurveSet;

class RicSummaryPlotFeatureImpl
{
public:
    enum class EnsembleColoringType
    {
        SINGLE_COLOR,
        PARAMETER,
        LOG_PARAMETER,
        NONE
    };

    static std::vector<RimSummaryCurve*> addDefaultCurvesToPlot( RimSummaryPlot* plot, RimSummaryCase* summaryCase );

    static void createSummaryPlotsFromArgumentLine( const QStringList& arguments );

    static RimSummaryPlot* createSummaryPlotForEnsemble( const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                         RimSummaryCaseCollection*           ensemble,
                                                         QStringList                         summaryAddressFilters,
                                                         bool                                addHistoryCurves = false,
                                                         EnsembleColoringType ensembleColoringStyle           = EnsembleColoringType::NONE,
                                                         QString              ensembleColoringParameter       = "" );

    static RimSummaryPlot* createSummaryPlotForCases( const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                      QStringList                         summaryAddressFilters,
                                                      bool                                addHistoryCurves = false );

    static std::vector<RimSummaryPlot*>
        createMultipleSummaryPlotsFromAddresses( const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                 RimSummaryCaseCollection*           ensemble,
                                                 QStringList                         summaryAddressFilters,
                                                 bool                                addHistoryCurves          = false,
                                                 EnsembleColoringType                ensembleColoringStyle     = EnsembleColoringType::NONE,
                                                 QString                             ensembleColoringParameter = "" );

    static void insertFilteredAddressesInSet( const QStringList&                        curveFilters,
                                              const std::set<RifEclipseSummaryAddress>& allAddressesInCase,
                                              std::set<RifEclipseSummaryAddress>*       setToInsertFilteredAddressesIn,
                                              std::vector<bool>*                        usedFilters );

    static QString summaryPlotCommandLineHelpText();

private:
    static RimEnsembleCurveSet* createCurveSet( RimSummaryCaseCollection*       ensemble,
                                                const RifEclipseSummaryAddress& addr,
                                                EnsembleColoringType            ensembleColoringStyle,
                                                QString                         ensembleColoringParameter );

    static RimSummaryCurve* createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& address );

    static RimSummaryCurve* createHistoryCurve( const RifEclipseSummaryAddress& addr, RimSummaryCase* summaryCasesToUse );

    static std::vector<RimSummaryCurve*> addCurvesFromAddressFiltersToPlot( const QStringList& curveFilters,
                                                                            RimSummaryPlot*    plot,
                                                                            RimSummaryCase*    summaryCase,
                                                                            bool               addHistoryCurves );

    static std::set<RifEclipseSummaryAddress> applySummaryAddressFiltersToCases( const std::vector<RimSummaryCase*>& summaryCasesToUse,
                                                                                 const QStringList& summaryAddressFilters );
};

#include "RigEclipseResultAddress.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigGridCellResultAddress
{
public:
    RigGridCellResultAddress()
        : gridIndex( -1 )
        , i( -1 )
        , j( -1 )
        , k( -1 )
    {
    }

    RigGridCellResultAddress( size_t gridIndex, size_t i, size_t j, size_t k, const RigEclipseResultAddress& eclipseResultAddress )
        : gridIndex( gridIndex )
        , i( i )
        , j( j )
        , k( k )
        , eclipseResultAddress( eclipseResultAddress )
    {
    }

    static std::vector<RigGridCellResultAddress> createGridCellAddressesFromFilter( const QString& text );
    // Using zero based ijk

    size_t                  gridIndex;
    size_t                  i;
    size_t                  j;
    size_t                  k;
    RigEclipseResultAddress eclipseResultAddress;
};
