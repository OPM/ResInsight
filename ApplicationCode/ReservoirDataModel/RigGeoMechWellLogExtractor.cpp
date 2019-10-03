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

//==================================================================================================
///
//==================================================================================================
#include "RigGeoMechWellLogExtractor.h"

#include "RiaDefines.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemTypes.h"
#include "RigGeoMechBoreHoleStressCalculator.h"
#include "RigGeoMechCaseData.h"

#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"
#include "RigWellPathIntersectionTools.h"

#include "cafTensor3.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"

#include <QPolygonF>

#include <type_traits>

const double RigGeoMechWellLogExtractor::UNIT_WEIGHT_OF_WATER = 9.81 * 1000.0; // N / m^3

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::RigGeoMechWellLogExtractor( RigGeoMechCaseData* aCase,
                                                        const RigWellPath*  wellpath,
                                                        const std::string&  wellCaseErrorMsgName )
    : RigWellLogExtractor( wellpath, wellCaseErrorMsgName )
    , m_caseData( aCase )
{
    calculateIntersection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::smoothCurveData( int                  frameIndex,
                                                  std::vector<double>* mds,
                                                  std::vector<double>* tvds,
                                                  std::vector<double>* values,
                                                  const double         smoothingTreshold )
{
    CVF_ASSERT( mds && tvds && values );

    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    RigFemResultAddress shAddr( RIG_ELEMENT_NODAL, "ST", "S3" );
    RigFemResultAddress porBarResAddr( RIG_ELEMENT_NODAL, "POR-Bar", "" );

    const std::vector<float>& unscaledShValues = resultCollection->resultValues( shAddr, 0, frameIndex );
    const std::vector<float>& porePressures    = resultCollection->resultValues( porBarResAddr, 0, frameIndex );

    std::vector<float> interfaceShValues      = interpolateInterfaceValues( shAddr, unscaledShValues );
    std::vector<float> interfacePorePressures = interpolateInterfaceValues( porBarResAddr, porePressures );

    std::vector<double> interfaceShValuesDbl( interfaceShValues.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> interfacePorePressuresDbl( interfacePorePressures.size(),
                                                   std::numeric_limits<double>::infinity() );
#pragma omp parallel for
    for ( int64_t i = 0; i < int64_t( m_intersections.size() ); ++i )
    {
        cvf::Vec3f centroid = cellCentroid( i );

        double trueVerticalDepth = -centroid.z();

        double effectiveDepthMeters       = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressureBar = pascalToBar( effectiveDepthMeters * UNIT_WEIGHT_OF_WATER );
        interfaceShValuesDbl[i]           = interfaceShValues[i] / hydroStaticPorePressureBar;
        interfacePorePressuresDbl[i]      = interfacePorePressures[i];
    }

    std::vector<std::vector<double>*> dependentValues = {tvds, &interfaceShValuesDbl, &interfacePorePressuresDbl};

    std::vector<unsigned char> smoothOrFilterSegments = determineFilteringOrSmoothing( interfacePorePressuresDbl );
    filterShortSegments( mds, values, &smoothOrFilterSegments, dependentValues );
    filterColinearSegments( mds, values, &smoothOrFilterSegments, dependentValues );

    smoothSegments( mds, tvds, values, interfaceShValuesDbl, smoothOrFilterSegments, smoothingTreshold );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::curveData( const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values )
{
    CVF_TIGHT_ASSERT( values );

    if ( resAddr.resultPosType == RIG_WELLPATH_DERIVED )
    {
        if ( resAddr.fieldName == RiaDefines::wbsFGResultName().toStdString() ||
             resAddr.fieldName == RiaDefines::wbsSFGResultName().toStdString() )
        {
            wellBoreWallCurveData( resAddr, frameIndex, values );
            return;
        }
        else if ( resAddr.fieldName == RiaDefines::wbsPoissonParameterName().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsUCSParameterName().toStdString() )
        {
            wellPathParameters( resAddr, frameIndex, values );
        }
        else if ( resAddr.fieldName == RiaDefines::wbsPPResultName().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsOBGResultName().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsSHResultName().toStdString() )
        {
            wellPathScaledCurveData( resAddr, frameIndex, values );
            return;
        }
        else if ( resAddr.fieldName == RiaDefines::wbsAzimuthResultName().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsInclinationResultName().toStdString() )
        {
            wellPathAngles( resAddr, values );
            return;
        }
    }

    if ( !resAddr.isValid() ) return;

    RigFemResultAddress convResAddr = resAddr;

    // When showing POR results, always use the element nodal result,
    // to get correct handling of elements without POR results

    if ( convResAddr.fieldName == "POR-Bar" ) convResAddr.resultPosType = RIG_ELEMENT_NODAL;

    CVF_ASSERT( resAddr.resultPosType != RIG_WELLPATH_DERIVED );

    const std::vector<float>& resultValues = m_caseData->femPartResults()->resultValues( convResAddr, 0, frameIndex );

    if ( !resultValues.size() ) return;

    values->resize( m_intersections.size() );

    for ( size_t intersectionIdx = 0; intersectionIdx < m_intersections.size(); ++intersectionIdx )
    {
        ( *values )[intersectionIdx] = static_cast<double>(
            interpolateGridResultValue<float>( convResAddr.resultPosType, resultValues, intersectionIdx ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<float, RigGeoMechWellLogExtractor::WbsParameterSource> RigGeoMechWellLogExtractor::calculatePorePressureInSegment(
    int64_t                   intersectionIdx,
    double                    effectiveDepthMeters,
    const std::vector<float>& interpolatedInterfacePorePressuresBar,
    const std::vector<float>& poreElementPressuresPascal ) const
{
    // Priority 1: Try pore pressure from the grid
    if ( m_porePressureSource == AUTO || m_porePressureSource == GRID )
    {
        float averagePorePressureBar = std::numeric_limits<float>::infinity();
        bool  validGridPorePressure  = averageIntersectionValuesToSegmentValue( intersectionIdx,
                                                                              interpolatedInterfacePorePressuresBar,
                                                                              std::numeric_limits<float>::infinity(),
                                                                              &averagePorePressureBar );
        if ( validGridPorePressure )
        {
            return std::make_pair( averagePorePressureBar, GRID );
        }
    }

    // Priority 2: Try LAS-file
    if ( m_porePressureSource == AUTO || m_porePressureSource == LAS_FILE )
    {
        double lasMudWeightKgPerM3 = getWellLogSegmentValue( intersectionIdx, m_wellLogMdAndMudWeightKgPerM3 );
        if ( lasMudWeightKgPerM3 != std::numeric_limits<double>::infinity() )
        {
            double specificMudWeightNPerM3 = lasMudWeightKgPerM3 * 9.81;
            double porePressurePascal      = specificMudWeightNPerM3 * effectiveDepthMeters;
            double porePressureBar         = pascalToBar( porePressurePascal );
            return std::make_pair( (float)porePressureBar, LAS_FILE );
        }
    }

    // Priority 3: Try element property tables
    if ( m_porePressureSource == AUTO || m_porePressureSource == ELEMENT_PROPERTY_TABLE )
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        if ( elmIdx < poreElementPressuresPascal.size() )
        {
            float porePressureBar = pascalToBar( poreElementPressuresPascal[elmIdx] );
            return std::make_pair( porePressureBar, ELEMENT_PROPERTY_TABLE );
        }
    }

    return std::make_pair( std::numeric_limits<float>::infinity(), INVALID );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<float, RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::calculatePoissonRatioInSegment( int64_t                   intersectionIdx,
                                                                const std::vector<float>& poissonRatios ) const
{
    // Priority 1: Las-file poisson ratio
    if ( m_poissonRatioSource == AUTO || m_poissonRatioSource == LAS_FILE )
    {
        if ( !m_wellLogMdAndPoissonRatios.empty() )
        {
            double lasPoissionRatio = getWellLogSegmentValue( intersectionIdx, m_wellLogMdAndPoissonRatios );
            if ( lasPoissionRatio != std::numeric_limits<double>::infinity() )
            {
                return std::make_pair( lasPoissionRatio, LAS_FILE );
            }
        }
    }

    // Priority 2: Element property table ratio
    if ( m_poissonRatioSource == AUTO || m_poissonRatioSource == ELEMENT_PROPERTY_TABLE )
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        if ( elmIdx < poissonRatios.size() )
        {
            return std::make_pair( poissonRatios[elmIdx], ELEMENT_PROPERTY_TABLE );
        }
    }

    // Priority 3: User defined poisson ratio
    return std::make_pair( (float)m_userDefinedPoissonRatio, USER_DEFINED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<float, RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::calculateUcsInSegment( int64_t                   intersectionIdx,
                                                       const std::vector<float>& ucsValuesPascal ) const
{
    if ( m_ucsSource == AUTO || m_ucsSource == LAS_FILE )
    {
        if ( !m_wellLogMdAndUcsBar.empty() )
        {
            double lasUniaxialStrengthInBar = getWellLogSegmentValue( intersectionIdx, m_wellLogMdAndUcsBar );
            if ( lasUniaxialStrengthInBar != std::numeric_limits<double>::infinity() )
            {
                return std::make_pair( lasUniaxialStrengthInBar, LAS_FILE );
            }
        }
    }
    // Priority 2: From element property table
    if ( m_ucsSource == AUTO || m_ucsSource == ELEMENT_PROPERTY_TABLE )
    {
        size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
        if ( elmIdx < ucsValuesPascal.size() )
        {
            // Read UCS from element table in Pascal
            float uniaxialStrengthInBar = pascalToBar( ucsValuesPascal[elmIdx] );
            return std::make_pair( uniaxialStrengthInBar, ELEMENT_PROPERTY_TABLE );
        }
    }
    // Priority 3: User defined UCS (in bar)
    return std::make_pair( m_userDefinedUcs, USER_DEFINED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellPathAngles( const RigFemResultAddress& resAddr, std::vector<double>* values )
{
    CVF_ASSERT( values );
    CVF_ASSERT( resAddr.fieldName == "Azimuth" || resAddr.fieldName == "Inclination" );
    values->resize( m_intersections.size(), 0.0f );
    const double     epsilon = 1.0e-6 * 360;
    const cvf::Vec3d trueNorth( 0.0, 1.0, 0.0 );
    const cvf::Vec3d up( 0.0, 0.0, 1.0 );
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        cvf::Vec3d wellPathTangent = calculateWellPathTangent( intersectionIdx, TangentFollowWellPathSegments );

        // Deviation from vertical. Since well path is tending downwards we compare with negative z.
        double inclination = cvf::Math::toDegrees(
            std::acos( cvf::Vec3d( 0.0, 0.0, -1.0 ) * wellPathTangent.getNormalized() ) );

        if ( resAddr.fieldName == "Azimuth" )
        {
            double azimuth = HUGE_VAL;

            // Azimuth is not defined when well path is vertical. We define it as infinite to avoid it showing up in the plot.
            if ( cvf::Math::valueInRange( inclination, epsilon, 180.0 - epsilon ) )
            {
                cvf::Vec3d projectedTangentXY = wellPathTangent;
                projectedTangentXY.z()        = 0.0;

                // Do tangentXY to true north for clockwise angles.
                double dotProduct   = projectedTangentXY * trueNorth;
                double crossProduct = ( projectedTangentXY ^ trueNorth ) * up;
                // http://www.glossary.oilfield.slb.com/Terms/a/azimuth.aspx
                azimuth = cvf::Math::toDegrees( std::atan2( crossProduct, dotProduct ) );
                if ( azimuth < 0.0 )
                {
                    // Straight atan2 gives angle from -PI to PI yielding angles from -180 to 180
                    // where the negative angles are counter clockwise.
                    // To get all positive clockwise angles, we add 360 degrees to negative angles.
                    azimuth = azimuth + 360.0;
                }
            }

            ( *values )[intersectionIdx] = azimuth;
        }
        else
        {
            ( *values )[intersectionIdx] = inclination;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellPathScaledCurveData( const RigFemResultAddress& resAddr,
                                                          int                        frameIndex,
                                                          std::vector<double>*       values )
{
    CVF_ASSERT( values );

    const RigFemPart*            femPart          = m_caseData->femParts()->part( 0 );
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::string nativeFieldName;
    std::string nativeCompName;
    if ( resAddr.fieldName == "PP" )
    {
        nativeFieldName = "POR-Bar"; // More likely to be in memory than POR
    }
    else if ( resAddr.fieldName == "OBG" )
    {
        nativeFieldName = "ST";
        nativeCompName  = "S33";
    }
    else if ( resAddr.fieldName == "SH" )
    {
        nativeFieldName = "ST";
        nativeCompName  = "S3";
    }

    RigFemResultAddress nativeAddr( RIG_ELEMENT_NODAL, nativeFieldName, nativeCompName );
    RigFemResultAddress porElementResAddr( RIG_ELEMENT, "POR", "" );

    const std::vector<float>& unscaledResultValues       = resultCollection->resultValues( nativeAddr, 0, frameIndex );
    const std::vector<float>& poreElementPressuresPascal = resultCollection->resultValues( porElementResAddr,
                                                                                           0,
                                                                                           frameIndex );

    std::vector<float> interpolatedInterfaceValues = interpolateInterfaceValues( nativeAddr, unscaledResultValues );

    values->resize( m_intersections.size(), 0.0f );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        // Set the value to invalid by default
        ( *values )[intersectionIdx] = std::numeric_limits<double>::infinity();

        size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType( elmIdx );

        if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

        cvf::Vec3f centroid = cellCentroid( intersectionIdx );

        double trueVerticalDepth = -centroid.z();

        double effectiveDepthMeters       = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressureBar = pascalToBar( effectiveDepthMeters * UNIT_WEIGHT_OF_WATER );

        float averageUnscaledValue = std::numeric_limits<float>::infinity();

        if ( resAddr.fieldName == "PP" )
        {
            auto ppSourcePair = calculatePorePressureInSegment( intersectionIdx,
                                                                effectiveDepthMeters,
                                                                interpolatedInterfaceValues,
                                                                poreElementPressuresPascal );
            if ( ppSourcePair.second == INVALID )
            {
                averageUnscaledValue = hydroStaticPorePressureBar;
            }
            else
            {
                averageUnscaledValue = ppSourcePair.first;
            }
        }

        ( *values )[intersectionIdx] = static_cast<double>( averageUnscaledValue ) / hydroStaticPorePressureBar;
    }

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellBoreWallCurveData( const RigFemResultAddress& resAddr,
                                                        int                        frameIndex,
                                                        std::vector<double>*       values )
{
    CVF_ASSERT( values );
    CVF_ASSERT( resAddr.fieldName == RiaDefines::wbsFGResultName().toStdString() ||
                resAddr.fieldName == RiaDefines::wbsSFGResultName().toStdString() );

    // The result addresses needed
    RigFemResultAddress stressResAddr( RIG_ELEMENT_NODAL, "ST", "" );
    RigFemResultAddress porBarResAddr( RIG_ELEMENT_NODAL, "POR-Bar", "" );
    // Allow POR as an element property value
    RigFemResultAddress porElementResAddr( RIG_ELEMENT, "POR", "" );
    RigFemResultAddress poissonResAddr( RIG_ELEMENT, "RATIO", "" );
    RigFemResultAddress ucsResAddr( RIG_ELEMENT, "UCS", "" );

    const RigFemPart*            femPart          = m_caseData->femParts()->part( 0 );
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    // Load results
    std::vector<caf::Ten3f> vertexStressesFloat = resultCollection->tensors( stressResAddr, 0, frameIndex );
    if ( !vertexStressesFloat.size() ) return;

    std::vector<caf::Ten3d> vertexStresses;
    vertexStresses.reserve( vertexStressesFloat.size() );
    for ( const caf::Ten3f& floatTensor : vertexStressesFloat )
    {
        vertexStresses.push_back( caf::Ten3d( floatTensor ) );
    }
    std::vector<float> porePressures              = resultCollection->resultValues( porBarResAddr, 0, frameIndex );
    std::vector<float> poreElementPressuresPascal = resultCollection->resultValues( porElementResAddr, 0, frameIndex );
    std::vector<float> poissonRatios              = resultCollection->resultValues( poissonResAddr, 0, frameIndex );
    std::vector<float> ucsValuesPascal            = resultCollection->resultValues( ucsResAddr, 0, frameIndex );

    std::vector<float> interpolatedInterfacePorePressuresBar = interpolateInterfaceValues<float>( porBarResAddr,
                                                                                                  porePressures );

    std::vector<caf::Ten3d> interpolatedInterfaceStressBar = interpolateInterfaceValues<caf::Ten3d>( stressResAddr,
                                                                                                     vertexStresses );

    values->resize( m_intersections.size(), 0.0f );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType( elmIdx );

        if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

        cvf::Vec3f centroid = cellCentroid( intersectionIdx );

        double trueVerticalDepth          = -centroid.z();
        double effectiveDepthMeters       = trueVerticalDepth + m_rkbDiff;
        double hydroStaticPorePressureBar = pascalToBar( effectiveDepthMeters * UNIT_WEIGHT_OF_WATER );

        auto   ppSourcePair    = calculatePorePressureInSegment( intersectionIdx,
                                                            effectiveDepthMeters,
                                                            interpolatedInterfacePorePressuresBar,
                                                            poreElementPressuresPascal );
        double porePressureBar = ppSourcePair.first;

        // FG is for sands, SFG for shale. Sands has valid PP, shale does not.
        bool isFGregion = ppSourcePair.second != INVALID;

        if ( ppSourcePair.second == INVALID )
        {
            porePressureBar = hydroStaticPorePressureBar;
        }

        double poissonRatio = calculatePoissonRatioInSegment( intersectionIdx, poissonRatios ).first;
        double ucsBar       = calculateUcsInSegment( intersectionIdx, ucsValuesPascal ).first;

        caf::Ten3d segmentStress;
        bool       validSegmentStress = averageIntersectionValuesToSegmentValue( intersectionIdx,
                                                                           interpolatedInterfaceStressBar,
                                                                           caf::Ten3d::invalid(),
                                                                           &segmentStress );

        cvf::Vec3d wellPathTangent     = calculateWellPathTangent( intersectionIdx, TangentConstantWithinCell );
        caf::Ten3d wellPathStressFloat = transformTensorToWellPathOrientation( wellPathTangent, segmentStress );
        caf::Ten3d wellPathStressDouble( wellPathStressFloat );

        RigGeoMechBoreHoleStressCalculator sigmaCalculator( wellPathStressDouble, porePressureBar, poissonRatio, ucsBar, 32 );
        double                             resultValue = std::numeric_limits<double>::infinity();
        if ( resAddr.fieldName == RiaDefines::wbsFGResultName().toStdString() )
        {
            if ( isFGregion && validSegmentStress )
            {
                resultValue = sigmaCalculator.solveFractureGradient();
            }
        }
        else
        {
            CVF_ASSERT( resAddr.fieldName == RiaDefines::wbsSFGResultName().toStdString() );
            if ( !isFGregion && validSegmentStress )
            {
                resultValue = sigmaCalculator.solveStassiDalia();
            }
        }
        if ( resultValue != std::numeric_limits<double>::infinity() )
        {
            if ( hydroStaticPorePressureBar > 1.0e-8 )
            {
                resultValue /= hydroStaticPorePressureBar;
            }
        }
        ( *values )[intersectionIdx] = resultValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellPathParameters( const RigFemResultAddress& resAddr,
                                                     int                        frameIndex,
                                                     std::vector<double>*       values )
{
    CVF_ASSERT( values );
    CVF_ASSERT( resAddr.fieldName == RiaDefines::wbsPoissonParameterName().toStdString() ||
                resAddr.fieldName == RiaDefines::wbsUCSParameterName().toStdString() );

    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    // Check for element property values
    RigFemResultAddress elmResAddr( RIG_ELEMENT, resAddr.fieldName, "" );
    std::vector<float>  elmPropertyValues = resultCollection->resultValues( elmResAddr, 0, frameIndex );

    values->resize( m_intersections.size(), 0.0f );

    if ( resAddr.fieldName == RiaDefines::wbsPoissonParameterName().toStdString() )
    {
#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            ( *values )[intersectionIdx] = calculatePoissonRatioInSegment( intersectionIdx, elmPropertyValues ).first;
        }
    }
    else
    {
#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            ( *values )[intersectionIdx] = calculateUcsInSegment( intersectionIdx, elmPropertyValues ).first / 100.0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGeoMechCaseData* RigGeoMechWellLogExtractor::caseData()
{
    return m_caseData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setRkbDiff( double rkbDiff )
{
    m_rkbDiff = rkbDiff;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWellLogMdAndMudWeightKgPerM3( const std::vector<std::pair<double, double>>& porePressures )
{
    m_wellLogMdAndMudWeightKgPerM3 = porePressures;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWellLogMdAndUcsBar( const std::vector<std::pair<double, double>>& ucsValues )
{
    m_wellLogMdAndUcsBar = ucsValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWellLogMdAndPoissonRatio( const std::vector<std::pair<double, double>>& poissonRatios )
{
    m_wellLogMdAndPoissonRatios = poissonRatios;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RigGeoMechWellLogExtractor::supportedSourcesForPorePressure()
{
    return {AUTO, GRID, LAS_FILE, ELEMENT_PROPERTY_TABLE, HYDROSTATIC_PP};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RigGeoMechWellLogExtractor::supportedSourcesForPoissonRatio()
{
    return {AUTO, LAS_FILE, ELEMENT_PROPERTY_TABLE, USER_DEFINED};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigGeoMechWellLogExtractor::WbsParameterSource> RigGeoMechWellLogExtractor::supportedSourcesForUcs()
{
    return {AUTO, LAS_FILE, ELEMENT_PROPERTY_TABLE, USER_DEFINED};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWbsParameters( WbsParameterSource porePressureSource,
                                                   WbsParameterSource poissonRatioSource,
                                                   WbsParameterSource ucsSource,
                                                   double             userDefinedPoissonRatio,
                                                   double             userDefinedUcs )
{
    m_porePressureSource      = porePressureSource;
    m_poissonRatioSource      = poissonRatioSource;
    m_ucsSource               = ucsSource;
    m_userDefinedPoissonRatio = userDefinedPoissonRatio;
    m_userDefinedUcs          = userDefinedUcs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::porePressureIntervals( int frameIndex )
{
    std::vector<double> ppValues( m_intersections.size(), 0.0 );

    RigFemResultAddress porBarResAddr( RIG_ELEMENT_NODAL, "POR-Bar", "" );
    RigFemResultAddress porElementResAddr( RIG_ELEMENT, "POR", "" );

    const RigFemPart*            femPart          = m_caseData->femParts()->part( 0 );
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    const std::vector<float>& porePressures = resultCollection->resultValues( porBarResAddr, 0, frameIndex );
    const std::vector<float>& poreElementPressuresPascal = resultCollection->resultValues( porElementResAddr,
                                                                                           0,
                                                                                           frameIndex );

    std::vector<float> interpolatedInterfacePorePressureBar;
    interpolatedInterfacePorePressureBar.resize( m_intersections.size(), std::numeric_limits<float>::infinity() );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType( elmIdx );
        if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

        interpolatedInterfacePorePressureBar[intersectionIdx] = interpolateGridResultValue( porBarResAddr.resultPosType,
                                                                                            porePressures,
                                                                                            intersectionIdx );
    }

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        auto ppSourcePair = calculatePorePressureInSegment( intersectionIdx,
                                                            0.0,
                                                            interpolatedInterfacePorePressureBar,
                                                            poreElementPressuresPascal );
        if ( ppSourcePair.second == INVALID )
        {
            ppValues[intersectionIdx] = static_cast<double>( HYDROSTATIC_PP );
        }
        else
        {
            ppValues[intersectionIdx] = static_cast<double>( ppSourcePair.second );
        }
    }
    return ppValues;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::poissonIntervals( int frameIndex )
{
    std::vector<double> poissonValues( m_intersections.size(), 0.0 );

    RigFemResultAddress poissonResAddr( RIG_ELEMENT, "RATIO", "" );

    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::vector<float> poissonRatios = resultCollection->resultValues( poissonResAddr, 0, frameIndex );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        auto poissonSourcePair         = calculatePoissonRatioInSegment( intersectionIdx, poissonRatios );
        poissonValues[intersectionIdx] = static_cast<double>( poissonSourcePair.second );
    }
    return poissonValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::ucsIntervals( int frameIndex )
{
    std::vector<double> ucsValues( m_intersections.size(), 0.0 );

    RigFemResultAddress ucsResAddr( RIG_ELEMENT, "UCS", "" );

    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::vector<float> ucsValuesPascal = resultCollection->resultValues( ucsResAddr, 0, frameIndex );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        auto ucsSourcePair         = calculateUcsInSegment( intersectionIdx, ucsValuesPascal );
        ucsValues[intersectionIdx] = static_cast<double>( ucsSourcePair.second );
    }
    return ucsValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
T RigGeoMechWellLogExtractor::interpolateGridResultValue( RigFemResultPosEnum   resultPosType,
                                                          const std::vector<T>& gridResultValues,
                                                          int64_t               intersectionIdx ) const
{
    const RigFemPart*              femPart    = m_caseData->femParts()->part( 0 );
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
    RigElementType elmType = femPart->elementType( elmIdx );

    if ( !( elmType == HEX8 || elmType == HEX8P ) ) return T();

    if ( resultPosType == RIG_FORMATION_NAMES )
    {
        resultPosType = RIG_ELEMENT_NODAL; // formation indices are stored per element node result.
    }

    if ( resultPosType == RIG_ELEMENT )
    {
        return gridResultValues[elmIdx];
    }

    cvf::StructGridInterface::FaceType cellFace = m_intersectedCellFaces[intersectionIdx];

    if ( cellFace == cvf::StructGridInterface::NO_FACE )
    {
        if ( resultPosType == RIG_ELEMENT_NODAL_FACE )
        {
            return std::numeric_limits<T>::infinity(); // undefined value. ELEMENT_NODAL_FACE values are only defined on a face.
        }
        // TODO: Should interpolate within the whole hexahedron. This requires converting to locals coordinates.
        // For now just pick the average value for the cell.
        size_t gridResultValueIdx = femPart->resultValueIdxFromResultPosType( resultPosType,
                                                                              static_cast<int>( elmIdx ),
                                                                              0 );
        T      sumOfVertexValues  = gridResultValues[gridResultValueIdx];
        for ( int i = 1; i < 8; ++i )
        {
            gridResultValueIdx = femPart->resultValueIdxFromResultPosType( resultPosType, static_cast<int>( elmIdx ), i );
            sumOfVertexValues = sumOfVertexValues + gridResultValues[gridResultValueIdx];
        }
        return sumOfVertexValues * ( 1.0 / 8.0 );
    }

    int        faceNodeCount              = 0;
    const int* elementLocalIndicesForFace = RigFemTypes::localElmNodeIndicesForFace( elmType, cellFace, &faceNodeCount );
    const int* elmNodeIndices             = femPart->connectivities( elmIdx );

    cvf::Vec3d v0( nodeCoords[elmNodeIndices[elementLocalIndicesForFace[0]]] );
    cvf::Vec3d v1( nodeCoords[elmNodeIndices[elementLocalIndicesForFace[1]]] );
    cvf::Vec3d v2( nodeCoords[elmNodeIndices[elementLocalIndicesForFace[2]]] );
    cvf::Vec3d v3( nodeCoords[elmNodeIndices[elementLocalIndicesForFace[3]]] );

    std::vector<size_t> nodeResIdx( 4, cvf::UNDEFINED_SIZE_T );

    for ( size_t i = 0; i < nodeResIdx.size(); ++i )
    {
        if ( resultPosType == RIG_ELEMENT_NODAL_FACE )
        {
            nodeResIdx[i] = gridResultIndexFace( elmIdx, cellFace, static_cast<int>( i ) );
        }
        else
        {
            nodeResIdx[i] = femPart->resultValueIdxFromResultPosType( resultPosType,
                                                                      static_cast<int>( elmIdx ),
                                                                      elementLocalIndicesForFace[i] );
        }
    }

    std::vector<T> nodeResultValues;
    nodeResultValues.reserve( 4 );
    for ( size_t i = 0; i < nodeResIdx.size(); ++i )
    {
        nodeResultValues.push_back( gridResultValues[nodeResIdx[i]] );
    }
    T interpolatedValue = cvf::GeometryTools::interpolateQuad<T>( v0,
                                                                  nodeResultValues[0],
                                                                  v1,
                                                                  nodeResultValues[1],
                                                                  v2,
                                                                  nodeResultValues[2],
                                                                  v3,
                                                                  nodeResultValues[3],
                                                                  m_intersections[intersectionIdx] );

    return interpolatedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigGeoMechWellLogExtractor::gridResultIndexFace( size_t                             elementIdx,
                                                        cvf::StructGridInterface::FaceType cellFace,
                                                        int                                faceLocalNodeIdx ) const
{
    CVF_ASSERT( cellFace != cvf::StructGridInterface::NO_FACE && faceLocalNodeIdx < 4 );
    return elementIdx * 24 + static_cast<int>( cellFace ) * 4 + faceLocalNodeIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::calculateIntersection()
{
    CVF_ASSERT( m_caseData->femParts()->partCount() == 1 );

    std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo> uniqueIntersections;

    const RigFemPart*              femPart    = m_caseData->femParts()->part( 0 );
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    for ( size_t wpp = 0; wpp < m_wellPath->m_wellPathPoints.size() - 1; ++wpp )
    {
        std::vector<HexIntersectionInfo> intersections;
        cvf::Vec3d                       p1 = m_wellPath->m_wellPathPoints[wpp];
        cvf::Vec3d                       p2 = m_wellPath->m_wellPathPoints[wpp + 1];

        cvf::BoundingBox bb;

        bb.add( p1 );
        bb.add( p2 );

        std::vector<size_t> closeCells = findCloseCells( bb );

        cvf::Vec3d hexCorners[8];
        for ( size_t ccIdx = 0; ccIdx < closeCells.size(); ++ccIdx )
        {
            RigElementType elmType = femPart->elementType( closeCells[ccIdx] );
            if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

            const int* cornerIndices = femPart->connectivities( closeCells[ccIdx] );

            hexCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
            hexCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
            hexCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
            hexCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
            hexCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
            hexCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
            hexCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
            hexCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

            // int intersectionCount = RigHexIntersector::lineHexCellIntersection(p1, p2, hexCorners,
            // closeCells[ccIdx], &intersections);
            RigHexIntersectionTools::lineHexCellIntersection( p1, p2, hexCorners, closeCells[ccIdx], &intersections );
        }

        // Now, with all the intersections of this piece of line, we need to
        // sort them in order, and set the measured depth and corresponding cell index

        // Inserting the intersections in this map will remove identical intersections
        // and sort them according to MD, CellIdx, Leave/enter

        double md1 = m_wellPath->m_measuredDepths[wpp];
        double md2 = m_wellPath->m_measuredDepths[wpp + 1];

        insertIntersectionsInMap( intersections, p1, md1, p2, md2, &uniqueIntersections );
    }

    this->populateReturnArrays( uniqueIntersections );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigGeoMechWellLogExtractor::findCloseCells( const cvf::BoundingBox& bb )
{
    std::vector<size_t> closeCells;

    if ( m_caseData->femParts()->partCount() )
    {
        m_caseData->femParts()->part( 0 )->findIntersectingCells( bb, &closeCells );
    }
    return closeCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGeoMechWellLogExtractor::calculateLengthInCell( size_t            cellIndex,
                                                              const cvf::Vec3d& startPoint,
                                                              const cvf::Vec3d& endPoint ) const
{
    std::array<cvf::Vec3d, 8> hexCorners;

    const RigFemPart*              femPart       = m_caseData->femParts()->part( 0 );
    const std::vector<cvf::Vec3f>& nodeCoords    = femPart->nodes().coordinates;
    const int*                     cornerIndices = femPart->connectivities( cellIndex );

    hexCorners[0] = cvf::Vec3d( nodeCoords[cornerIndices[0]] );
    hexCorners[1] = cvf::Vec3d( nodeCoords[cornerIndices[1]] );
    hexCorners[2] = cvf::Vec3d( nodeCoords[cornerIndices[2]] );
    hexCorners[3] = cvf::Vec3d( nodeCoords[cornerIndices[3]] );
    hexCorners[4] = cvf::Vec3d( nodeCoords[cornerIndices[4]] );
    hexCorners[5] = cvf::Vec3d( nodeCoords[cornerIndices[5]] );
    hexCorners[6] = cvf::Vec3d( nodeCoords[cornerIndices[6]] );
    hexCorners[7] = cvf::Vec3d( nodeCoords[cornerIndices[7]] );

    return RigWellPathIntersectionTools::calculateLengthInCell( hexCorners, startPoint, endPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigGeoMechWellLogExtractor::calculateWellPathTangent( int64_t                    intersectionIdx,
                                                                 WellPathTangentCalculation calculationType ) const
{
    if ( calculationType == TangentFollowWellPathSegments )
    {
        cvf::Vec3d segmentStart, segmentEnd;
        m_wellPath->twoClosestPoints( m_intersections[intersectionIdx], &segmentStart, &segmentEnd );
        return ( segmentEnd - segmentStart ).getNormalized();
    }
    else
    {
        cvf::Vec3d wellPathTangent;
        if ( intersectionIdx % 2 == 0 )
        {
            wellPathTangent = m_intersections[intersectionIdx + 1] - m_intersections[intersectionIdx];
        }
        else
        {
            wellPathTangent = m_intersections[intersectionIdx] - m_intersections[intersectionIdx - 1];
        }
        CVF_ASSERT( wellPathTangent.length() > 1.0e-7 );
        return wellPathTangent.getNormalized();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Ten3d RigGeoMechWellLogExtractor::transformTensorToWellPathOrientation( const cvf::Vec3d& wellPathTangent,
                                                                             const caf::Ten3d& tensor )
{
    // Create local coordinate system for well path segment
    cvf::Vec3d local_z = wellPathTangent;
    cvf::Vec3d local_x = local_z.perpendicularVector().getNormalized();
    cvf::Vec3d local_y = ( local_z ^ local_x ).getNormalized();
    // Calculate the rotation matrix from global i, j, k to local x, y, z.
    cvf::Mat4d rotationMatrix = cvf::Mat4d::fromCoordSystemAxes( &local_x, &local_y, &local_z );

    return tensor.rotated( rotationMatrix.toMatrix3() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RigGeoMechWellLogExtractor::cellCentroid( size_t intersectionIdx ) const
{
    const RigFemPart*              femPart    = m_caseData->femParts()->part( 0 );
    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;

    size_t         elmIdx           = m_intersectedCellsGlobIdx[intersectionIdx];
    RigElementType elmType          = femPart->elementType( elmIdx );
    int            elementNodeCount = RigFemTypes::elmentNodeCount( elmType );

    const int* elmNodeIndices = femPart->connectivities( elmIdx );

    cvf::Vec3f centroid( 0.0, 0.0, 0.0 );
    for ( int i = 0; i < elementNodeCount; ++i )
    {
        centroid += nodeCoords[elmNodeIndices[i]];
    }
    return centroid / elementNodeCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::getWellLogSegmentValue( size_t intersectionIdx,
                                                           const std::vector<std::pair<double, double>>& wellLogValues ) const
{
    if ( !wellLogValues.empty() )
    {
        double startMD, endMD;
        if ( intersectionIdx % 2 == 0 )
        {
            startMD = m_intersectionMeasuredDepths[intersectionIdx];
            endMD   = m_intersectionMeasuredDepths[intersectionIdx + 1];
        }
        else
        {
            startMD = m_intersectionMeasuredDepths[intersectionIdx - 1];
            endMD   = m_intersectionMeasuredDepths[intersectionIdx];
        }

        RiaWeightedMeanCalculator<double> averageCalc;
        for ( auto& depthAndValue : wellLogValues )
        {
            if ( cvf::Math::valueInRange( depthAndValue.first, startMD, endMD ) )
            {
                cvf::Vec3d position = m_wellPath->interpolatedPointAlongWellPath( depthAndValue.first );
                cvf::Vec3d centroid( cellCentroid( intersectionIdx ) );
                double     weight = 1.0;
                double     dist   = ( position - centroid ).length();
                if ( dist > 1.0 )
                {
                    weight = 1.0 / dist;
                }
                averageCalc.addValueAndWeight( depthAndValue.second, weight );
            }
        }
        if ( averageCalc.validAggregatedWeight() )
        {
            return averageCalc.weightedMean();
        }
    }
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::pascalToBar( double pascalValue )
{
    return pascalValue * 1.0e-5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
bool RigGeoMechWellLogExtractor::averageIntersectionValuesToSegmentValue( size_t                intersectionIdx,
                                                                          const std::vector<T>& values,
                                                                          const T&              invalidValue,
                                                                          T* averagedCellValue ) const
{
    CVF_ASSERT( values.size() >= 2 );

    *averagedCellValue = invalidValue;

    T          value1, value2;
    cvf::Vec3d centroid( cellCentroid( intersectionIdx ) );
    double     dist1 = 0.0, dist2 = 0.0;
    if ( intersectionIdx % 2 == 0 )
    {
        value1 = values[intersectionIdx];
        value2 = values[intersectionIdx + 1];

        dist1 = ( centroid - m_intersections[intersectionIdx] ).length();
        dist2 = ( centroid - m_intersections[intersectionIdx + 1] ).length();
    }
    else
    {
        value1 = values[intersectionIdx - 1];
        value2 = values[intersectionIdx];

        dist1 = ( centroid - m_intersections[intersectionIdx - 1] ).length();
        dist2 = ( centroid - m_intersections[intersectionIdx] ).length();
    }

    if ( invalidValue == value1 || invalidValue == value2 )
    {
        return false;
    }

    RiaWeightedMeanCalculator<T> averageCalc;
    averageCalc.addValueAndWeight( value1, dist2 );
    averageCalc.addValueAndWeight( value2, dist1 );
    if ( averageCalc.validAggregatedWeight() )
    {
        *averagedCellValue = averageCalc.weightedMean();
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> RigGeoMechWellLogExtractor::interpolateInterfaceValues( RigFemResultAddress   nativeAddr,
                                                                       const std::vector<T>& unscaledResultValues ) const
{
    std::vector<T> interpolatedInterfaceValues;
    initializeResultValues( interpolatedInterfaceValues, m_intersections.size() );

    const RigFemPart* femPart = m_caseData->femParts()->part( 0 );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        size_t         elmIdx  = m_intersectedCellsGlobIdx[intersectionIdx];
        RigElementType elmType = femPart->elementType( elmIdx );
        if ( !( elmType == HEX8 || elmType == HEX8P ) ) continue;

        interpolatedInterfaceValues[intersectionIdx] = interpolateGridResultValue<T>( nativeAddr.resultPosType,
                                                                                      unscaledResultValues,
                                                                                      intersectionIdx );
    }
    return interpolatedInterfaceValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::initializeResultValues( std::vector<float>& resultValues, size_t resultCount )
{
    resultValues.resize( resultCount, std::numeric_limits<float>::infinity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::initializeResultValues( std::vector<caf::Ten3d>& resultValues, size_t resultCount )
{
    resultValues.resize( resultCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::filterShortSegments( std::vector<double>*               xValues,
                                                      std::vector<double>*               yValues,
                                                      std::vector<unsigned char>*        filterSegments,
                                                      std::vector<std::vector<double>*>& vectorOfDependentValues )
{
    std::vector<double>              simplerXValues;
    std::vector<double>              simplerYValues;
    std::vector<unsigned char>       simpledFilterSegments;
    std::vector<std::vector<double>> simplerDependentValues( vectorOfDependentValues.size() );

    simplerXValues.push_back( xValues->front() );
    simplerYValues.push_back( yValues->front() );
    simpledFilterSegments.push_back( filterSegments->front() );
    for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
    {
        simplerDependentValues[n].push_back( vectorOfDependentValues[n]->front() );
    }
    for ( int64_t i = 1; i < int64_t( xValues->size() - 1 ); ++i )
    {
        cvf::Vec2d vecIn( ( ( *xValues )[i] - simplerXValues.back() ) / std::max( 1.0, simplerXValues.back() ),
                          ( ( *yValues )[i] - simplerYValues.back() ) / std::max( 1.0, simplerYValues.back() ) );
        if ( ( *filterSegments )[i] == 0u || vecIn.length() > 1.0e-3 )
        {
            simplerXValues.push_back( ( *xValues )[i] );
            simplerYValues.push_back( ( *yValues )[i] );
            simpledFilterSegments.push_back( ( *filterSegments )[i] );
            for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
            {
                simplerDependentValues[n].push_back( ( *vectorOfDependentValues[n] )[i] );
            }
        }
    }
    simplerXValues.push_back( xValues->back() );
    simplerYValues.push_back( yValues->back() );
    simpledFilterSegments.push_back( filterSegments->back() );
    for ( size_t i = 0; i < vectorOfDependentValues.size(); ++i )
    {
        simplerDependentValues[i].push_back( vectorOfDependentValues[i]->back() );
    }

    xValues->swap( simplerXValues );
    yValues->swap( simplerYValues );
    filterSegments->swap( simpledFilterSegments );
    for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
    {
        vectorOfDependentValues[n]->swap( simplerDependentValues[n] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::filterColinearSegments( std::vector<double>*               xValues,
                                                         std::vector<double>*               yValues,
                                                         std::vector<unsigned char>*        filterSegments,
                                                         std::vector<std::vector<double>*>& vectorOfDependentValues )
{
    std::vector<double>              simplerXValues;
    std::vector<double>              simplerYValues;
    std::vector<unsigned char>       simpledFilterSegments;
    std::vector<std::vector<double>> simplerDependentValues( vectorOfDependentValues.size() );

    simplerXValues.push_back( xValues->front() );
    simplerYValues.push_back( yValues->front() );
    simpledFilterSegments.push_back( filterSegments->front() );

    for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
    {
        simplerDependentValues[n].push_back( vectorOfDependentValues[n]->front() );
    }
    for ( int64_t i = 1; i < int64_t( xValues->size() - 1 ); ++i )
    {
        cvf::Vec2d vecIn( ( ( *xValues )[i] - simplerXValues.back() ) / std::max( 1.0, simplerXValues.back() ),
                          ( ( *yValues )[i] - simplerYValues.back() ) / std::max( 1.0, simplerYValues.back() ) );
        cvf::Vec2d vecOut( ( ( *xValues )[i + 1] - ( *xValues )[i] ) / std::max( 1.0, ( *xValues )[i] ),
                           ( ( *yValues )[i + 1] - ( *yValues )[i] ) / std::max( 1.0, ( *yValues )[i] ) );
        vecIn.normalize();
        vecOut.normalize();
        double dotProduct = std::abs( vecIn * vecOut );

        if ( ( *filterSegments )[i] == 0u || std::fabs( 1.0 - dotProduct ) > 1.0e-3 )
        {
            simplerXValues.push_back( ( *xValues )[i] );
            simplerYValues.push_back( ( *yValues )[i] );
            simpledFilterSegments.push_back( ( *filterSegments )[i] );
            for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
            {
                simplerDependentValues[n].push_back( ( *vectorOfDependentValues[n] )[i] );
            }
        }
    }
    simplerXValues.push_back( xValues->back() );
    simplerYValues.push_back( yValues->back() );
    simpledFilterSegments.push_back( filterSegments->back() );

    for ( size_t i = 0; i < vectorOfDependentValues.size(); ++i )
    {
        simplerDependentValues[i].push_back( vectorOfDependentValues[i]->back() );
    }

    xValues->swap( simplerXValues );
    yValues->swap( simplerYValues );
    filterSegments->swap( simpledFilterSegments );
    for ( size_t n = 0; n < vectorOfDependentValues.size(); ++n )
    {
        vectorOfDependentValues[n]->swap( simplerDependentValues[n] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::smoothSegments( std::vector<double>*              mds,
                                                 std::vector<double>*              tvds,
                                                 std::vector<double>*              values,
                                                 const std::vector<double>&        interfaceShValues,
                                                 const std::vector<unsigned char>& smoothSegments,
                                                 const double                      smoothingThreshold )
{
    const double eps = 1.0e-6;

    double maxOriginalMd  = ( *mds )[0];
    double maxOriginalTvd = ( !tvds->empty() ) ? ( *tvds )[0] : 0.0;
    for ( int64_t i = 1; i < int64_t( mds->size() - 1 ); ++i )
    {
        double originalMD  = ( *mds )[i];
        double originalTVD = ( !tvds->empty() ) ? ( *tvds )[i] : 0.0;

        bool smoothSegment = smoothSegments[i] != 0u;

        double diffMd = std::fabs( ( *mds )[i + 1] - ( *mds )[i] ) / std::max( eps, ( *mds )[i] );
        double diffSh = std::fabs( interfaceShValues[i + 1] - interfaceShValues[i] ) /
                        std::max( eps, interfaceShValues[i] );

        bool leapSh = diffSh > smoothingThreshold && diffMd < eps;
        if ( smoothSegment )
        {
            if ( leapSh )
            {
                // Update depth of current
                ( *mds )[i] = 0.5 * ( ( *mds )[i] + maxOriginalMd );

                if ( !tvds->empty() )
                {
                    ( *tvds )[i] = 0.5 * ( ( *tvds )[i] + maxOriginalTvd );
                }
            }
            else
            {
                // Update depth of current
                ( *mds )[i] = ( *mds )[i - 1];

                if ( !tvds->empty() )
                {
                    ( *tvds )[i] = ( *tvds )[i - 1];
                }
            }
            double diffMd_m1 = std::fabs( ( *mds )[i] - ( *mds )[i - 1] );
            if ( diffMd_m1 < ( *mds )[i] * eps && ( *values )[i - 1] != std::numeric_limits<double>::infinity() )
            {
                ( *values )[i] = ( *values )[i - 1];
            }
        }
        if ( leapSh )
        {
            maxOriginalMd  = std::max( maxOriginalMd, originalMD );
            maxOriginalTvd = std::max( maxOriginalTvd, originalTVD );
        }
    }
    ( *values )[0] = std::numeric_limits<float>::infinity();
}

//--------------------------------------------------------------------------------------------------
/// Note that this is unsigned char because std::vector<bool> is not thread safe
//--------------------------------------------------------------------------------------------------
std::vector<unsigned char>
    RigGeoMechWellLogExtractor::determineFilteringOrSmoothing( const std::vector<double>& porePressures )
{
    std::vector<unsigned char> smoothOrFilterSegments( porePressures.size(), false );
#pragma omp parallel for
    for ( int64_t i = 1; i < int64_t( porePressures.size() - 1 ); ++i )
    {
        bool validPP_im1 = porePressures[i - 1] >= 0.0 && porePressures[i - 1] != std::numeric_limits<double>::infinity();
        bool validPP_i   = porePressures[i] >= 0.0 && porePressures[i] != std::numeric_limits<double>::infinity();
        bool validPP_ip1 = porePressures[i + 1] >= 0.0 && porePressures[i + 1] != std::numeric_limits<double>::infinity();
        bool anyValidPP           = validPP_im1 || validPP_i || validPP_ip1;
        smoothOrFilterSegments[i] = !anyValidPP;
    }
    return smoothOrFilterSegments;
}
