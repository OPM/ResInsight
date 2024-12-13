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

#include "RimEclipseBoundingBoxCase.h"

#include "RifReaderBoundingBoxModel.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

CAF_PDM_SOURCE_INIT( RimEclipseBoundingBoxCase, "EclipseBoundingBoxCase" );
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseBoundingBoxCase::RimEclipseBoundingBoxCase()
{
    CAF_PDM_InitObject( "Bounding Box Case", ":/Case48x48.png", "", "Bounding Box Case" );

    CAF_PDM_InitFieldNoDefault( &m_minimum, "Minimum", "Minimum" );
    m_minimum.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_maximum, "Maximum", "Maximum" );
    m_maximum.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

void RimEclipseBoundingBoxCase::setBoundingBox( const cvf::BoundingBox& boundingBox )
{
    m_minimum = boundingBox.min();
    m_maximum = boundingBox.max();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseBoundingBoxCase::createModel( QString modelName )
{
    cvf::ref<RifReaderBoundingBoxModel> reader    = new RifReaderBoundingBoxModel;
    cvf::ref<RigEclipseCaseData>        reservoir = new RigEclipseCaseData( this );

    reader->setWorldCoordinates( m_minimum, m_maximum );

    cvf::Vec3st gridPointDimensions( 50, 50, 10 );
    reader->setGridPointDimensions( gridPointDimensions );

    reader->open( "", reservoir.p() );

    setReservoirData( reservoir.p() );

    return reader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseBoundingBoxCase::openEclipseGridFile()
{
    if ( eclipseCaseData() ) return true;

    auto readerInterface = createModel( "" );

    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->setReaderInterface( readerInterface.p() );
    results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->setReaderInterface( readerInterface.p() );

    computeCachedData();
    results( RiaDefines::PorosityModelType::MATRIX_MODEL )->computeCellVolumes();

    return true;
}
