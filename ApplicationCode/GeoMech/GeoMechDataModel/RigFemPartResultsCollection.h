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

#pragma once

#include "RigFemResultAddress.h"

#include "cafTensor3.h"

#include "cvfCollection.h"
#include "cvfObject.h"

#include <QString>

#include <map>
#include <string>
#include <vector>

class RifGeoMechReaderInterface;
class RifElementPropertyReader;
class RigFemScalarResultFrames;
class RigFemPartResultsCollection;
class RigFemPartResults;
class RigStatisticsDataCache;
class RigFemPartCollection;
class RigFormationNames;

namespace caf
{
class ProgressInfo;
}

class RigFemPartResultsCollection : public cvf::Object
{
public:
    static const std::string FIELD_NAME_COMPACTION;

    RigFemPartResultsCollection( RifGeoMechReaderInterface*  readerInterface,
                                 RifElementPropertyReader*   elementPropertyReader,
                                 const RigFemPartCollection* femPartCollection );
    ~RigFemPartResultsCollection() override;

    void                 setActiveFormationNames( RigFormationNames* activeFormationNames );
    std::vector<QString> formationNames() const;

    void                             addElementPropertyFiles( const std::vector<QString>& filenames );
    std::vector<RigFemResultAddress> removeElementPropertyFiles( const std::vector<QString>& filenames );

    void   setCalculationParameters( double cohesion, double frictionAngleRad );
    double parameterCohesion() const { return m_cohesion; }
    double parameterFrictionAngleRad() const { return m_frictionAngleRad; }

    std::map<std::string, std::vector<std::string>> scalarFieldAndComponentNames( RigFemResultPosEnum resPos );
    std::vector<std::string>                        filteredStepNames() const;
    bool                                            assertResultsLoaded( const RigFemResultAddress& resVarAddr );
    void                                            deleteResult( const RigFemResultAddress& resVarAddr );
    void deleteResultFrame( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );
    std::vector<RigFemResultAddress> loadedResults() const;

    const std::vector<float>& resultValues( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );
    std::vector<caf::Ten3f>   tensors( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );
    int                       partCount() const;
    int                       frameCount();

    static float dsm( float p1, float p3, float tanFricAng, float cohPrTanFricAngle );

    void minMaxScalarValues( const RigFemResultAddress& resVarAddr, int frameIndex, double* localMin, double* localMax );
    void minMaxScalarValues( const RigFemResultAddress& resVarAddr, double* globalMin, double* globalMax );
    void posNegClosestToZero( const RigFemResultAddress& resVarAddr,
                              int                        frameIndex,
                              double*                    localPosClosestToZero,
                              double*                    localNegClosestToZero );
    void posNegClosestToZero( const RigFemResultAddress& resVarAddr,
                              double*                    globalPosClosestToZero,
                              double*                    globalNegClosestToZero );
    void meanScalarValue( const RigFemResultAddress& resVarAddr, double* meanValue );
    void meanScalarValue( const RigFemResultAddress& resVarAddr, int frameIndex, double* meanValue );
    void p10p90ScalarValues( const RigFemResultAddress& resVarAddr, double* p10, double* p90 );
    void p10p90ScalarValues( const RigFemResultAddress& resVarAddr, int frameIndex, double* p10, double* p90 );
    void sumScalarValue( const RigFemResultAddress& resVarAddr, double* sum );
    void sumScalarValue( const RigFemResultAddress& resVarAddr, int frameIndex, double* sum );
    const std::vector<size_t>& scalarValuesHistogram( const RigFemResultAddress& resVarAddr );
    const std::vector<size_t>& scalarValuesHistogram( const RigFemResultAddress& resVarAddr, int frameIndex );

