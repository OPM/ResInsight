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

#include "RimSummaryPlotNameHelperInterface.h"

//==================================================================================================
//
//==================================================================================================
class RimMultiSummaryPlotNameHelper : public RimSummaryPlotNameHelperInterface
{
public:
    explicit RimMultiSummaryPlotNameHelper( std::vector<const RimSummaryPlotNameHelperInterface*> nameHelpers );

    QString plotTitle() const override;

    bool isPlotDisplayingSingleQuantity() const override;
    bool isWellNameInTitle() const override;
    bool isWellGroupNameInTitle() const override;
    bool isRegionInTitle() const override;
    bool isCaseInTitle() const override;
    bool isBlockInTitle() const override;
    bool isSegmentInTitle() const override;
    bool isCompletionInTitle() const override;

    QString caseName() const override;

    std::string titleQuantity() const override;
    std::string titleWellName() const override;
    std::string titleWellGroupName() const override;
    std::string titleRegion() const override;
    std::string titleBlock() const override;
    std::string titleSegment() const override;
    std::string titleCompletion() const override;

private:
    std::vector<const RimSummaryPlotNameHelperInterface*> m_nameHelpers;
};
