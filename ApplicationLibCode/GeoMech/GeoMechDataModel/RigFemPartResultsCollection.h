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

#include "RimMudWeightWindowParameters.h"

#include "cafTensor3.h"

#include "cvfCollection.h"
#include "cvfObject.h"

#include <QString>

#include <map>
#include <memory>
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
class RigFemPartResultCalculator;

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
    std::map<std::string, QString>   addressesInElementPropertyFiles( const std::vector<QString>& filenames );

    void   setCalculationParameters( double cohesion, double frictionAngleRad );
    double parameterCohesion() const { return m_cohesion; }
    double parameterFrictionAngleRad() const { return m_frictionAngleRad; }

    void    setBiotCoefficientParameters( double fixedFactor, const QString& biotResultAddress );
    double  biotFixedFactor() const { return m_biotFixedFactor; }
    QString biotResultAddress() const { return m_biotResultAddress; }

    void    setPermeabilityParameters( double         fixedInitalPermeability,
                                       const QString& initialPermeabilityAddress,
                                       double         permeabilityExponent );
    double  initialPermeabilityFixed() const;
    QString initialPermeabilityAddress() const;
    double  permeabilityExponent() const;

    void    setCalculationParameters( RimMudWeightWindowParameters::ParameterType parameterType,
                                      const QString&                              address,
                                      double                                      value );
    double  getCalculationParameterValue( RimMudWeightWindowParameters::ParameterType ) const;
    QString getCalculationParameterAddress( RimMudWeightWindowParameters::ParameterType ) const;

    void setMudWeightWindowParameters( double                                                        airGap,
                                       RimMudWeightWindowParameters::UpperLimitType                  upperLimit,
                                       RimMudWeightWindowParameters::LowerLimitType                  lowerLimit,
                                       int                                                           referenceLayer,
                                       RimMudWeightWindowParameters::FractureGradientCalculationType fgCalculationType,
                                       double                                                        shMultiplier,
                                       RimMudWeightWindowParameters::NonReservoirPorePressureType nonReservoirPorePressureType,
                                       double         hydroStaticMultiplierPPNonRes,
                                       const QString& nonReservoirPorePressureAddress );

    double airGapMudWeightWindow() const;
    double shMultiplierMudWeightWindow() const;

    double                                                     hydrostaticMultiplierPPNonRes() const;
    RimMudWeightWindowParameters::NonReservoirPorePressureType nonReservoirPorePressureTypeMudWeightWindow() const;
    const QString&                                             nonReservoirPorePressureAddressMudWeightWindow() const;

    RimMudWeightWindowParameters::UpperLimitType upperLimitParameterMudWeightWindow() const;
    RimMudWeightWindowParameters::LowerLimitType lowerLimitParameterMudWeightWindow() const;
    size_t                                       referenceLayerMudWeightWindow() const;

    RimMudWeightWindowParameters::FractureGradientCalculationType fractureGradientCalculationTypeMudWeightWindow() const;

    double waterDensityShearSlipIndicator() const;
    void   setWaterDensityShearSlipIndicator( double waterDensity );

    std::map<std::string, std::vector<std::string>> scalarFieldAndComponentNames( RigFemResultPosEnum resPos );
    std::vector<std::string>                        filteredStepNames() const;
    bool                                            assertResultsLoaded( const RigFemResultAddress& resVarAddr );

    void deleteResult( const RigFemResultAddress& resVarAddr );
    void deleteResultForAllTimeSteps( const std::vector<RigFemResultAddress>& addresses );
    void deleteResultFrame( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );

    std::vector<RigFemResultAddress> loadedResults() const;

    const std::vector<float>& resultValues( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );
    void globalResultValues( const RigFemResultAddress& resVarAddr, int timeStepIndex, std::vector<float>& resultValues );

    std::vector<caf::Ten3f> tensors( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex );

    const RigFemPartCollection* parts() const;
    int                         partCount() const;
    int                         frameCount();

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

    static bool isResultInSet( const RigFemResultAddress& result, const std::set<RigFemResultAddress>& results );

    static std::vector<RigFemResultAddress> tensorComponentAddresses( const RigFemResultAddress& resVarAddr );
    static std::vector<RigFemResultAddress> tensorPrincipalComponentAdresses( const RigFemResultAddress& resVarAddr );
    static std::set<RigFemResultAddress>    normalizedResults();
    static bool                             isNormalizableResult( const RigFemResultAddress& result );

    void   setNormalizationAirGap( double normalizationAirGap );
    double normalizationAirGap() const;

    void setReferenceTimeStep( int referenceTimeStep );
    int  referenceTimeStep() const;

    static std::set<RigFemResultAddress> referenceCaseDependentResults();
    static bool                          isReferenceCaseDependentResult( const RigFemResultAddress& result );

    static std::set<RigFemResultAddress> initialPermeabilityDependentResults();
    static std::set<RigFemResultAddress> mudWeightWindowResults();

    RigFemScalarResultFrames* findOrLoadScalarResult( int partIndex, const RigFemResultAddress& resVarAddr );
    RigFemScalarResultFrames* createScalarResult( int partIndex, const RigFemResultAddress& resVarAddr );
    void                      deleteAllScalarResults();

    bool                            isValidBiotData( const std::vector<float>& biotData, size_t elementCount ) const;
    static std::vector<std::string> getStressComponentNames( bool includeShear = true );
    static std::vector<std::string> getStressGradientComponentNames( bool includeShear = true );
    static std::vector<std::string> getStressAnisotropyComponentNames();
    const RigFormationNames*        activeFormationNames() const;

