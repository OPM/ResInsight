/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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
#include "RifSummaryReaderInterface.h"

class RimDerivedEnsembleCase;


//==================================================================================================
///  
//==================================================================================================
class RifDerivedEnsembleReader : public RifSummaryReaderInterface
{
public:
    RifDerivedEnsembleReader(RimDerivedEnsembleCase* derivedCase);

    virtual const std::vector<time_t>&  timeSteps(const RifEclipseSummaryAddress& resultAddress) const override;
    virtual bool                        values(const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values) const override;
    virtual std::string                 unitName(const RifEclipseSummaryAddress& resultAddress) const override;

private:
    RimDerivedEnsembleCase* m_derivedCase;
};
