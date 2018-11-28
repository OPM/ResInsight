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

#include "RigTernaryResultAccessor.h"

#include "RigResultAccessor.h"

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigTernaryResultAccessor::RigTernaryResultAccessor()
{

}

//--------------------------------------------------------------------------------------------------
/// Requires at least two data objects present, asserts if more than one data accessor is nullptr
//--------------------------------------------------------------------------------------------------
void RigTernaryResultAccessor::setTernaryResultAccessors(RigResultAccessor* soil, RigResultAccessor* sgas, RigResultAccessor* swat)
{
    m_soilAccessor = soil;
    m_sgasAccessor = sgas;
    m_swatAccessor = swat;
}

//--------------------------------------------------------------------------------------------------
/// If only swat is present, soil is set to (1.0 - swat) and sgas to 0
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RigTernaryResultAccessor::cellScalar(size_t gridLocalCellIndex) const
{
    double soil = 0.0;
    double sgas = 0.0;

    if (m_soilAccessor.notNull())
    { 
        soil = m_soilAccessor->cellScalar(gridLocalCellIndex);

        if (m_sgasAccessor.notNull())
        {
            sgas = m_sgasAccessor->cellScalar(gridLocalCellIndex);
        }
        else if (m_swatAccessor.notNull())
        {
            sgas = 1.0 - soil - m_swatAccessor->cellScalar(gridLocalCellIndex);
        }
        else
        {
            sgas = 1.0 - soil;
        }
    }
    else
    {
        if (m_sgasAccessor.notNull())
        {
            sgas = m_sgasAccessor->cellScalar(gridLocalCellIndex);

            if (m_swatAccessor.notNull())
            {
                soil = 1.0 - sgas - m_swatAccessor->cellScalar(gridLocalCellIndex);
            }
            else
            {
                soil = 1.0 - sgas;
            }
        }
        else if (m_swatAccessor.notNull())
        {
            soil = 1.0 - m_swatAccessor->cellScalar(gridLocalCellIndex);
        }
    }

    return cvf::Vec2d(soil, sgas);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RigTernaryResultAccessor::cellFaceScalar(size_t gridLocalCellIndex, cvf::StructGridInterface::FaceType faceId) const
{
    return cellScalar(gridLocalCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2d RigTernaryResultAccessor::cellScalarGlobIdx(size_t globCellIndex) const
{
    double soil = 0.0;
    double sgas = 0.0;

    if (m_soilAccessor.notNull())
    {
        soil = m_soilAccessor->cellScalarGlobIdx(globCellIndex);

        if (soil == HUGE_VAL)
        {
            return cvf::Vec2d(HUGE_VAL, HUGE_VAL);
        }

        if (m_sgasAccessor.notNull())
        {
            sgas = m_sgasAccessor->cellScalarGlobIdx(globCellIndex);
        }
        else if (m_swatAccessor.notNull())
        {
            sgas = 1.0 - soil - m_swatAccessor->cellScalarGlobIdx(globCellIndex);
        }
        else
        {
            sgas = 1.0 - soil;
        }
    }
    else
    {
        if (m_sgasAccessor.notNull())
        {
            sgas = m_sgasAccessor->cellScalarGlobIdx(globCellIndex);

            if (sgas == HUGE_VAL)
            {
                return cvf::Vec2d(HUGE_VAL, HUGE_VAL);
            }

            if (m_swatAccessor.notNull())
            {
                soil = 1.0 - sgas - m_swatAccessor->cellScalarGlobIdx(globCellIndex);
            }
            else
            {
                soil = 1.0 - sgas;
            }
        }
        else if (m_swatAccessor.notNull())
        {
            double swat = m_swatAccessor->cellScalarGlobIdx(globCellIndex);
            if (swat == HUGE_VAL)
            {
                return cvf::Vec2d(HUGE_VAL, HUGE_VAL);
            }

            soil = 1.0 - m_swatAccessor->cellScalarGlobIdx(globCellIndex);
        }
    }

    return cvf::Vec2d(soil, sgas);
}
