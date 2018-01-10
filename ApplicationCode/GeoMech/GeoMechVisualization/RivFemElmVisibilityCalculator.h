/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfArray.h"

#include "RimCellFilter.h"

namespace cvf
{
    class CellRangeFilter;
}

class RigFemPart;
class RimGeoMechPropertyFilterCollection;
class RimViewController;

class RivFemElmVisibilityCalculator
{
public:
    static void computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart );
    static void computeRangeVisibility(cvf::UByteArray* elmVisibilities, RigFemPart* femPart,  
                                        const cvf::CellRangeFilter& rangeFilter);

    static void computePropertyVisibility(cvf::UByteArray* cellVisibility, 
                                          const RigFemPart* grid,
                                          int timeStepIndex,
                                          const cvf::UByteArray* rangeFilterVisibility,
                                          RimGeoMechPropertyFilterCollection* propFilterColl);

    static void evaluateAndSetCellVisibiliy(double scalarValue, double lowerBound, double upperBound,
                                            const RimCellFilter::FilterModeType filterType, 
                                            cvf::UByteArray* cellVisibility, int cellIndex);


    static void computeOverriddenCellVisibility(cvf::UByteArray* elmVisibilities, 
                                                const RigFemPart* femPart , 
                                                RimViewController* masterViewLink);

};


