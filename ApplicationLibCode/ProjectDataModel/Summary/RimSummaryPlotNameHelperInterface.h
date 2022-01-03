/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include <vector>

class QString;

class RimSummaryCase;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//==================================================================================================
class RimSummaryPlotNameHelperInterface
{
public:
    virtual void clear() = 0;

    virtual void appendAddresses( const std::vector<RifEclipseSummaryAddress>& addresses )       = 0;
    virtual void setSummaryCases( const std::vector<RimSummaryCase*>& summaryCases )             = 0;
    virtual void setEnsembleCases( const std::vector<RimSummaryCaseCollection*>& ensembleCases ) = 0;

    virtual QString plotTitle() const = 0;
    // QString plotTitle( const RimSummaryPlotNameHelper& summaryMultiPlotNameHelper ) const;

    virtual bool isPlotDisplayingSingleQuantity() const = 0;
    virtual bool isWellNameInTitle() const              = 0;
    virtual bool isWellGroupNameInTitle() const         = 0;
    virtual bool isRegionInTitle() const                = 0;
    virtual bool isCaseInTitle() const                  = 0;
    virtual bool isBlockInTitle() const                 = 0;
    virtual bool isSegmentInTitle() const               = 0;
    virtual bool isCompletionInTitle() const            = 0;
};
