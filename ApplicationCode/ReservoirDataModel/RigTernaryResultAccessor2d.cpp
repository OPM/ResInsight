/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RigTernaryResultAccessor2d.h"

#include "RigResultAccessor.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigTernaryResultAccessor::RigTernaryResultAccessor()
{

}

//--------------------------------------------------------------------------------------------------
/// Requires at least two data objects present, asserts if more than one data accessor is NULL
//--------------------------------------------------------------------------------------------------
void RigTernaryResultAccessor::setTernaryResultAccessors(RigResultAccessor* soil, RigResultAccessor* sgas, RigResultAccessor* swat)
{
	m_soilAccessor = soil;
	m_sgasAccessor = sgas;
	m_swatAccessor = swat;

	int nullPointerCount = 0;
	if (!soil) nullPointerCount++;
	if (!sgas) nullPointerCount++;
	if (!swat) nullPointerCount++;

	CVF_ASSERT(nullPointerCount <= 1);
}

//--------------------------------------------------------------------------------------------------
/// 
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
		else
		{
			sgas = 1.0 - soil - m_swatAccessor->cellScalar(gridLocalCellIndex);
		}
	}
	else
	{
		sgas = m_sgasAccessor->cellScalar(gridLocalCellIndex);

		soil = 1.0 - sgas - m_swatAccessor->cellScalar(gridLocalCellIndex);
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
