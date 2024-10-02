/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RifReaderRegularGridModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRegularGridModel::RifReaderRegularGridModel()
    : m_reservoir( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRegularGridModel::~RifReaderRegularGridModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderRegularGridModel::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    m_reservoirBuilder.createGridsAndCells( eclipseCase );
    m_reservoir = eclipseCase;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderRegularGridModel::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    CAF_ASSERT( false );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderRegularGridModel::dynamicResult( const QString&                result,
                                               RiaDefines::PorosityModelType matrixOrFracture,
                                               size_t                        stepIndex,
                                               std::vector<double>*          values )
{
    CAF_ASSERT( false );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRegularGridModel::setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate )
{
    m_reservoirBuilder.setWorldCoordinates( minWorldCoordinate, maxWorldCoordinate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRegularGridModel::setGridPointDimensions( const cvf::Vec3st& gridPointDimensions )
{
    m_reservoirBuilder.setIJKCount( gridPointDimensions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderRegularGridModel::addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                                        const cvf::Vec3st& maxCellPosition,
                                                        const cvf::Vec3st& singleCellRefinementFactors )
{
    m_reservoirBuilder.addLocalGridRefinement( minCellPosition, maxCellPosition, singleCellRefinementFactors );
}
