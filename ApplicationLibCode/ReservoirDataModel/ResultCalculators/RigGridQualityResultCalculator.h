/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include <cstddef>

#include "RigEclipseResultCalculator.h"

class RigCell;

//==================================================================================================
///
//==================================================================================================
class RigGridQualityResultCalculator : public RigEclipseResultCalculator
{
public:
    RigGridQualityResultCalculator( RigCaseCellResultsData& resultsData );
    ~RigGridQualityResultCalculator() override;
    bool isMatching( const RigEclipseResultAddress& resVarAddr ) const override;
    void calculate( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) override;

    static void calculateResults( RigEclipseCaseData* data );

private:
    static int faulted( const RigCell& cell );
    static int negativeThickness( const RigCell& cell );

    static RigEclipseResultAddress createDoubleAddress( QString resultName );
    static RigEclipseResultAddress createIntegerAddress( QString resultName );

    // map from cell face corner index to neighbor cell face corner index, ref StructGridInterface::cellFaceVertexIndices
    static const std::vector<size_t> m_cornerMap;
};
