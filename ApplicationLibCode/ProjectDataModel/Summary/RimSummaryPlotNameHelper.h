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

#include "RimSummaryNameHelper.h"

#include "cafPdmPointer.h"

#include <QString>

#include <memory>
#include <string>
#include <vector>

class RimSummaryCurve;
class RimSummaryCase;
class RimSummaryEnsemble;
class RiaSummaryAddressAnalyzer;
class RifEclipseSummaryAddress;
class RiaSummaryCurveAddress;

//==================================================================================================
//
//==================================================================================================
class RimSummaryPlotNameHelper : public RimSummaryNameHelper
{
public:
    RimSummaryPlotNameHelper();

    void clear();

    void appendAddresses( const std::vector<RiaSummaryCurveAddress>& addresses );
    void setSummaryCases( const std::vector<RimSummaryCase*>& summaryCases );
    void setEnsembleCases( const std::vector<RimSummaryEnsemble*>& ensembleCases );

    QString plotTitle() const override;

    bool isPlotDisplayingSingleCurve() const override;
    bool isWellNameInTitle() const override;
    bool isGroupNameInTitle() const override;
    bool isNetworkInTitle() const override;
    bool isRegionInTitle() const override;
    bool isCaseInTitle() const override;
    bool isBlockInTitle() const override;
    bool isSegmentInTitle() const override;
    bool isCompletionInTitle() const override;

    std::vector<std::string> vectorNames() const override;
    QString                  caseName() const override;

    std::string titleVectorName() const override;
    std::string titleWellName() const override;
    std::string titleGroupName() const override;
    std::string titleNetwork() const override;
    std::string titleRegion() const override;
    std::string titleBlock() const override;
    std::string titleSegment() const override;
    std::string titleCompletion() const override;

    size_t numberOfCases() const override;

private:
    void clearTitleSubStrings();
    void extractPlotTitleSubStrings();

    std::set<RimSummaryCase*>     setOfSummaryCases() const;
    std::set<RimSummaryEnsemble*> setOfEnsembleCases() const;

private:
    std::unique_ptr<RiaSummaryAddressAnalyzer> m_analyzer;

    std::vector<caf::PdmPointer<RimSummaryCase>>     m_summaryCases;
    std::vector<caf::PdmPointer<RimSummaryEnsemble>> m_ensembleCases;

    std::string m_titleQuantity;
    std::string m_titleWellName;
    std::string m_titleGroupName;
    std::string m_titleNetwork;
    std::string m_titleRegion;
    std::string m_titleBlock;
    std::string m_titleSegment;
    std::string m_titleCompletion;

    QString m_titleCaseName;
};
