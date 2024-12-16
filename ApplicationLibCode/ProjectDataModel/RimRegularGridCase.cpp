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

#include "RimRegularGridCase.h"

#include "RifReaderRegularGridModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

CAF_PDM_SOURCE_INIT( RimRegularGridCase, "EclipseBoundingBoxCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularGridCase::RimRegularGridCase()
{
    CAF_PDM_InitObject( "Bounding Box Case", ":/Case48x48.png", "", "Bounding Box Case" );

    CAF_PDM_InitFieldNoDefault( &m_minimum, "Minimum", "Minimum" );
    m_minimum.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_maximum, "Maximum", "Maximum" );
    m_maximum.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_cellCountI, "CellCountI", 100, "Cell Count I" );
    m_cellCountI.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_cellCountJ, "CellCountJ", 100, "Cell Count J" );
    m_cellCountJ.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_cellCountK, "CellCountK", 10, "Cell Count K" );
    m_cellCountK.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularGridCase::setBoundingBox( const cvf::BoundingBox& boundingBox )
{
    m_minimum = boundingBox.min();
    m_maximum = boundingBox.max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRegularGridCase::setCellCount( const cvf::Vec3st& cellCount )
{
    m_cellCountI = static_cast<int>( cellCount.x() );
    m_cellCountJ = static_cast<int>( cellCount.y() );
    m_cellCountK = static_cast<int>( cellCount.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimRegularGridCase::createModel( QString modelName )
{
    cvf::ref<RifReaderRegularGridModel> reader    = new RifReaderRegularGridModel;
    cvf::ref<RigEclipseCaseData>        reservoir = new RigEclipseCaseData( this );

    reader->setWorldCoordinates( m_minimum, m_maximum );

    cvf::Vec3st gridPointDimensions( m_cellCountI, m_cellCountJ, m_cellCountK );
    reader->setGridPointDimensions( gridPointDimensions );

    reader->open( "", reservoir.p() );

    setReservoirData( reservoir.p() );

    return reader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimRegularGridCase::openEclipseGridFile()
{
    if ( eclipseCaseData() ) return true;

    auto readerInterface = createModel( "" );

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
    results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

    computeCachedData();

    return true;
}
