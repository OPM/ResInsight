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

#include "RigFemPartResultCalculatorPorosityPermeability.h"

#include "RiaEclipseUnitTools.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"

#include "Riu3DMainWindowTools.h"

#include "cafProgressInfo.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorPorosityPermeability::RigFemPartResultCalculatorPorosityPermeability(
    RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorPorosityPermeability::~RigFemPartResultCalculatorPorosityPermeability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorPorosityPermeability::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "PORO-PERM" &&
             ( resVarAddr.componentName == "PHI" || resVarAddr.componentName == "DPHI" ||
               resVarAddr.componentName == "PERM" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultCalculatorPorosityPermeability::calculate( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo stepCountProgress( m_resultCollection->timeStepCount() * 6, "" );
    stepCountProgress.setProgressDescription( "Calculating Porosity/Permeability" );

    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* srcPorePressureDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );
    stepCountProgress.incrementProgress();

    // Volumetric Strain
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* srcEVDataFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "EV" ) );

    stepCountProgress.incrementProgress();

    // Pore Compressibility
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* poreCompressibilityFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType, "COMPRESSIBILITY", "PORE" ) );
    if ( poreCompressibilityFrames->frameData( 0, 0 ).empty() )
    {
        QString txt = QString( "Failed to compute %1\n" ).arg( QString::fromStdString( resVarAddr.componentName ) );
        txt += "Missing pore compressibility data";

        Riu3DMainWindowTools::reportAndShowWarning( "Required data missing", txt );

        return nullptr;
    }

    stepCountProgress.incrementProgress();

    // Initial permeability (k0)
    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );
    RigFemScalarResultFrames* initialPermeabilityFrames = nullptr;
    if ( !m_resultCollection->initialPermeabilityAddress().isEmpty() )
    {
        initialPermeabilityFrames =
            m_resultCollection
                ->findOrLoadScalarResult( partIndex,
                                          RigFemResultAddress( RIG_ELEMENT,
                                                               m_resultCollection->initialPermeabilityAddress().toStdString(),
                                                               "" ) );
    }
    stepCountProgress.incrementProgress();

    stepCountProgress.setNextProgressIncrement( m_resultCollection->timeStepCount() );

    RigFemScalarResultFrames* voidRatioFrames =
        m_resultCollection->findOrLoadScalarResult( partIndex,
                                                    RigFemResultAddress( resVarAddr.resultPosType, "VOIDR", "" ) );

    RigFemScalarResultFrames* porosityFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "PHI" ) );
    RigFemScalarResultFrames* porosityDeltaFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "DPHI" ) );
    RigFemScalarResultFrames* permeabilityFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "PERM" ) );
    stepCountProgress.incrementProgress();

    const RigFemPart* femPart = m_resultCollection->parts()->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    stepCountProgress.setNextProgressIncrement( 1u );

    auto [refStepIdx, refFrame] = m_resultCollection->referenceStepAndFrameIndex();

    int timeSteps = srcEVDataFrames->timeStepCount();
    for ( int stepIdx = 0; stepIdx < timeSteps; stepIdx++ )
    {
        for ( int fIdx = 0; fIdx < srcEVDataFrames->frameCount( stepIdx ); fIdx++ )
        {
            const std::vector<float>& evData          = srcEVDataFrames->frameData( stepIdx, fIdx );
            const std::vector<float>& referenceEvData = srcEVDataFrames->frameData( refStepIdx, refFrame );
            const std::vector<float>& voidRatioData   = voidRatioFrames->frameData( 0, 0 );
            const std::vector<float>& refPorFrameData = srcPorePressureDataFrames->frameData( refStepIdx, refFrame );
            const std::vector<float>& porFrameData    = srcPorePressureDataFrames->frameData( stepIdx, fIdx );
            const std::vector<float>& poreCompressibilityFrameData = poreCompressibilityFrames->frameData( stepIdx, fIdx );

            std::vector<float>& porosityFrameData      = porosityFrames->frameData( stepIdx, fIdx );
            std::vector<float>& porosityDeltaFrameData = porosityDeltaFrames->frameData( stepIdx, fIdx );
            std::vector<float>& permeabilityFrameData  = permeabilityFrames->frameData( stepIdx, fIdx );

            size_t valCount = evData.size();
            porosityFrameData.resize( valCount );
            porosityDeltaFrameData.resize( valCount );
            permeabilityFrameData.resize( valCount );

            int elementCount = femPart->elementCount();

            std::vector<float> initialPermeabilityData;
            if ( initialPermeabilityFrames )
            {
                initialPermeabilityData = initialPermeabilityFrames->frameData( stepIdx, fIdx );
            }

#pragma omp parallel for
            for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
            {
                RigElementType elmType = femPart->elementType( elmIdx );

                int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

                if ( elmType == HEX8P )
                {
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        if ( elmNodResIdx < evData.size() )
                        {
                            // User provides initial permeability
                            double initialPermeability = 1.0;
                            if ( initialPermeabilityData.empty() )
                            {
                                // 1. Same value for all cells
                                initialPermeability = m_resultCollection->initialPermeabilityFixed();
                            }
                            else
                            {
                                // 2. From element property table
                                initialPermeability = initialPermeabilityData[elmIdx];
                            }

                            int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                            // Calculate initial porosity
                            double voidr           = voidRatioData[elmNodResIdx];
                            double initialPorosity = voidr / ( 1.0 + voidr );

                            // Calculate porosity change.
                            // No change for geostatic timestep
                            double deltaPorosity = 0.0;
                            if ( fIdx != 0 )
                            {
                                // Calculate difference in pore pressure between reference state and this state,
                                // and convert unit from Bar to Pascal.
                                double referencePorePressure = refPorFrameData[nodeIdx];
                                double framePorePressure     = porFrameData[nodeIdx];
                                double deltaPorePressure =
                                    RiaEclipseUnitTools::barToPascal( framePorePressure - referencePorePressure );

                                // Pore compressibility. Convert from 1/GPa to 1/Pa.
                                double poreCompressibility = poreCompressibilityFrameData[elmNodResIdx] / 1.0e9;

                                // Volumetric strain
                                double deltaEv = evData[elmNodResIdx] - referenceEvData[elmNodResIdx];

                                // Porosity change between reference state and initial state (geostatic).
                                deltaPorosity = initialPorosity * ( poreCompressibility * deltaPorePressure + deltaEv );
                            }

                            // Current porosity
                            double currentPorosity = initialPorosity + deltaPorosity;

                            // Permeability. Formula from Petunin, 2011.
                            double permeabilityExponent = m_resultCollection->permeabilityExponent();
                            double permeability         = initialPermeability *
                                                  std::pow( currentPorosity / initialPorosity, permeabilityExponent );

                            porosityFrameData[elmNodResIdx]      = currentPorosity;
                            porosityDeltaFrameData[elmNodResIdx] = deltaPorosity;
                            permeabilityFrameData[elmNodResIdx]  = permeability;
                        }
                    }
                }
                else
                {
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        if ( elmNodResIdx < poreCompressibilityFrameData.size() )
                        {
                            porosityFrameData[elmNodResIdx]      = inf;
                            porosityDeltaFrameData[elmNodResIdx] = inf;
                            permeabilityFrameData[elmNodResIdx]  = inf;
                        }
                    }
                }
            }
        }
        stepCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedResultFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedResultFrames;
}
