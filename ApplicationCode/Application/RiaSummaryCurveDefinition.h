/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include <QString>

#include <utility>
#include <vector>

#include <gsl/gsl>

class RimSummaryCase;
class RimSummaryCaseCollection;

//==================================================================================================
///
//==================================================================================================
class RiaSummaryCurveDefinition
{
public:
    RiaSummaryCurveDefinition();
    explicit RiaSummaryCurveDefinition( gsl::not_null<RimSummaryCase*>  summaryCase,
                                        const RifEclipseSummaryAddress& summaryAddress,
                                        bool                            isEnsembleCurve );
    explicit RiaSummaryCurveDefinition( gsl::not_null<RimSummaryCaseCollection*> ensemble,
                                        const RifEclipseSummaryAddress&          summaryAddress );

    RimSummaryCase*                 summaryCase() const;
    const RifEclipseSummaryAddress& summaryAddress() const;
    RimSummaryCaseCollection*       ensemble() const;
    bool                            isEnsembleCurve() const;

    bool operator<( const RiaSummaryCurveDefinition& other ) const;

    // TODO: Consider moving to a separate tools class
    static void resultValues( const RiaSummaryCurveDefinition& curveDefinition, gsl::not_null<std::vector<double>*> values );
    static const std::vector<time_t>& timeSteps( const RiaSummaryCurveDefinition& curveDefinition );

    QString curveDefinitionText() const;

    static QString curveDefinitionText( const QString& caseName, const RifEclipseSummaryAddress& summaryAddress );

private:
    RimSummaryCase*           m_summaryCase;
    RifEclipseSummaryAddress  m_summaryAddress;
    RimSummaryCaseCollection* m_ensemble;
    bool                      m_isEnsembleCurve;
};

class RiaSummaryCurveDefinitionAnalyser
{
public:
    RiaSummaryCurveDefinitionAnalyser() = default;
    void setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefs );

    std::set<RimSummaryCase*>           m_singleSummaryCases; // All summary cases used
    std::set<RimSummaryCaseCollection*> m_ensembles; // All the ensembles referenced by the summary cases

    std::set<RifEclipseSummaryAddress> m_summaryItems; // Quantity name set to "", stores only the identifiers
    std::set<std::string>              m_quantityNames; // Quantity names from the addresses
};
