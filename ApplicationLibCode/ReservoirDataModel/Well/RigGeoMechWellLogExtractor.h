/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RiaDefines.h"
#include "RigWbsParameter.h"
#include "RigWellLogExtractor.h"

#include "RigFemResultPosEnum.h"

#include "cafAppEnum.h"
#include "cafTensor3.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RigFemPart;
class RigFemResultAddress;
class RigGeoMechCaseData;
class RigWellPath;

namespace cvf
{
class BoundingBox;
}

//==================================================================================================
///
//==================================================================================================
class RigGeoMechWellLogExtractor : public RigWellLogExtractor
{
public:
    static const double PURE_WATER_DENSITY_GCM3;
    static const double GRAVITY_ACCEL;

    using WbsParameterSource     = RigWbsParameter::Source;
    using WbsParameterSourceEnum = RigWbsParameter::SourceEnum;

public:
    RigGeoMechWellLogExtractor( gsl::not_null<RigGeoMechCaseData*> aCase,
                                int                                partId,
                                gsl::not_null<const RigWellPath*>  wellpath,
                                const std::string&                 wellCaseErrorMsgName );

    void performCurveDataSmoothing( int                  timeStepIndex,
                                    int                  frameIndex,
                                    std::vector<double>* mds,
                                    std::vector<double>* tvds,
                                    std::vector<double>* values,
                                    const double         smoothingTreshold );

    QString curveData( const RigFemResultAddress& resAddr, int timeStepIndex, int frameIndex, std::vector<double>* values );
    const RigGeoMechCaseData* caseData();

    void setWbsLasValues( const RigWbsParameter& parameter, const std::vector<std::pair<double, double>>& values );
    void setWbsParametersSource( RigWbsParameter parameter, WbsParameterSource source );
    void setWbsUserDefinedValue( RigWbsParameter parameter, double userDefinedValue );

    static QString parameterInputUnits( const RigWbsParameter& parameter );

    std::vector<double> porePressureSourceRegions( int timeStepIndex, int frameIndex );
    std::vector<double> poissonSourceRegions( int timeStepIndex, int frameIndex );
    std::vector<double> ucsSourceRegions( int timeStepIndex, int frameIndex );

    static caf::Ten3d transformTensorToWellPathOrientation( const cvf::Vec3d& wellPathTangent, const caf::Ten3d& wellPathTensor );

    static double hydroStaticPorePressureAtDepth( double effectiveDepthMeters, double waterDensityGCM3 = PURE_WATER_DENSITY_GCM3 );

    double waterDepth() const;
    double estimateWaterDepth() const;

    int  partId() const;
    bool valid() const;

private:
    enum WellPathTangentCalculation
    {
        TangentFollowWellPathSegments,
        TangentConstantWithinCell
    };

    std::vector<WbsParameterSource> calculateWbsParameterForAllSegments( const RigWbsParameter& parameter,
                                                                         WbsParameterSource     primarySource,
                                                                         int                    timeStepIndex,
                                                                         int                    frameIndex,
                                                                         std::vector<double>*   outputValues,
                                                                         bool                   allowNormalization );
    std::vector<WbsParameterSource> calculateWbsParameterForAllSegments( const RigWbsParameter& parameter,
                                                                         int                    timeStepIndex,
                                                                         int                    frameIndex,
                                                                         std::vector<double>*   outputValues,
                                                                         bool                   allowNormalization );
    std::vector<WbsParameterSource> calculateWbsParametersForAllSegments( const RigFemResultAddress& resAddr,
                                                                          int                        timeStepIndex,
                                                                          int                        frameIndex,
                                                                          std::vector<double>*       values,
                                                                          bool                       allowNormalization );

