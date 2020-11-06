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

#include "RigFemPartResultCalculatorMudWeightWindow.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFemScalarResultFrames.h"
#include "RigGeoMechBoreHoleStressCalculator.h"
#include "RigGeoMechWellLogExtractor.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RimMudWeightWindowParameters.h"
#include "cafProgressInfo.h"

#include "cvfBoundingBox.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorMudWeightWindow::RigFemPartResultCalculatorMudWeightWindow( RigFemPartResultsCollection& collection )
    : RigFemPartResultCalculator( collection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultCalculatorMudWeightWindow::~RigFemPartResultCalculatorMudWeightWindow()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultCalculatorMudWeightWindow::isMatching( const RigFemResultAddress& resVarAddr ) const
{
    return ( resVarAddr.fieldName == "MUD-WEIGHT" &&
             ( resVarAddr.componentName == "MWW" || resVarAddr.componentName == "MWM" ||
               resVarAddr.componentName == "UMWL" || resVarAddr.componentName == "LMWL" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultCalculatorMudWeightWindow::calculate( int                        partIndex,
                                                                                const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( isMatching( resVarAddr ) );

    const std::vector<RimMudWeightWindowParameters::ParameterType> parameterTypes =
        { RimMudWeightWindowParameters::ParameterType::WELL_DEVIATION,
          RimMudWeightWindowParameters::ParameterType::WELL_AZIMUTH,
          RimMudWeightWindowParameters::ParameterType::UCS,
          RimMudWeightWindowParameters::ParameterType::POISSONS_RATIO,
          RimMudWeightWindowParameters::ParameterType::K0_FG,
          RimMudWeightWindowParameters::ParameterType::OBG0 };

    caf::ProgressInfo frameCountProgress( m_resultCollection->frameCount() * ( 5 + parameterTypes.size() ), "" );
    frameCountProgress.setProgressDescription( "Calculating Mud Weight Window" );

    std::map<RimMudWeightWindowParameters::ParameterType, RigFemScalarResultFrames*> parameterFrames;
    std::map<RimMudWeightWindowParameters::ParameterType, float>                     parameterValues;

    for ( auto parameterType : parameterTypes )
    {
        auto task =
            frameCountProgress.task( "Loading parameter: " +
                                         caf::AppEnum<RimMudWeightWindowParameters::ParameterType>::uiText( parameterType ),
                                     m_resultCollection->frameCount() );
        loadParameterFramesOrValue( parameterType, partIndex, parameterFrames, parameterValues );
    }

    double airGap       = m_resultCollection->airGapMudWeightWindow();
    double shMultiplier = m_resultCollection->shMultiplierMudWeightWindow();

    RimMudWeightWindowParameters::FractureGradientCalculationType fractureGradientCalculationType =
        m_resultCollection->fractureGradientCalculationTypeMudWeightWindow();
    RimMudWeightWindowParameters::UpperLimitType upperLimitParameter =
        m_resultCollection->upperLimitParameterMudWeightWindow();
    RimMudWeightWindowParameters::LowerLimitType lowerLimitParameter =
        m_resultCollection->lowerLimitParameterMudWeightWindow();

    // Pore pressure
    RigFemScalarResultFrames* porePressureDataFrames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading POR-Bar.", m_resultCollection->frameCount() );
        porePressureDataFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, "POR-Bar", "" ) );
    }

    // Stress (ST.S3)
    RigFemScalarResultFrames* stressDataFrames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading ST.S3", m_resultCollection->frameCount() );
        stressDataFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex,
                                                        RigFemResultAddress( resVarAddr.resultPosType, "ST", "S3" ) );
    }

    // Initial overburden gradient (ST.S33)
    RigFemScalarResultFrames* obg0DataFrames = nullptr;
    {
        auto task = frameCountProgress.task( "Loading ST.S33", m_resultCollection->frameCount() );
        obg0DataFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex,
                                                        RigFemResultAddress( resVarAddr.resultPosType, "ST", "S33" ) );
    }

    RigFemScalarResultFrames* mudWeightWindowFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "MWW" ) );
    RigFemScalarResultFrames* mudWeightMiddleFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "MWM" ) );
    RigFemScalarResultFrames* upperMudWeightLimitFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "UMWL" ) );
    RigFemScalarResultFrames* lowerMudWeightLimitFrames =
        m_resultCollection->createScalarResult( partIndex,
                                                RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "LMWL" ) );

    const RigFemPart*     femPart     = m_resultCollection->parts()->part( partIndex );
    const RigFemPartGrid* femPartGrid = femPart->getOrCreateStructGrid();

    const bool OBG0FromGrid =
        m_resultCollection->getCalculationParameterAddress( RimMudWeightWindowParameters::ParameterType::OBG0 ).isEmpty();

    RimMudWeightWindowParameters::NonReservoirPorePressureType PP_NonReservoirType =
        m_resultCollection->nonReservoirPorePressureTypeMudWeightWindow();
    double         hydrostaticMultiplier = m_resultCollection->hydrostaticMultiplierPPNonRes();
    const QString& nonReservoirAddress   = m_resultCollection->nonReservoirPorePressureAddressMudWeightWindow();
    RigFemScalarResultFrames* nonReservoirResultFrames = nullptr;

    if ( PP_NonReservoirType != RimMudWeightWindowParameters::NonReservoirPorePressureType::HYDROSTATIC &&
         !nonReservoirAddress.isEmpty() )
    {
        auto task = frameCountProgress.task( "Loading non-reservoir pore pressure.", m_resultCollection->frameCount() );
        nonReservoirResultFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex,
                                                        RigFemResultAddress( RIG_ELEMENT,
                                                                             nonReservoirAddress.toStdString(),
                                                                             "" ) );
    }

    float inf = std::numeric_limits<float>::infinity();

    frameCountProgress.setNextProgressIncrement( 1u );
    frameCountProgress.setProgressDescription( "Calculating Mud Weight Window." );

    int frameCount = stressDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& porFrameData        = porePressureDataFrames->frameData( fIdx );
        const std::vector<float>& initialPorFrameData = porePressureDataFrames->frameData( 0 );

        const std::vector<float>& stressFrameData = stressDataFrames->frameData( fIdx );
        const std::vector<float>& obg0FrameData   = obg0DataFrames->frameData( 0 );

        std::vector<float>& mudWeightWindowFrameData     = mudWeightWindowFrames->frameData( fIdx );
        std::vector<float>& mudWeightMiddleFrameData     = mudWeightMiddleFrames->frameData( fIdx );
        std::vector<float>& upperMudWeightLimitFrameData = upperMudWeightLimitFrames->frameData( fIdx );
        std::vector<float>& lowerMudWeightLimitFrameData = lowerMudWeightLimitFrames->frameData( fIdx );

        size_t valCount = stressFrameData.size();
        mudWeightWindowFrameData.resize( valCount );
        mudWeightMiddleFrameData.resize( valCount );
        upperMudWeightLimitFrameData.resize( valCount );
        lowerMudWeightLimitFrameData.resize( valCount );

        int elementCount = femPart->elementCount();

        std::map<RimMudWeightWindowParameters::ParameterType, std::vector<float>> parameterFrameData;
        for ( auto parameterType : parameterTypes )
        {
            parameterFrameData[parameterType] = loadDataForFrame( parameterType, parameterFrames, fIdx );
        }

        std::vector<float> nonReservoirPP;
        if ( nonReservoirResultFrames )
        {
            nonReservoirPP = nonReservoirResultFrames->frameData( 0 );
        }

        // Load stress
        RigFemResultAddress     stressResAddr( RIG_ELEMENT_NODAL, "ST", "" );
        std::vector<caf::Ten3f> vertexStressesFloat = m_resultCollection->tensors( stressResAddr, partIndex, fIdx );

        std::vector<caf::Ten3d> vertexStresses;
        vertexStresses.reserve( vertexStressesFloat.size() );
        for ( const caf::Ten3f& floatTensor : vertexStressesFloat )
        {
            vertexStresses.push_back( caf::Ten3d( floatTensor ) );
        }

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            bool isHexahedron = femPart->isHexahedron( elmIdx );
            int  elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

            // Use hydrostatic pressure from cell centroid.
            // Use centroid to avoid intra-element differences
            cvf::Vec3d cellCentroid       = femPartGrid->cellCentroid( elmIdx );
            double     cellCentroidTvdRKB = -cellCentroid.z() + airGap;
            double     waterDensityGCM3   = 1.03;
            double     hydroStaticPressure =
                RigGeoMechWellLogExtractor::hydroStaticPorePressureAtDepth( cellCentroidTvdRKB, waterDensityGCM3 );
            double hydroStaticPressureForNormalization =
                RigGeoMechWellLogExtractor::hydroStaticPorePressureAtDepth( cellCentroidTvdRKB, 1.0 );

            if ( isHexahedron && hydroStaticPressureForNormalization != 0.0 )
            {
                double wellPathDeviation = getValueForElement( RimMudWeightWindowParameters::ParameterType::WELL_DEVIATION,
                                                               parameterFrameData,
                                                               parameterValues,
                                                               elmIdx );

                double wellPathAzimuth = getValueForElement( RimMudWeightWindowParameters::ParameterType::WELL_AZIMUTH,
                                                             parameterFrameData,
                                                             parameterValues,
                                                             elmIdx );

                double ucsBar = getValueForElement( RimMudWeightWindowParameters::ParameterType::UCS,
                                                    parameterFrameData,
                                                    parameterValues,
                                                    elmIdx );

                double poissonsRatio = getValueForElement( RimMudWeightWindowParameters::ParameterType::POISSONS_RATIO,
                                                           parameterFrameData,
                                                           parameterValues,
                                                           elmIdx );

                double K0_FG = getValueForElement( RimMudWeightWindowParameters::ParameterType::K0_FG,
                                                   parameterFrameData,
                                                   parameterValues,
                                                   elmIdx );

                double OBG0 = 0.0;
                if ( !OBG0FromGrid )
                {
                    OBG0 = getValueForElement( RimMudWeightWindowParameters::ParameterType::OBG0,
                                               parameterFrameData,
                                               parameterValues,
                                               elmIdx );
                }

                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < stressFrameData.size() )
                    {
                        // Pore pressure (unit: Bar)
                        float porePressureBar        = porFrameData[elmNodResIdx];
                        float initialPorePressureBar = initialPorFrameData[elmNodResIdx];

                        // Initial overburden gradient
                        if ( OBG0FromGrid )
                        {
                            OBG0 = obg0FrameData[elmNodResIdx];
                        }

                        // FG is for sands, SFG for shale. Sands has valid PP, shale does not.
                        bool isSand = ( porePressureBar != inf );

                        //
                        if ( porePressureBar == inf )
                        {
                            //
                            if ( PP_NonReservoirType ==
                                 RimMudWeightWindowParameters::NonReservoirPorePressureType::HYDROSTATIC )
                            {
                                porePressureBar        = hydroStaticPressure * hydrostaticMultiplier;
                                initialPorePressureBar = hydroStaticPressure * hydrostaticMultiplier;
                            }
                            else if ( !nonReservoirPP.empty() )
                            {
                                // Get from element table
                                porePressureBar        = nonReservoirPP[elmIdx];
                                initialPorePressureBar = nonReservoirPP[elmIdx];
                            }
                        }

                        caf::Ten3d segmentStress = caf::Ten3d( vertexStressesFloat[elmNodResIdx] );

                        cvf::Vec3d wellPathTangent = calculateWellPathTangent( wellPathAzimuth, wellPathDeviation );
                        caf::Ten3d wellPathStressFloat =
                            RigGeoMechWellLogExtractor::transformTensorToWellPathOrientation( wellPathTangent,
                                                                                              segmentStress );
                        caf::Ten3d wellPathStressDouble( wellPathStressFloat );

                        // Calculate upper limit
                        float upperLimit = inf;
                        if ( upperLimitParameter == RimMudWeightWindowParameters::UpperLimitType::FG && isSand )
                        {
                            RigGeoMechBoreHoleStressCalculator sigmaCalculator( wellPathStressDouble,
                                                                                porePressureBar,
                                                                                poissonsRatio,
                                                                                ucsBar,
                                                                                32 );
                            upperLimit = sigmaCalculator.solveFractureGradient() / hydroStaticPressureForNormalization;
                        }
                        else if ( upperLimitParameter == RimMudWeightWindowParameters::UpperLimitType::SH_MIN )
                        {
                            upperLimit = stressFrameData[elmNodResIdx] / hydroStaticPressureForNormalization;
                        }

                        //
                        if ( upperLimit == inf )
                        {
                            if ( fractureGradientCalculationType ==
                                 RimMudWeightWindowParameters::FractureGradientCalculationType::DERIVED_FROM_K0FG )
                            {
                                float PP0            = initialPorePressureBar / hydroStaticPressureForNormalization;
                                float normalizedOBG0 = OBG0 / hydroStaticPressureForNormalization;
                                upperLimit           = K0_FG * ( normalizedOBG0 - PP0 ) + PP0;
                            }
                            else
                            {
                                upperLimit =
                                    stressFrameData[elmNodResIdx] * shMultiplier / hydroStaticPressureForNormalization;
                            }
                        }

                        // Calculate lower limit
                        float lowerLimit = inf;
                        if ( lowerLimitParameter == RimMudWeightWindowParameters::LowerLimitType::PORE_PRESSURE )
                        {
                            lowerLimit = porePressureBar;
                        }
                        else if ( lowerLimitParameter ==
                                  RimMudWeightWindowParameters::LowerLimitType::MAX_OF_PORE_PRESSURE_AND_SFG )
                        {
                            if ( isSand )
                            {
                                lowerLimit = porePressureBar;
                            }
                            else
                            {
                                RigGeoMechBoreHoleStressCalculator sigmaCalculator( wellPathStressDouble,
                                                                                    hydroStaticPressureForNormalization,
                                                                                    poissonsRatio,
                                                                                    ucsBar,
                                                                                    32 );

                                double SFG = sigmaCalculator.solveStassiDalia();
                                lowerLimit = std::max( porePressureBar, static_cast<float>( SFG ) );
                            }
                        }

                        // Upper limit values have already been normalized where appropriate
                        upperMudWeightLimitFrameData[elmNodResIdx] = upperLimit;

                        // Normalize by hydrostatic pore pressure
                        lowerMudWeightLimitFrameData[elmNodResIdx] = lowerLimit / hydroStaticPressureForNormalization;
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < stressFrameData.size() )
                    {
                        mudWeightWindowFrameData[elmNodResIdx]     = inf;
                        mudWeightMiddleFrameData[elmNodResIdx]     = inf;
                        upperMudWeightLimitFrameData[elmNodResIdx] = inf;
                        lowerMudWeightLimitFrameData[elmNodResIdx] = inf;
                    }
                }
            }
        }

        size_t kRefLayer = m_resultCollection->referenceLayerMudWeightWindow();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            bool isHexahedron = femPart->isHexahedron( elmIdx );

            int elmNodeCount = RigFemTypes::elementNodeCount( femPart->elementType( elmIdx ) );

            size_t i, j, k;
            bool   validIndex = femPartGrid->ijkFromCellIndex( elmIdx, &i, &j, &k );
            size_t kMin       = std::min( k, kRefLayer );
            size_t kMax       = std::max( k, kRefLayer );

            if ( isHexahedron && validIndex )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );

                    float maxLowerMudWeightLimit = lowerMudWeightLimitFrameData[elmNodResIdx];
                    float minUpperMudWeightLimit = upperMudWeightLimitFrameData[elmNodResIdx];

                    for ( size_t currentK = kMin; currentK < kMax; currentK++ )
                    {
                        size_t kElmIdx = femPartGrid->cellIndexFromIJK( i, j, currentK );
                        if ( kElmIdx != cvf::UNDEFINED_SIZE_T && femPart->isHexahedron( kElmIdx ) )
                        {
                            size_t kElmNodResIdx = femPart->elementNodeResultIdx( static_cast<int>( kElmIdx ), elmNodIdx );

                            float currentLowerMudWeightLimit = lowerMudWeightLimitFrameData[kElmNodResIdx];
                            if ( currentLowerMudWeightLimit > maxLowerMudWeightLimit )
                            {
                                maxLowerMudWeightLimit = currentLowerMudWeightLimit;
                            }

                            float currentUpperMudWeightLimit = upperMudWeightLimitFrameData[kElmNodResIdx];
                            if ( currentUpperMudWeightLimit < minUpperMudWeightLimit )
                            {
                                minUpperMudWeightLimit = currentUpperMudWeightLimit;
                            }
                        }
                    }

                    float mudWeightWindow                  = minUpperMudWeightLimit - maxLowerMudWeightLimit;
                    mudWeightWindowFrameData[elmNodResIdx] = mudWeightWindow;

                    float mudWeightMiddle = inf;
                    if ( mudWeightWindow > 0.0 )
                    {
                        mudWeightMiddle = maxLowerMudWeightLimit + mudWeightWindow / 2.0;
                    }
                    mudWeightMiddleFrameData[elmNodResIdx] = mudWeightMiddle;
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedResultFrames = m_resultCollection->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedResultFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigFemPartResultCalculatorMudWeightWindow::calculateWellPathTangent( double azimuth, double inclination )
{
    double aziRad = cvf::Math::toRadians( azimuth );
    double incRad = cvf::Math::toRadians( inclination );
    return RiaOffshoreSphericalCoords::unitVectorFromAziInc( aziRad, incRad );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultCalculatorMudWeightWindow::loadParameterFramesOrValue(
    RimMudWeightWindowParameters::ParameterType                                       parameterType,
    int                                                                               partIndex,
    std::map<RimMudWeightWindowParameters::ParameterType, RigFemScalarResultFrames*>& parameterFrames,
    std::map<RimMudWeightWindowParameters::ParameterType, float>&                     parameterValues )
{
    RigFemScalarResultFrames* resultFrames = nullptr;

    QString resultAddress = m_resultCollection->getCalculationParameterAddress( parameterType );
    if ( !resultAddress.isEmpty() )
    {
        resultFrames =
            m_resultCollection->findOrLoadScalarResult( partIndex,
                                                        RigFemResultAddress( RIG_ELEMENT, resultAddress.toStdString(), "" ) );
        parameterFrames[parameterType] = resultFrames;
    }

    parameterValues[parameterType] = m_resultCollection->getCalculationParameterValue( parameterType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RigFemPartResultCalculatorMudWeightWindow::loadDataForFrame(
    RimMudWeightWindowParameters::ParameterType                                       parameterType,
    std::map<RimMudWeightWindowParameters::ParameterType, RigFemScalarResultFrames*>& parameterFrames,
    int                                                                               frameIndex )
{
    auto it = parameterFrames.find( parameterType );
    if ( it != parameterFrames.end() )
    {
        RigFemScalarResultFrames* frame        = it->second;
        std::vector<float>        dataForFrame = frame->frameData( frameIndex );
        return dataForFrame;
    }
    else
    {
        return std::vector<float>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigFemPartResultCalculatorMudWeightWindow::getValueForElement(
    RimMudWeightWindowParameters::ParameterType                                      parameterType,
    const std::map<RimMudWeightWindowParameters::ParameterType, std::vector<float>>& parameterFrameData,
    const std::map<RimMudWeightWindowParameters::ParameterType, float>               parameterValues,
    int                                                                              elmIdx )
{
    // Use data per element if available
    auto it = parameterFrameData.find( parameterType );
    if ( it != parameterFrameData.end() )
    {
        if ( !it->second.empty() && static_cast<size_t>( elmIdx ) < it->second.size() )
        {
            return it->second[elmIdx];
        }
    }

    // Use fixed value
    auto value = parameterValues.find( parameterType );
    if ( value != parameterValues.end() )
    {
        return value->second;
    }
    else
    {
        // No value found (should not happen)
        return std::numeric_limits<float>::infinity();
    }
}
