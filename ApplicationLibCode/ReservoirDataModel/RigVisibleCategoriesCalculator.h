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

#pragma once

#include <cstddef>
#include <set>

class RimEclipseView;
class RigFlowDiagResults;
class RigFlowDiagResultAddress;

//==================================================================================================
///
//==================================================================================================
class RigVisibleCategoriesCalculator
{
public:
    static std::set<int> visibleFlowDiagCategories( RimEclipseView&                 eclView,
                                                    RigFlowDiagResults&             flowDiagResults,
                                                    const RigFlowDiagResultAddress& resVarAddr,
                                                    size_t                          timeStepIndex );

    static std::set<size_t> visibleAllanCategories( RimEclipseView* eclView );
    static std::set<int>    visibleCategories( RimEclipseView* eclView );

private:
    static std::set<size_t> visibleNncConnectionIndices( RimEclipseView* eclView );
    static std::set<size_t> visibleFaultAndIntersectionCells( RimEclipseView* eclView );
};