    void minMaxScalarValuesOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                    int                        frameIndex,
                                                    double*                    localMin,
                                                    double*                    localMax );
    void minMaxScalarValuesOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                    double*                    globalMin,
                                                    double*                    globalMax );
    void posNegClosestToZeroOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                     int                        frameIndex,
                                                     double*                    localPosClosestToZero,
                                                     double*                    localNegClosestToZero );
    void posNegClosestToZeroOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                     double*                    globalPosClosestToZero,
                                                     double*                    globalNegClosestToZero );

    static std::vector<RigFemResultAddress> tensorComponentAddresses( const RigFemResultAddress& resVarAddr );
    static std::vector<RigFemResultAddress> tensorPrincipalComponentAdresses( const RigFemResultAddress& resVarAddr );
    static std::set<RigFemResultAddress>    normalizedResults();
    static bool                             isNormalizableResult( const RigFemResultAddress& result );

    void setNormalizationAirGap( double normalizationAirGap );

private:
    RigFemScalarResultFrames* findOrLoadScalarResult( int partIndex, const RigFemResultAddress& resVarAddr );

    RigFemScalarResultFrames* calculateDerivedResult( int partIndex, const RigFemResultAddress& resVarAddr );

    void calculateGammaFromFrames( int                             partIndex,
                                   const RigFemScalarResultFrames* totalStressComponentDataFrames,
                                   const RigFemScalarResultFrames* srcPORDataFrames,
                                   RigFemScalarResultFrames*       dstDataFrames,
                                   caf::ProgressInfo*              frameCountProgress );

    RigFemScalarResultFrames* calculateBarConvertedResult( int                        partIndex,
                                                           const RigFemResultAddress& convertedResultAddr,
                                                           const std::string&         fieldNameToConvert );
    RigFemScalarResultFrames* calculateEnIpPorBarResult( int partIndex, const RigFemResultAddress& convertedResultAddr );
    RigFemScalarResultFrames* calculateTimeLapseResult( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateMeanStressSEM( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateSFI( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateDSM( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateFOS( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateMeanStressSTM( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateDeviatoricStress( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateVolumetricStrain( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateDeviatoricStrain( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateSurfaceAlignedStress( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateSurfaceAngles( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculatePrincipalStressValues( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculatePrincipalStrainValues( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateCompactionValues( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateNE( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateSE( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateST_11_22_33( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateST_12_13_23( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateGamma( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateFormationIndices( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateStressGradients( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateNodalGradients( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* calculateNormalizedResult( int partIndex, const RigFemResultAddress& resVarAddr );

    const RigFormationNames* activeFormationNames() const;

private:
    cvf::Collection<RigFemPartResults>  m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface> m_readerInterface;
    cvf::ref<RifElementPropertyReader>  m_elementPropertyReader;
    cvf::cref<RigFemPartCollection>     m_femParts;
    cvf::cref<RigFormationNames>        m_activeFormationNamesData;

    double m_cohesion;
    double m_frictionAngleRad;
    double m_normalizationAirGap;

    RigStatisticsDataCache*          statistics( const RigFemResultAddress& resVarAddr );
    std::vector<RigFemResultAddress> getResAddrToComponentsToRead( const RigFemResultAddress& resVarAddr );
    std::map<RigFemResultAddress, cvf::ref<RigStatisticsDataCache>> m_resultStatistics;

    static std::vector<std::string> getStressComponentNames();
    static std::vector<std::string> getStressGradientComponentNames();
};

class RigFemPart;

class RigFemClosestResultIndexCalculator
{
public:
    RigFemClosestResultIndexCalculator( RigFemPart*         femPart,
                                        RigFemResultPosEnum resultPosition,
                                        int                 elementIndex,
                                        int                 m_face,
                                        const cvf::Vec3d&   intersectionPointInDomain );

    int resultIndexToClosestResult() { return m_resultIndexToClosestResult; }
    int closestNodeId() { return m_closestNodeId; }
    int closestElementNodeResIdx() { return m_closestElementNodeResIdx; }

private:
    int m_resultIndexToClosestResult;
    int m_closestNodeId;
    int m_closestElementNodeResIdx;
};
