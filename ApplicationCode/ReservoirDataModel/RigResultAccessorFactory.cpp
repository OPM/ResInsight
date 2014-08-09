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

#include "RigResultAccessorFactory.h"

#include "RigResultAccessor.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"

#include "cvfLibCore.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfAssert.h"

#include "RigMainGrid.h"
#include "RigCaseCellResultsData.h"
#include "RigActiveCellInfo.h"
#include "RigGridBase.h"
#include "RigCaseData.h"
#include <math.h>
#include "RigCombTransResultAccessor.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex,
	RifReaderInterface::PorosityModelResultType porosityModel,
	size_t timeStepIndex,
	const QString& uiResultName)
{
	CVF_ASSERT(gridIndex < eclipseCase->gridCount());
	CVF_ASSERT(eclipseCase);
	CVF_ASSERT(eclipseCase->results(porosityModel));
	CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

	RigGridBase* grid = eclipseCase->grid(gridIndex);

	if (uiResultName == RimDefines::combinedTransmissibilityResultName())
	{
		cvf::ref<RigCombTransResultAccessor> cellFaceAccessObject = new RigCombTransResultAccessor(grid);

		cvf::ref<RigResultAccessor> xTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANX");
		cvf::ref<RigResultAccessor> yTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANY");
		cvf::ref<RigResultAccessor> zTransAccessor = RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, "TRANZ");

		cellFaceAccessObject->setTransResultAccessors(xTransAccessor.p(), yTransAccessor.p(), zTransAccessor.p());

		return cellFaceAccessObject;
	}

	return RigResultAccessorFactory::createNativeResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, uiResultName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex,
	RifReaderInterface::PorosityModelResultType porosityModel,
	size_t timeStepIndex,
	const QString& uiResultName,
	RimDefines::ResultCatType resultType)
{
	CVF_ASSERT(gridIndex < eclipseCase->gridCount());
	CVF_ASSERT(eclipseCase);
	CVF_ASSERT(eclipseCase->results(porosityModel));
	CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

	RigGridBase *grid = eclipseCase->grid(gridIndex);

	if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
	{
		return NULL;
	}

	size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(resultType, uiResultName);
	if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
	{
		return NULL;
	}

	return createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// This function must be harmonized with RigResultModifierFactory::createResultModifier()
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createNativeResultAccessor(RigCaseData* eclipseCase,
    size_t gridIndex,
    RifReaderInterface::PorosityModelResultType porosityModel,
    size_t timeStepIndex,
    const QString& uiResultName)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    RigGridBase *grid = eclipseCase->grid(gridIndex);

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    size_t scalarSetIndex = eclipseCase->results(porosityModel)->findScalarResultIndex(uiResultName);
    if (scalarSetIndex == cvf::UNDEFINED_SIZE_T)
    {
        return NULL;
    }

	return createResultAccessor(eclipseCase, gridIndex, porosityModel, timeStepIndex, scalarSetIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> RigResultAccessorFactory::createResultAccessor(RigCaseData* eclipseCase,
	size_t gridIndex, 
	RifReaderInterface::PorosityModelResultType porosityModel, 
	size_t timeStepIndex, 
	size_t resultIndex)
{
	if (!eclipseCase) return NULL;

	RigGridBase* grid = eclipseCase->grid(gridIndex);
	if (!grid) return NULL;

	std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(resultIndex);
	if (timeStepIndex >= scalarSetResults.size())
	{
		return NULL;
	}

	std::vector<double>* resultValues = NULL;
	if (timeStepIndex < scalarSetResults.size())
	{
		resultValues = &(scalarSetResults[timeStepIndex]);
	}

	bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(resultIndex);
	if (useGlobalActiveIndex)
	{
		cvf::ref<RigResultAccessor> object = new RigActiveCellsResultAccessor(grid, resultValues, eclipseCase->activeCellInfo(porosityModel));
		return object;
	}
	else
	{
		cvf::ref<RigResultAccessor> object = new RigAllGridCellsResultAccessor(grid, resultValues);
		return object;
	}
}




//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Rest of this file is to be deleted
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridAllCellsScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridAllCellsScalarDataAccess(const RigGridBase* grid, std::vector<double>* reservoirResultValues);

    virtual double          cellScalar(size_t gridLocalCellIndex) const;
    virtual void            setCellScalar(size_t cellIndex, double value);

private:
    const RigGridBase*      m_grid;
    std::vector<double>*    m_reservoirResultValues;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGridAllCellsScalarDataAccess::RigGridAllCellsScalarDataAccess(const RigGridBase* grid, std::vector<double>* reservoirResultValues) 
    : m_grid(grid),
    m_reservoirResultValues(reservoirResultValues)
{
    CVF_ASSERT(reservoirResultValues != NULL);
    CVF_ASSERT(grid != NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RigGridAllCellsScalarDataAccess::cellScalar(size_t gridLocalCellIndex) const
{
    if (m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

    size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
    CVF_TIGHT_ASSERT(reservoirCellIndex < m_reservoirResultValues->size());

    return m_reservoirResultValues->at(reservoirCellIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGridAllCellsScalarDataAccess::setCellScalar(size_t gridLocalCellIndex, double scalarValue)
{
    size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
    CVF_TIGHT_ASSERT(reservoirCellIndex < m_reservoirResultValues->size());

    (*m_reservoirResultValues)[reservoirCellIndex] = scalarValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RigGridActiveCellsScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridActiveCellsScalarDataAccess(const RigGridBase* grid, std::vector<double>* reservoirResultValues, const RigActiveCellInfo* activeCellInfo)
      : m_grid(grid),
        m_reservoirResultValues(reservoirResultValues),
        m_activeCellInfo(activeCellInfo)
      {

          CVF_ASSERT(grid != NULL);
      }

      virtual double  cellScalar(size_t gridLocalCellIndex) const
      {
          if (m_reservoirResultValues == NULL || m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

          size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
          size_t resultValueIndex = m_activeCellInfo->cellResultIndex(reservoirCellIndex);
          if (resultValueIndex == cvf::UNDEFINED_SIZE_T) return HUGE_VAL;

          CVF_TIGHT_ASSERT(resultValueIndex < m_reservoirResultValues->size());

          return m_reservoirResultValues->at(resultValueIndex);
      }

      //--------------------------------------------------------------------------------------------------
      /// 
      //--------------------------------------------------------------------------------------------------
      virtual void setCellScalar(size_t gridLocalCellIndex, double scalarValue)
      {
          size_t reservoirCellIndex = m_grid->reservoirCellIndex(gridLocalCellIndex);
          size_t resultValueIndex = m_activeCellInfo->cellResultIndex(reservoirCellIndex);

          CVF_TIGHT_ASSERT(m_reservoirResultValues != NULL && resultValueIndex < m_reservoirResultValues->size());

          (*m_reservoirResultValues)[resultValueIndex] = scalarValue;
      }

private:
    const RigActiveCellInfo*    m_activeCellInfo;
    const RigGridBase*          m_grid;
    std::vector<double>*        m_reservoirResultValues;
};



class StructGridScalarDataAccessHugeVal : public cvf::StructGridScalarDataAccess
{
public:
    virtual double cellScalar(size_t cellIndex) const
    {
        return HUGE_VAL;
    }
    virtual void   setCellScalar(size_t cellIndex, double value)
    {
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::StructGridScalarDataAccess> RigResultAccessorFactory::TO_BE_DELETED_createNativeDataAccessObject(RigCaseData* eclipseCase,
                                                                                                        size_t gridIndex,
                                                                                                        RifReaderInterface::PorosityModelResultType porosityModel,
                                                                                                        size_t timeStepIndex,
                                                                                                        size_t scalarSetIndex)
{
    CVF_ASSERT(gridIndex < eclipseCase->gridCount());
    CVF_ASSERT(eclipseCase);
    CVF_ASSERT(eclipseCase->results(porosityModel));
    CVF_ASSERT(eclipseCase->activeCellInfo(porosityModel));

    RigGridBase *grid = eclipseCase->grid(gridIndex);

    if (!eclipseCase || !eclipseCase->results(porosityModel) || !eclipseCase->activeCellInfo(porosityModel))
    {
        return NULL;
    }

    std::vector< std::vector<double> >& scalarSetResults = eclipseCase->results(porosityModel)->cellScalarResults(scalarSetIndex);

    // A generated result with a generated results for a subset of time steps, will end up with a result container with less entries than time steps
    // See RiaSetGridProperty command in RiaPropertyDataCommands
    //
    // Some functions requires a valid data access object to be present, these might be rewritten to avoid this dummy object always returning HUGE_VAL
    if (timeStepIndex >= scalarSetResults.size())
    {
         cvf::ref<cvf::StructGridScalarDataAccess> object = new StructGridScalarDataAccessHugeVal;

         return object;
    }

    std::vector<double>* resultValues = NULL;
    if (timeStepIndex < scalarSetResults.size())
    {
        resultValues = &(scalarSetResults[timeStepIndex]);
    }

    bool useGlobalActiveIndex = eclipseCase->results(porosityModel)->isUsingGlobalActiveIndex(scalarSetIndex);
    if (useGlobalActiveIndex)
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridActiveCellsScalarDataAccess(grid, resultValues, eclipseCase->activeCellInfo(porosityModel));
        return object;
    }
    else
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridAllCellsScalarDataAccess(grid, resultValues);
        return object;
    }
}
