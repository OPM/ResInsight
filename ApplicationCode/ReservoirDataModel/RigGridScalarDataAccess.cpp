//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#include "RigGridScalarDataAccess.h"

#include "cvfLibCore.h"
#include "cvfBase.h"
#include "cvfAssert.h"

#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGridScalarDataAccess::RigGridScalarDataAccess(const RigGridBase* grid, bool useGlobalActiveIndex, std::vector<double>* resultValues) :
    m_grid(grid),
    m_useGlobalActiveIndex(useGlobalActiveIndex),
    m_resultValues(resultValues)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigGridScalarDataAccess> RigGridScalarDataAccess::createDataAccessObject(const RigGridBase* grid, RifReaderInterface::PorosityModelResultType porosityModel, size_t timeStepIndex, size_t scalarSetIndex)
{
    CVF_ASSERT(grid);
    CVF_ASSERT(grid->mainGrid());
    CVF_ASSERT(grid->mainGrid()->results(porosityModel));

    if (!grid || !grid->mainGrid() || !grid->mainGrid()->results(porosityModel))
    {
        return NULL;
    }

    bool useGlobalActiveIndex = grid->mainGrid()->results(porosityModel)->isUsingGlobalActiveIndex(scalarSetIndex);

    std::vector< std::vector<double> > & scalarSetResults = grid->mainGrid()->results(porosityModel)->cellScalarResults(scalarSetIndex);
    if (timeStepIndex >= scalarSetResults.size())
    {
        return NULL;
    }

    std::vector<double>* resultValues = &(scalarSetResults[timeStepIndex]);

    cvf::ref<RigGridScalarDataAccess> object = new RigGridScalarDataAccess(grid, useGlobalActiveIndex, resultValues);
    return object;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridScalarDataAccess::cellScalar(size_t i, size_t j, size_t k) const
{
    size_t cellIndex = m_grid->cellIndexFromIJK(i, j, k);

    return cellScalar(cellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridScalarDataAccess::cellScalar(size_t cellIndex) const
{
    if (m_resultValues->size() == 0 ) return HUGE_VAL;

    size_t resultValueIndex = cellIndex;

    if (m_useGlobalActiveIndex)
    {
        resultValueIndex = m_grid->cell(cellIndex).activeIndexInMatrixModel();
        if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;
    }

    if (m_resultValues->size() <= resultValueIndex) return HUGE_VAL;

    return m_resultValues->at(resultValueIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridScalarDataAccess::gridPointScalar(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridScalarDataAccess::cellCornerScalars(size_t i, size_t j, size_t k, double scalars[8]) const
{
    CVF_ASSERT(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridScalarDataAccess::pointScalar(const cvf::Vec3d& p, double* scalarValue) const
{
    CVF_ASSERT(false);
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d* RigGridScalarDataAccess::cellVector(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return new cvf::Vec3d();
}

