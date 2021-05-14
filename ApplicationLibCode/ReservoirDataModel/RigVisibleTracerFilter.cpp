/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RigVisibleTracerFilter.h"

#include "RigFlowDiagResults.h"
#include "RigFlowDiagVisibleCellsStatCalc.h"

#include "RimEclipseView.h"

#include "cvfArray.h"

#include <cmath>
#include <cstddef>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVisibleTracerFilter::filterByVisibility( RimEclipseView&                 eclView,
                                                 RigFlowDiagResults&             flowDiagResults,
                                                 const RigFlowDiagResultAddress& resVarAddr,
                                                 size_t                          timeStepIndex,
                                                 std::set<int>&                  visibleTracers )
{
    cvf::ref<cvf::UByteArray> cellVisibilities = eclView.currentTotalCellVisibility();

    cvf::ref<RigFlowDiagVisibleCellsStatCalc> calculator =
        cvf::make_ref<RigFlowDiagVisibleCellsStatCalc>( &flowDiagResults, resVarAddr, cellVisibilities.p() );
    calculator->uniqueValues( timeStepIndex, visibleTracers );
}
