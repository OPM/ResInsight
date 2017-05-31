/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cvfObject.h"

namespace cvf
{
    class DrawableGeo;
    class StructGridQuadToCellFaceMapper;
}

class RimCellEdgeColors;
class RimEclipseCellColors;
class RigGridBase;
class RigResultAccessor;
class RigEclipseCaseData;


class RivCellEdgeGeometryUtils
{
public:
    static void addCellEdgeResultsToDrawableGeo(size_t timeStepIndex,
        RimEclipseCellColors* cellResultColors,
        RimCellEdgeColors* cellEdgeResultColors,
        const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
        cvf::DrawableGeo* geo,
        size_t gridIndex,
        bool useDefaultValueForHugeVals,
        float opacityLevel);

    static void addTernaryCellEdgeResultsToDrawableGeo(size_t timeStepIndex,
        RimEclipseCellColors* cellResultColors,
        RimCellEdgeColors* cellEdgeResultColors,
        const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
        cvf::DrawableGeo* geo,
        size_t gridIndex,
        float opacityLevel);


private:
    static cvf::ref<RigResultAccessor> createCellCenterResultAccessor(
        RimEclipseCellColors* cellResultColors,
        size_t timeStepIndex,
        RigEclipseCaseData* eclipseCase,
        const RigGridBase* grid);

    static cvf::ref<RigResultAccessor> createCellEdgeResultAccessor(
        RimEclipseCellColors* cellResultColors,
        RimCellEdgeColors* cellEdgeResultColors,
        size_t timeStepIndex,
        RigEclipseCaseData* eclipseCase,
        const RigGridBase* grid);

    static bool hideScalarValue(double scalarValue, double scalarValueToHide, double tolerance);
};
