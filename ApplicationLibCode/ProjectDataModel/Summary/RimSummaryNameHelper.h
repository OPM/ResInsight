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

#include <set>
#include <string>
#include <vector>

class QString;

class RimSummaryCase;
class RimSummaryCaseCollection;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//==================================================================================================
class RimSummaryNameHelper
{
public:
    virtual QString plotTitle() const = 0;

    QString aggregatedPlotTitle( const RimSummaryNameHelper& summaryMultiPlotNameHelper ) const;

    virtual bool isPlotDisplayingSingleVectorName() const = 0;
    virtual bool isWellNameInTitle() const                = 0;
    virtual bool isGroupNameInTitle() const               = 0;
    virtual bool isRegionInTitle() const                  = 0;
    virtual bool isCaseInTitle() const                    = 0;
    virtual bool isBlockInTitle() const                   = 0;
    virtual bool isSegmentInTitle() const                 = 0;
    virtual bool isCompletionInTitle() const              = 0;

    virtual std::set<std::string> vectorNames() const = 0;
    virtual QString               caseName() const    = 0;

    virtual std::string titleVectorName() const = 0;
    virtual std::string titleWellName() const   = 0;
    virtual std::string titleGroupName() const  = 0;
    virtual std::string titleRegion() const     = 0;
    virtual std::string titleBlock() const      = 0;
    virtual std::string titleSegment() const    = 0;
    virtual std::string titleCompletion() const = 0;
};
