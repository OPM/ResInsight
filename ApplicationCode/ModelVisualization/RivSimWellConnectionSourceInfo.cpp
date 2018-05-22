/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RivSimWellConnectionSourceInfo.h"

#include "RimSimWellInView.h"
#include "RivWellConnectionFactorGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSimWellConnectionSourceInfo::RivSimWellConnectionSourceInfo(RimSimWellInView*                         simWellInView,
                                                               RivWellConnectionFactorGeometryGenerator* geometryGenerator)
    : m_simWellInView(simWellInView)
    , m_geometryGenerator(geometryGenerator)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RivSimWellConnectionSourceInfo::simWellInView() const
{
    return m_simWellInView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RivSimWellConnectionSourceInfo::globalCellIndexFromTriangleIndex(cvf::uint triangleIndex) const
{
    if (m_geometryGenerator.isNull()) return 0;

    return m_geometryGenerator->globalCellIndexFromTriangleIndex(triangleIndex);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RivSimWellConnectionSourceInfo::connectionFactorFromTriangleIndex(cvf::uint triangleIndex) const
{
    if (m_geometryGenerator.isNull()) return 0.0;

    return m_geometryGenerator->connectionFactor(triangleIndex);
}
