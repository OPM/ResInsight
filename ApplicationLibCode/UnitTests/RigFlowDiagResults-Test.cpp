/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "gtest/gtest.h"

#include "RigFlowDiagDefines.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResultFrames.h"
#include "RigFlowDiagResults.h"
#include "RigStatisticsDataCache.h"

#include "RimFlowDiagSolution.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
/// Derived results must not dereference a missing eclipse case when the solution is detached
//--------------------------------------------------------------------------------------------------
TEST( RigFlowDiagResults, DerivedResultWithoutEclipseCase )
{
    auto               flowDiagSolution = std::make_unique<RimFlowDiagSolution>();
    RigFlowDiagResults results( flowDiagSolution.get(), 1 );

    const std::vector<QString> derivedResultNames = { RigFlowDiagDefines::tofResultName(),
                                                      RigFlowDiagDefines::cellFractionResultName(),
                                                      RigFlowDiagDefines::communicationResultName(),
                                                      RigFlowDiagDefines::maxFractionTracerResultName() };

    for ( const auto& name : derivedResultNames )
    {
        RigFlowDiagResultAddress address( name.toStdString(), RigFlowDiagResultAddress::PHASE_ALL, std::set<std::string>() );
        EXPECT_EQ( nullptr, results.resultValues( address, 0 ) );
    }
}
