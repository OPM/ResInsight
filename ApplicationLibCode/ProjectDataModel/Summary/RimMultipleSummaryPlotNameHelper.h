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

//==================================================================================================
//
//==================================================================================================
class RimMultiSummaryPlotNameHelper : public RimSummaryNameHelper
{
public:
    explicit RimMultiSummaryPlotNameHelper( std::vector<const RimSummaryNameHelper*> nameHelpers );

    QString plotTitle() const override;

    bool isPlotDisplayingSingleCurve() const override;
    bool isWellNameInTitle() const override;
    bool isGroupNameInTitle() const override;
    bool isNetworkInTitle() const override;
    bool isRegionInTitle() const override;
    bool isCaseInTitle() const override;
    bool isBlockInTitle() const override;
    bool isSegmentInTitle() const override;
    bool isWellCompletionInTitle() const override;
    bool isConnectionInTitle() const override;

    std::vector<std::string> vectorNames() const override;
    QString                  caseName() const override;

    std::string titleVectorName() const override;
    std::string titleWellName() const override;
    std::string titleGroupName() const override;
    std::string titleNetwork() const override;
    std::string titleRegion() const override;
    std::string titleBlock() const override;
    std::string titleSegment() const override;
    std::string titleWellCompletion() const override;
    std::string titleConnection() const override;

    size_t numberOfCases() const override;

private:
    std::vector<const RimSummaryNameHelper*> m_nameHelpers;
};