    void                            wellPathAngles( const RigFemResultAddress& resAddr, std::vector<double>* values );
    std::vector<WbsParameterSource> wellPathScaledCurveData( const RigFemResultAddress& resAddr,
                                                             int                        timeStepIndex,
                                                             int                        frameIndex,
                                                             std::vector<double>*       values,
                                                             bool                       forceGridSourceforPPReservoir = false );
    void wellBoreWallCurveData( const RigFemResultAddress& resAddr, int timeStepIndex, int frameIndex, std::vector<double>* values );

    void wellBoreFGShale( const RigWbsParameter& parameter, int timeStepIndex, int frameIndex, std::vector<double>* values );
    void wellBoreSH_MatthewsKelly( int                  timeStepIndex,
                                   int                  frameIndex,
                                   const QString&       wbsPPResultName,
                                   const QString&       wbsPP0ResultName,
                                   std::vector<double>* values );

    void wellBoreFGDerivedFromK0FG( const QString& ppResult, int timeStepIndex, int frameIndex, std::vector<double>* values, bool onlyForPPReservoir );

    template <typename T>
    T interpolateGridResultValue( RigFemResultPosEnum resultPosType, const std::vector<T>& gridResultValues, int64_t intersectionIdx ) const;
    size_t              gridResultIndexFace( size_t elementIdx, cvf::StructGridInterface::FaceType cellFace, int faceLocalNodeIdx ) const;
    void                calculateIntersection();
    std::vector<size_t> findCloseCells( const cvf::BoundingBox& bb );
    cvf::Vec3d          calculateLengthInCell( size_t cellIndex, const cvf::Vec3d& startPoint, const cvf::Vec3d& endPoint ) const override;
    cvf::Vec3d          calculateWellPathTangent( int64_t intersectionIdx, WellPathTangentCalculation calculationType ) const;

    cvf::Vec3f cellCentroid( size_t intersectionIdx ) const;
    double     getWellLogIntersectionValue( size_t intersectionIdx, const std::vector<std::pair<double, double>>& wellLogValues ) const;

    template <typename T>
    bool          averageIntersectionValuesToSegmentValue( size_t                intersectionIdx,
                                                           const std::vector<T>& intersectionValues,
                                                           const T&              invalidValue,
                                                           T*                    averagedSegmentValue ) const;
    static double pascalToBar( double pascalValue );

    template <typename T>
    std::vector<T> interpolateInterfaceValues( RigFemResultAddress   nativeAddr,
                                               int                   timeStepIndex,
                                               int                   frameIndex,
                                               const std::vector<T>& unscaledResultValues );

    static void initializeResultValues( std::vector<float>& resultValues, size_t resultCount );
    static void initializeResultValues( std::vector<caf::Ten3d>& resultValues, size_t resultCount );

    void smoothSegments( std::vector<double>*              mds,
                         std::vector<double>*              tvds,
                         std::vector<double>*              values,
                         const std::vector<double>&        interfaceShValues,
                         const std::vector<unsigned char>& smoothSegments,
                         const double                      smoothingThreshold );

    std::vector<unsigned char> determineFilteringOrSmoothing( const std::vector<double>& porePressures );

    double hydroStaticPorePressureForIntersection( size_t intersectionIdx, double waterDensityGCM3 = PURE_WATER_DENSITY_GCM3 ) const;
    double hydroStaticPorePressureForSegment( size_t intersectionIdx, double waterDensityGCM3 = PURE_WATER_DENSITY_GCM3 ) const;

    double wbsCurveValuesAtMsl() const;

    static bool isValid( double value );
    static bool isValid( float value );

    double calculateWaterDepth() const;

private:
    cvf::ref<RigGeoMechCaseData> m_caseData;

    std::map<RigWbsParameter, std::vector<std::pair<double, double>>> m_lasFileValues;
    std::map<RigWbsParameter, QString>                                m_lasFileInputUnits;
    std::map<RigWbsParameter, WbsParameterSource>                     m_parameterSources;
    std::map<RigWbsParameter, double>                                 m_userDefinedValues;

    double m_waterDepth;
    int    m_partId;
    bool   m_valid;
};
