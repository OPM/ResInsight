/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <cstdlib>

#include "RifElementPropertyReader.h"
#include "RifGeoMechReaderInterface.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RifInpReader.h"
#include "RifVtkReader.h"
#ifdef USE_ODB_API
#include "RifOdbReader.h"
#endif

#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"

#include "cafProgressInfo.h"
#include "cvfBoundingBox.h"

#include <QString>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::RigGeoMechCaseData( const std::string& fileName )
    : m_geoMechCaseFileName( fileName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::~RigGeoMechCaseData()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RigGeoMechCaseData::femParts()
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPartCollection* RigGeoMechCaseData::femParts() const
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFemPartResultsCollection* RigGeoMechCaseData::femPartResults() const
{
    return m_femPartResultsColl.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection* RigGeoMechCaseData::femPartResults()
{
    return m_femPartResultsColl.p();
}

#include "RiaStdStringTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::open( std::string* errorMessage )
{
    auto filename = RiaStdStringTools::toUpper( m_geoMechCaseFileName );

    if ( filename.ends_with( ".INP" ) )
    {
        m_readerInterface = new RifInpReader();
    }
    else if ( filename.ends_with( ".PVD" ) )
    {
        m_readerInterface = new RifVtkReader();
    }
#ifdef USE_ODB_API
    else if ( filename.ends_with( ".ODB" ) )
    {
        m_readerInterface = new RifOdbReader();
    }
#endif

    return m_readerInterface.notNull() && m_readerInterface->openFile( m_geoMechCaseFileName, errorMessage );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::readTimeSteps( std::string* errorMessage, std::vector<std::string>* stepNames )
{
    CVF_ASSERT( stepNames );
    if ( m_readerInterface.notNull() && m_readerInterface->isOpen() )
    {
        *stepNames = m_readerInterface->allStepNames();
        return true;
    }
    *errorMessage = std::string( "Could not read time steps" );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::readFemParts( std::string* errorMessage, const std::vector<size_t>& timeStepFilter, bool readOnlyLastFrame )
{
    CVF_ASSERT( errorMessage );

    if ( m_readerInterface.notNull() && m_readerInterface->isOpen() )
    {
        m_readerInterface->setTimeStepFilter( timeStepFilter, readOnlyLastFrame );
        m_femParts = new RigFemPartCollection();

        caf::ProgressInfo progress( 10, "" ); // Here because the next call uses progress
        progress.setNextProgressIncrement( 9 );
        if ( m_readerInterface->readFemParts( m_femParts.p() ) )
        {
            progress.incrementProgress();
            progress.setProgressDescription( "Calculating element neighbors" );

            if ( m_femParts->partCount() > 0 )
            {
                m_elementPropertyReader = new RifElementPropertyReader( m_femParts->part( 0 )->elementIdxToId() );
                // Initialize results containers
                m_femPartResultsColl = new RigFemPartResultsCollection( m_readerInterface.p(), m_elementPropertyReader.p(), m_femParts.p() );

                // Calculate derived Fem data
                for ( int pIdx = 0; pIdx < m_femParts->partCount(); ++pIdx )
                {
                    m_femParts->part( pIdx )->assertNodeToElmIndicesIsCalculated();
                    m_femParts->part( pIdx )->assertElmNeighborsIsCalculated();
                }
                return true;
            }
        }
    }

    *errorMessage = std::string( "Could not read FEM parts" );
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::readDisplacements( std::string* errorMessage, int partId, int timeStep, int frameIndex, std::vector<cvf::Vec3f>* displacements )
{
    CVF_ASSERT( errorMessage );

    if ( m_readerInterface.notNull() && m_readerInterface->isOpen() )
    {
        m_readerInterface->readDisplacements( partId, timeStep, frameIndex, displacements );
        return true;
    }

    *errorMessage = std::string( "Could not read displacements." );
    return false;
}
