/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigFemPartResultCalculatorPrincipalStrain.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorPrincipalStrain::RigFemPartResultCalculatorPrincipalStrain( RigFemPartResultsCollection& collection,
                                                                                      const std::string fieldName,
                                                                                      const std::string componentPrefix )
    : RigFemPartResultCalculator( collection )
    , m_fieldName( fieldName )
    , m_componentPrefix( componentPrefix )
    , m_componentNames( 3 )
{
    m_componentNames[0] = componentPrefix + "1";
    m_componentNames[1] = componentPrefix + "2";
    m_componentNames[2] = componentPrefix + "3";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorPrincipalStrain::~RigFemPartResultCalculatorPrincipalStrain()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorPrincipalStrain::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    if ( resVarAddr.fieldName != m_fieldName ) return false;

    for ( const auto& component : m_componentNames )
        if ( resVarAddr.componentName == component ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorPrincipalStrain::calculate( int                        partIndex,
                                                                                const RigFemResultAddress& resAddr )
{
    CVF_ASSERT( resAddr.componentName == m_componentNames[0] || resAddr.componentName == m_componentNames[1] ||
                resAddr.componentName == m_componentNames[2] );

    QString progressText = "Calculating " + QString::fromStdString( resAddr.fieldName + ": " + resAddr.componentName );

    caf::ProgressInfo frameCountProgress( static_cast<size_t>( m_resultCollection->frameCount() ) * 7, progressText );

    auto loadFrameLambda = [&]( const std::string& component ) {
        auto task = frameCountProgress.task( QString::fromStdString( "Loading " + component ),
                                             m_resultCollection->frameCount() );
        return m_resultCollection->findOrLoadScalarResult( partIndex, resAddr.copyWithComponent( component ) );
    };

    RigFemScalarResultFrames* e11Frames = loadFrameLambda( m_componentPrefix + "11" );
    RigFemScalarResultFrames* e22Frames = loadFrameLambda( m_componentPrefix + "22" );
    RigFemScalarResultFrames* e33Frames = loadFrameLambda( m_componentPrefix + "33" );
    RigFemScalarResultFrames* e12Frames = loadFrameLambda( m_componentPrefix + "12" );
    RigFemScalarResultFrames* e13Frames = loadFrameLambda( m_componentPrefix + "13" );
    RigFemScalarResultFrames* e23Frames = loadFrameLambda( m_componentPrefix + "23" );

    RigFemScalarResultFrames* e1Frames =
        m_resultCollection->createScalarResult( partIndex, resAddr.copyWithComponent( m_componentNames[0] ) );
    RigFemScalarResultFrames* e2Frames =
        m_resultCollection->createScalarResult( partIndex, resAddr.copyWithComponent( m_componentNames[1] ) );
    RigFemScalarResultFrames* e3Frames =
        m_resultCollection->createScalarResult( partIndex, resAddr.copyWithComponent( m_componentNames[2] ) );

    int frameCount = e11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        auto task = frameCountProgress.task( QString( "Frame %1" ).arg( fIdx ) );

        const std::vector<float>& e11 = e11Frames->frameData( fIdx );
        const std::vector<float>& e22 = e22Frames->frameData( fIdx );
        const std::vector<float>& e33 = e33Frames->frameData( fIdx );
        const std::vector<float>& e12 = e12Frames->frameData( fIdx );
        const std::vector<float>& e13 = e13Frames->frameData( fIdx );
        const std::vector<float>& e23 = e23Frames->frameData( fIdx );

        std::vector<float>& e1 = e1Frames->frameData( fIdx );
        std::vector<float>& e2 = e2Frames->frameData( fIdx );
        std::vector<float>& e3 = e3Frames->frameData( fIdx );

        size_t valCount = e11.size();

        e1.resize( valCount );
        e2.resize( valCount );
        e3.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            caf::Ten3f T( e11[vIdx], e22[vIdx], e33[vIdx], e12[vIdx], e23[vIdx], e13[vIdx] );
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principals = T.calculatePrincipals( principalDirs );
            e1[vIdx]              = principals[0];
            e2[vIdx]              = principals[1];
            e3[vIdx]              = principals[2];
        }
    }

    RigFemScalarResultFrames* requestedPrincipal = m_resultCollection->findOrLoadScalarResult( partIndex, resAddr );

    return requestedPrincipal;
}
