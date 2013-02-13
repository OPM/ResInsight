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
#include "cvfObject.h"
#include "cvfAssert.h"

#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"
#include "RigActiveCellInfo.h"
#include "RigGridBase.h"
#include "RigReservoir.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridAllCellsScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridAllCellsScalarDataAccess(const RigGridBase* grid, const std::vector<double>* resultValues);

    virtual double  cellScalar(size_t i, size_t j, size_t k) const;
    virtual double  cellScalar(size_t cellIndex) const;
    virtual void    cellCornerScalars(size_t i, size_t j, size_t k, double scalars[8]) const;
    virtual double  gridPointScalar(size_t i, size_t j, size_t k) const;
    virtual bool    pointScalar(const cvf::Vec3d& p, double* scalarValue) const;

    virtual const cvf::Vec3d* cellVector(size_t i, size_t j, size_t k) const;

protected:
    cvf::cref<RigGridBase>  m_grid;
    const std::vector<double>*    m_resultValues;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGridAllCellsScalarDataAccess::RigGridAllCellsScalarDataAccess(const RigGridBase* grid, const std::vector<double>* resultValues) :
m_grid(grid),
    m_resultValues(resultValues)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridAllCellsScalarDataAccess::cellScalar(size_t i, size_t j, size_t k) const
{
    size_t cellIndex = m_grid->cellIndexFromIJK(i, j, k);

    return cellScalar(cellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridAllCellsScalarDataAccess::cellScalar(size_t cellIndex) const
{
    if (m_resultValues->size() == 0 ) return HUGE_VAL;

    if (m_resultValues->size() <= cellIndex) return HUGE_VAL;

    return m_resultValues->at(cellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridAllCellsScalarDataAccess::gridPointScalar(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridAllCellsScalarDataAccess::cellCornerScalars(size_t i, size_t j, size_t k, double scalars[8]) const
{
    CVF_ASSERT(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGridAllCellsScalarDataAccess::pointScalar(const cvf::Vec3d& p, double* scalarValue) const
{
    CVF_ASSERT(false);
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Vec3d* RigGridAllCellsScalarDataAccess::cellVector(size_t i, size_t j, size_t k) const
{
    CVF_ASSERT(false);
    return new cvf::Vec3d();
}








//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridMatrixActiveCellsScalarDataAccess : public RigGridAllCellsScalarDataAccess
{
public:
    RigGridMatrixActiveCellsScalarDataAccess(const RigGridBase* grid, const std::vector<double>* resultValues, const RigActiveCellInfo* activeCellInfo) : 
      RigGridAllCellsScalarDataAccess(grid, resultValues),
        m_activeCellInfo(activeCellInfo)
      {

      }

    virtual double  cellScalar(size_t cellIndex) const
    {
        if (m_resultValues->size() == 0 ) return HUGE_VAL;

        size_t globalGridCellIndex = m_grid->globalGridCellIndex(cellIndex);
        size_t resultValueIndex = m_activeCellInfo->activeIndexInMatrixModel(globalGridCellIndex);
        if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

        if (m_resultValues->size() <= resultValueIndex) return HUGE_VAL;

        return m_resultValues->at(resultValueIndex);
    }

protected:
    const RigActiveCellInfo*    m_activeCellInfo;
};






//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridFractureActiveCellsScalarDataAccess : public RigGridAllCellsScalarDataAccess
{
public:
    RigGridFractureActiveCellsScalarDataAccess(const RigGridBase* grid, const std::vector<double>* resultValues, const RigActiveCellInfo* activeCellInfo) : 
      RigGridAllCellsScalarDataAccess(grid, resultValues),
          m_activeCellInfo(activeCellInfo)
      {
      }

      virtual double  cellScalar(size_t cellIndex) const
      {
          if (m_resultValues->size() == 0 ) return HUGE_VAL;

          size_t globalGridCellIndex = m_grid->globalGridCellIndex(cellIndex);
          size_t resultValueIndex = m_activeCellInfo->activeIndexInFractureModel(globalGridCellIndex);
          if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

          if (m_resultValues->size() <= resultValueIndex) return HUGE_VAL;

          return m_resultValues->at(resultValueIndex);
      }

protected:
    const RigActiveCellInfo*    m_activeCellInfo;
};





//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::StructGridScalarDataAccess> RigGridScalarDataAccessFactory::createDataAccessObject(const RigGridBase* grid,
    const RigReservoir* reservoir,
    RifReaderInterface::PorosityModelResultType porosityModel,
    size_t timeStepIndex,
    size_t scalarSetIndex)
{
    CVF_ASSERT(grid);
    CVF_ASSERT(reservoir);
    CVF_ASSERT(reservoir->results(porosityModel));
    CVF_ASSERT(reservoir->activeCellInfo());

    if (!grid || !reservoir || !reservoir->results(porosityModel) || !reservoir->activeCellInfo())
    {
        return NULL;
    }

    const std::vector< std::vector<double> > & scalarSetResults = reservoir->results(porosityModel)->cellScalarResults(scalarSetIndex);
    if (timeStepIndex >= scalarSetResults.size())
    {
        return NULL;
    }

    const std::vector<double>* resultValues = &(scalarSetResults[timeStepIndex]);


    bool useGlobalActiveIndex = reservoir->results(porosityModel)->isUsingGlobalActiveIndex(scalarSetIndex);
    if (useGlobalActiveIndex)
    {
        if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
        {
            cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridMatrixActiveCellsScalarDataAccess(grid, resultValues, reservoir->activeCellInfo());
            return object;
        }
        else
        {
            cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridFractureActiveCellsScalarDataAccess(grid, resultValues, reservoir->activeCellInfo());
            return object;
        }
    }
    else
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridAllCellsScalarDataAccess(grid, resultValues);
        return object;
    }

}

