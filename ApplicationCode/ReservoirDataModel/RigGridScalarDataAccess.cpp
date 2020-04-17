/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigResultAccessObjectFactory.h"

#include "cvfLibCore.h"

#include "cvfAssert.h"
#include "cvfObject.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include <math.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigGridAllCellsScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridAllCellsScalarDataAccess( const RigGridBase* grid, std::vector<double>* reservoirResultValues );

    virtual double cellScalar( size_t gridLocalCellIndex ) const;
    virtual void   setCellScalar( size_t cellIndex, double value );

private:
    const RigGridBase*   m_grid;
    std::vector<double>* m_reservoirResultValues;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridAllCellsScalarDataAccess::RigGridAllCellsScalarDataAccess( const RigGridBase*   grid,
                                                                  std::vector<double>* reservoirResultValues )
    : m_grid( grid )
    , m_reservoirResultValues( reservoirResultValues )
{
    CVF_ASSERT( reservoirResultValues != NULL );
    CVF_ASSERT( grid != NULL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGridAllCellsScalarDataAccess::cellScalar( size_t gridLocalCellIndex ) const
{
    if ( m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

    size_t globalGridCellIndex = m_grid->globalGridCellIndex( gridLocalCellIndex );
    CVF_TIGHT_ASSERT( globalGridCellIndex < m_reservoirResultValues->size() );

    return m_reservoirResultValues->at( globalGridCellIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGridAllCellsScalarDataAccess::setCellScalar( size_t gridLocalCellIndex, double scalarValue )
{
    size_t globalGridCellIndex = m_grid->globalGridCellIndex( gridLocalCellIndex );
    CVF_TIGHT_ASSERT( globalGridCellIndex < m_reservoirResultValues->size() );

    ( *m_reservoirResultValues )[globalGridCellIndex] = scalarValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigGridActiveCellsScalarDataAccess : public cvf::StructGridScalarDataAccess
{
public:
    RigGridActiveCellsScalarDataAccess( const RigGridBase*       grid,
                                        std::vector<double>*     reservoirResultValues,
                                        const RigActiveCellInfo* activeCellInfo )
        : m_grid( grid )
        , m_reservoirResultValues( reservoirResultValues )
        , m_activeCellInfo( activeCellInfo )
    {
        CVF_ASSERT( grid != NULL );
    }

    virtual double cellScalar( size_t gridLocalCellIndex ) const
    {
        if ( m_reservoirResultValues == NULL || m_reservoirResultValues->size() == 0 ) return HUGE_VAL;

        size_t globalGridCellIndex = m_grid->globalGridCellIndex( gridLocalCellIndex );
        size_t resultValueIndex    = m_activeCellInfo->cellResultIndex( globalGridCellIndex );
        if ( resultValueIndex == cvf::UNDEFINED_SIZE_T ) return HUGE_VAL;

        CVF_TIGHT_ASSERT( resultValueIndex < m_reservoirResultValues->size() );

        return m_reservoirResultValues->at( resultValueIndex );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    virtual void setCellScalar( size_t gridLocalCellIndex, double scalarValue )
    {
        size_t globalGridCellIndex = m_grid->globalGridCellIndex( gridLocalCellIndex );
        size_t resultValueIndex    = m_activeCellInfo->cellResultIndex( globalGridCellIndex );

        CVF_TIGHT_ASSERT( m_reservoirResultValues != NULL && resultValueIndex < m_reservoirResultValues->size() );

        ( *m_reservoirResultValues )[resultValueIndex] = scalarValue;
    }

private:
    const RigActiveCellInfo* m_activeCellInfo;
    const RigGridBase*       m_grid;
    std::vector<double>*     m_reservoirResultValues;
};

class StructGridScalarDataAccessHugeVal : public cvf::StructGridScalarDataAccess
{
public:
    virtual double cellScalar( size_t cellIndex ) const { return HUGE_VAL; }
    virtual void   setCellScalar( size_t cellIndex, double value ) {}
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::StructGridScalarDataAccess>
    RigResultAccessObjectFactory::createNativeDataAccessObject( RigCaseData*                                eclipseCase,
                                                                size_t                                      gridIndex,
                                                                RifReaderInterface::PorosityModelResultType porosityModel,
                                                                size_t timeStepIndex,
                                                                size_t scalarSetIndex )
{
    CVF_ASSERT( gridIndex < eclipseCase->gridCount() );
    CVF_ASSERT( eclipseCase );
    CVF_ASSERT( eclipseCase->results( porosityModel ) );
    CVF_ASSERT( eclipseCase->activeCellInfo( porosityModel ) );

    RigGridBase* grid = eclipseCase->grid( gridIndex );

    if ( !eclipseCase || !eclipseCase->results( porosityModel ) || !eclipseCase->activeCellInfo( porosityModel ) )
    {
        return NULL;
    }

    std::vector<std::vector<double>>& scalarSetResults =
        eclipseCase->results( porosityModel )->cellScalarResults( scalarSetIndex );

    // A generated result with a generated results for a subset of time steps, will end up with a result container with
    // less entries than time steps See RiaSetGridProperty command in RiaPropertyDataCommands
    //
    // Some functions requires a valid data access object to be present, these might be rewritten to avoid this dummy
    // object always returning HUGE_VAL
    if ( timeStepIndex >= scalarSetResults.size() )
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object = new StructGridScalarDataAccessHugeVal;

        return object;
    }

    std::vector<double>* resultValues = NULL;
    if ( timeStepIndex < scalarSetResults.size() )
    {
        resultValues = &( scalarSetResults[timeStepIndex] );
    }

    bool useGlobalActiveIndex = eclipseCase->results( porosityModel )->isUsingGlobalActiveIndex( scalarSetIndex );
    if ( useGlobalActiveIndex )
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object =
            new RigGridActiveCellsScalarDataAccess( grid, resultValues, eclipseCase->activeCellInfo( porosityModel ) );
        return object;
    }
    else
    {
        cvf::ref<cvf::StructGridScalarDataAccess> object = new RigGridAllCellsScalarDataAccess( grid, resultValues );
        return object;
    }
}
