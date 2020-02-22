/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RifReaderMockModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    m_reservoirBuilder.populateReservoir( eclipseCase );

    m_reservoir = eclipseCase;

    RigCaseCellResultsData* cellResults = eclipseCase->results( RiaDefines::MATRIX_MODEL );

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;
    {
        std::vector<QDateTime> dates;
        std::vector<double>    days;
        std::vector<int>       repNumbers;

        for ( int i = 0; i < static_cast<int>( m_reservoirBuilder.timeStepCount() ); i++ )
        {
            dates.push_back( QDateTime( QDate( 2012 + i, 6, 1 ) ) );
            days.push_back( i );
            repNumbers.push_back( i );
        }

        timeStepInfos = RigEclipseTimeStepInfo::createTimeStepInfos( dates, repNumbers, days );
    }

    for ( size_t i = 0; i < m_reservoirBuilder.resultCount(); i++ )
    {
        RigEclipseResultAddress resAddr( RiaDefines::DYNAMIC_NATIVE, QString( "Dynamic_Result_%1" ).arg( i ) );
        cellResults->createResultEntry( resAddr, false );
        cellResults->setTimeStepInfos( resAddr, timeStepInfos );
    }

    if ( m_reservoirBuilder.timeStepCount() == 0 ) return true;

    std::vector<RigEclipseTimeStepInfo> staticResultTimeStepInfos;
    staticResultTimeStepInfos.push_back( timeStepInfos[0] );

    for ( int i = 0; i < static_cast<int>( m_reservoirBuilder.resultCount() ); i++ )
    {
        QString varEnd;
        if ( i == 0 ) varEnd = "X";
        if ( i == 1 ) varEnd = "Y";
        int resIndex = 0;
        if ( i > 1 ) resIndex = i;

        RigEclipseResultAddress resAddr( RiaDefines::STATIC_NATIVE,
                                         QString( "Static_Result_%1%2" ).arg( resIndex ).arg( varEnd ) );

        cellResults->createResultEntry( resAddr, false );
        cellResults->setTimeStepInfos( resAddr, staticResultTimeStepInfos );
    }

#define ADD_INPUT_PROPERTY( Name )                                                                          \
    {                                                                                                       \
        QString                 resultName( Name );                                                         \
        RigEclipseResultAddress resAddr( RiaDefines::INPUT_PROPERTY, resultName );                          \
        cellResults->createResultEntry( resAddr, false );                                                   \
        cellResults->setTimeStepInfos( resAddr, staticResultTimeStepInfos );                                \
        cellResults->modifiableCellScalarResultTimesteps( resAddr )->resize( 1 );                           \
        std::vector<double>& values = cellResults->modifiableCellScalarResultTimesteps( resAddr )->at( 0 ); \
        this->inputProperty( resultName, &values );                                                         \
    }

    ADD_INPUT_PROPERTY( "PORO" );
    ADD_INPUT_PROPERTY( "PERM" );
    ADD_INPUT_PROPERTY( "MULTX" );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::inputProperty( const QString& propertyName, std::vector<double>* values )
{
    return m_reservoirBuilder.inputProperty( m_reservoir, propertyName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::staticResult( const QString&                result,
                                       RiaDefines::PorosityModelType matrixOrFracture,
                                       std::vector<double>*          values )
{
    m_reservoirBuilder.staticResult( m_reservoir, result, values );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderMockModel::dynamicResult( const QString&                result,
                                        RiaDefines::PorosityModelType matrixOrFracture,
                                        size_t                        stepIndex,
                                        std::vector<double>*          values )
{
    m_reservoirBuilder.dynamicResult( m_reservoir, result, stepIndex, values );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderMockModel::RifReaderMockModel()
    : m_reservoir( nullptr )
{
    /*
    m_cellResults.push_back("Dummy results");
    m_cellProperties.push_back("Dummy static result");
    */
}

RifReaderMockModel::~RifReaderMockModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setWorldCoordinates( cvf::Vec3d minWorldCoordinate, cvf::Vec3d maxWorldCoordinate )
{
    m_reservoirBuilder.setWorldCoordinates( minWorldCoordinate, maxWorldCoordinate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setGridPointDimensions( const cvf::Vec3st& gridPointDimensions )
{
    m_reservoirBuilder.setGridPointDimensions( gridPointDimensions );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::setResultInfo( size_t resultCount, size_t timeStepCount )
{
    m_reservoirBuilder.setResultInfo( resultCount, timeStepCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::addLocalGridRefinement( const cvf::Vec3st& minCellPosition,
                                                 const cvf::Vec3st& maxCellPosition,
                                                 const cvf::Vec3st& singleCellRefinementFactors )
{
    m_reservoirBuilder.addLocalGridRefinement( minCellPosition, maxCellPosition, singleCellRefinementFactors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::populateReservoir( RigEclipseCaseData* eclipseCase )
{
    m_reservoirBuilder.populateReservoir( eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderMockModel::enableWellData( bool enableWellData )
{
    m_reservoirBuilder.enableWellData( enableWellData );
}
