/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimSummaryCase.h"
#include "RifSummaryReaderInterface.h"

#include <memory>

class RifCalculatedSummaryCurveReader;
class RimSummaryCalculation;
class RimSummaryCalculationCollection;


//==================================================================================================
//
//==================================================================================================
class RifCalculatedSummaryCurveReader : public RifSummaryReaderInterface
{
public:
    explicit RifCalculatedSummaryCurveReader(RimSummaryCalculationCollection* calculationCollection);

    const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;
    bool                        values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const override;
    std::string                 unitName(const RifEclipseSummaryAddress& resultAddress) const override;

    void                                buildMetaData();

    RimSummaryCalculation*              findCalculationByName(const RifEclipseSummaryAddress& resultAddress) const;


    RiaEclipseUnitTools::UnitSystem unitSystem() const override;

private:
    caf::PdmPointer<RimSummaryCalculationCollection> m_calculationCollection;
};
