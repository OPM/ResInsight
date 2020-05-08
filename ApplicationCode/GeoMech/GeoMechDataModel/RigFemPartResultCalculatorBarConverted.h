/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigFemPartResultCalculator.h"

#include <string>

class RigFemPartResultsCollection;
class RigFemScalarResultFrames;
class RigFemResultAddress;

//==================================================================================================
///
//==================================================================================================
class RigFemPartResultCalculatorBarConverted : public RigFemPartResultCalculator
{
public:
    RigFemPartResultCalculatorBarConverted( RigFemPartResultsCollection& collection,
                                            const std::string&           fieldName,
                                            const std::string&           fieldNameToConvert );
    virtual ~RigFemPartResultCalculatorBarConverted();
    bool                      isMatching( const RigFemResultAddress& resVarAddr ) const override;
    RigFemScalarResultFrames* calculate( int partIndex, const RigFemResultAddress& resVarAddr ) override;

protected:
    std::string m_fieldName;
    std::string m_fieldNameToConvert;
};
