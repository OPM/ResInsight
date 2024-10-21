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
#include "RifEclipseSummaryAddressDefines.h"

#include <QString>

#include <utility>
#include <vector>

#include <gsl/gsl>

class RimSummaryCase;
class RimSummaryEnsemble;
class RiaSummaryCurveAddress;

//==================================================================================================
///
//==================================================================================================
class RiaSummaryCurveDefinition
{
public:
    RiaSummaryCurveDefinition();
    explicit RiaSummaryCurveDefinition( RimSummaryCase* summaryCaseY, const RifEclipseSummaryAddress& summaryAddressY, bool isEnsembleCurve );
    explicit RiaSummaryCurveDefinition( RimSummaryEnsemble* ensemble, const RifEclipseSummaryAddress& summaryAddressY );
    explicit RiaSummaryCurveDefinition( RimSummaryEnsemble* ensemble, const RiaSummaryCurveAddress& summaryCurveAddress );

    // X and Y Axis
    RimSummaryEnsemble* ensemble() const;
    void                setEnsemble( RimSummaryEnsemble* ensemble );

    // Y Axis
    RimSummaryCase*          summaryCaseY() const;
    RifEclipseSummaryAddress summaryAddressY() const;
    bool                     isEnsembleCurve() const;
    void                     setSummaryAddressY( const RifEclipseSummaryAddress& address );

    // X Axis
    void                     setSummaryCaseX( RimSummaryCase* summaryCase );
    void                     setSummaryAddressX( const RifEclipseSummaryAddress& summaryAddress );
    RimSummaryCase*          summaryCaseX() const;
    RifEclipseSummaryAddress summaryAddressX() const;

    RiaSummaryCurveAddress summaryCurveAddress() const;

    void setIdentifierText( SummaryCategory category, const std::string& name );

    bool operator<( const RiaSummaryCurveDefinition& other ) const;

    // TODO: Consider moving to a separate tools class
    static std::vector<double> resultValues( const RiaSummaryCurveDefinition& curveDefinition );
    static std::vector<time_t> timeSteps( const RiaSummaryCurveDefinition& curveDefinition );

    QString curveDefinitionText() const;

    static QString curveDefinitionText( const QString& caseName, const RifEclipseSummaryAddress& summaryAddress );

private:
    RimSummaryCase*          m_summaryCaseY;
    RifEclipseSummaryAddress m_summaryAddressY;
    RimSummaryCase*          m_summaryCaseX;
    RifEclipseSummaryAddress m_summaryAddressX;
    RimSummaryEnsemble*      m_ensemble;
    bool                     m_isEnsembleCurve;
};

class RiaSummaryCurveDefinitionAnalyser
{
public:
    RiaSummaryCurveDefinitionAnalyser() = default;
    void setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefs );

    std::set<RimSummaryCase*>     m_singleSummaryCases; // All summary cases used
    std::set<RimSummaryEnsemble*> m_ensembles; // All the ensembles referenced by the summary cases

    std::set<RifEclipseSummaryAddress> m_summaryAdresses;
    std::set<std::string>              m_vectorNames;
};