private:
    RigFemScalarResultFrames* calculateDerivedResult( int partIndex, const RigFemResultAddress& resVarAddr );

private:
    cvf::Collection<RigFemPartResults>  m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface> m_readerInterface;
    cvf::ref<RifElementPropertyReader>  m_elementPropertyReader;
    cvf::cref<RigFemPartCollection>     m_femParts;
    cvf::cref<RigFormationNames>        m_activeFormationNamesData;

    double m_cohesion;
    double m_frictionAngleRad;
    double m_normalizationAirGap;

    double  m_biotFixedFactor;
    QString m_biotResultAddress;

    double  m_initialPermeabilityFixed;
    QString m_initialPermeabilityResultAddress;
    double  m_permeabilityExponent;

    int m_referenceTimeStep;

    double                                                        m_airGapMudWeightWindow;
    double                                                        m_shMultiplierMudWeightWindow;
    int                                                           m_referenceLayerMudWeightWindow;
    RimMudWeightWindowParameters::UpperLimitType                  m_upperLimitParameterMudWeightWindow;
    RimMudWeightWindowParameters::LowerLimitType                  m_lowerLimitParameterMudWeightWindow;
    RimMudWeightWindowParameters::FractureGradientCalculationType m_fractureGradientCalculationTypeMudWeightWindow;

    RimMudWeightWindowParameters::NonReservoirPorePressureType m_nonReservoirPorePressureTypeMudWeightWindow;
    double                                                     m_hydrostaticMultiplierPPNonResMudWeightWindow;
    QString                                                    m_nonReservoirPorePressureAddressMudWeightWindow;

    std::map<RimMudWeightWindowParameters::ParameterType, QString> parameterAddresses;
    std::map<RimMudWeightWindowParameters::ParameterType, double>  parameterValues;

    double m_waterDensityShearSlipIndicator;

    std::vector<std::unique_ptr<RigFemPartResultCalculator>> m_resultCalculators;

    RigStatisticsDataCache*          statistics( const RigFemResultAddress& resVarAddr );
    std::vector<RigFemResultAddress> getResAddrToComponentsToRead( const RigFemResultAddress& resVarAddr );
    std::map<RigFemResultAddress, cvf::ref<RigStatisticsDataCache>> m_resultStatistics;
};
