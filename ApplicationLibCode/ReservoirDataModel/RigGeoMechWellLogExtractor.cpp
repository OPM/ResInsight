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
#include "RiaLogging.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemTypes.h"
#include "RigGeoMechBoreHoleStressCalculator.h"
#include "RigGeoMechCaseData.h"

#include "RiaWellLogUnitTools.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"
#include "RigWellPathIntersectionTools.h"

#include "cafTensor3.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"

#include <QDebug>
#include <QPolygonF>

#include <type_traits>

const double RigGeoMechWellLogExtractor::PURE_WATER_DENSITY_GCM3 = 1.0; // g / cm^3
const double RigGeoMechWellLogExtractor::GRAVITY_ACCEL           = 9.81; // m / s^2

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor::RigGeoMechWellLogExtractor( gsl::not_null<RigGeoMechCaseData*> aCase,
                                                        gsl::not_null<const RigWellPath*>  wellpath,
                                                        const std::string&                 wellCaseErrorMsgName )
    : RigWellLogExtractor( wellpath, wellCaseErrorMsgName )
    , m_caseData( aCase )
{
    calculateIntersection();

    for ( RigWbsParameter parameter : RigWbsParameter::allParameters() )
    {
        m_parameterSources[parameter]  = parameter.sources().front();
        m_lasFileValues[parameter]     = std::vector<std::pair<double, double>>();
        m_userDefinedValues[parameter] = std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::performCurveDataSmoothing( int                  frameIndex,
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

    std::vector<float> interfaceShValues      = interpolateInterfaceValues( shAddr, frameIndex, unscaledShValues );
    std::vector<float> interfacePorePressures = interpolateInterfaceValues( porBarResAddr, frameIndex, porePressures );

    std::vector<double> interfaceShValuesDbl( interfaceShValues.size(), std::numeric_limits<double>::infinity() );
    std::vector<double> interfacePorePressuresDbl( interfacePorePressures.size(), std::numeric_limits<double>::infinity() );
#pragma omp parallel for
    for ( int64_t i = 0; i < int64_t( m_intersections.size() ); ++i )
    {
        double hydroStaticPorePressureBar = hydroStaticPorePressureForSegment( i );
        interfaceShValuesDbl[i]           = interfaceShValues[i] / hydroStaticPorePressureBar;
        interfacePorePressuresDbl[i]      = interfacePorePressures[i];
    }

    if ( !mds->empty() && !values->empty() )
    {
        std::vector<unsigned char> smoothOrFilterSegments = determineFilteringOrSmoothing( interfacePorePressuresDbl );

        smoothSegments( mds, tvds, values, interfaceShValuesDbl, smoothOrFilterSegments, smoothingTreshold );
    }
}

//--------------------------------------------------------------------------------------------------
/// Get curve data for a given parameter. Returns the output units of the data.
//--------------------------------------------------------------------------------------------------
QString RigGeoMechWellLogExtractor::curveData( const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values )
{
    CVF_TIGHT_ASSERT( values );

    if ( resAddr.resultPosType == RIG_WELLPATH_DERIVED )
    {
        if ( m_wellPathGeometry->rkbDiff() == HUGE_VAL )
        {
            RiaLogging::error( "Well path has an invalid datum elevation and we cannot estimate TVDRKB. No well bore "
                               "stability curves created." );
            return "";
        }

        if ( resAddr.fieldName == RiaDefines::wbsFGResult().toStdString() )
        {
            wellBoreWallCurveData( resAddr, frameIndex, values );
            // Try to replace invalid values with Shale-values
            wellBoreFGShale( frameIndex, values );
            values->front() = wbsCurveValuesAtMsl();
        }
        else if ( resAddr.fieldName == RiaDefines::wbsSFGResult().toStdString() )
        {
            wellBoreWallCurveData( resAddr, frameIndex, values );
        }
        else if ( resAddr.fieldName == RiaDefines::wbsPPResult().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsOBGResult().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsSHResult().toStdString() )
        {
            wellPathScaledCurveData( resAddr, frameIndex, values );
            values->front() = wbsCurveValuesAtMsl();
        }
        else if ( resAddr.fieldName == RiaDefines::wbsAzimuthResult().toStdString() ||
                  resAddr.fieldName == RiaDefines::wbsInclinationResult().toStdString() )
        {
            wellPathAngles( resAddr, values );
        }
        else if ( resAddr.fieldName == RiaDefines::wbsSHMkResult().toStdString() )
        {
            wellBoreSH_MatthewsKelly( frameIndex, values );
            values->front() = wbsCurveValuesAtMsl();
        }
        else
        {
            // Plotting parameters as curves
            RigWbsParameter param;
            if ( RigWbsParameter::findParameter( QString::fromStdString( resAddr.fieldName ), &param ) )
            {
                if ( param == RigWbsParameter::FG_Shale() )
                {
                    wellBoreFGShale( frameIndex, values );
                }
                else
                {
                    if ( param == RigWbsParameter::OBG0() )
                    {
                        frameIndex = 0;
                    }
                    calculateWbsParameterForAllSegments( param, frameIndex, values, true );
                    if ( param == RigWbsParameter::UCS() ) // UCS is reported as UCS/100
                    {
                        for ( double& value : *values )
                        {
                            if ( isValid( value ) ) value /= 100.0;
                        }
                        return RiaWellLogUnitTools<double>::barX100UnitString();
                    }
                    else if ( param == RigWbsParameter::DF() || param == RigWbsParameter::poissonRatio() )
                    {
                        return RiaWellLogUnitTools<double>::noUnitString();
                    }
                }
            }
        }
        return RiaWellLogUnitTools<double>::sg_emwUnitString();
    }
    else if ( resAddr.isValid() )
    {
        RigFemResultAddress convResAddr = resAddr;

        // When showing POR results, always use the element nodal result,
        // to get correct handling of elements without POR results

        if ( convResAddr.fieldName == "POR-Bar" ) convResAddr.resultPosType = RIG_ELEMENT_NODAL;

        CVF_ASSERT( resAddr.resultPosType != RIG_WELLPATH_DERIVED );

        const std::vector<float>& resultValues = m_caseData->femPartResults()->resultValues( convResAddr, 0, frameIndex );

        if ( !resultValues.empty() )
        {
            std::vector<float> interfaceValues = interpolateInterfaceValues( convResAddr, frameIndex, resultValues );

            values->resize( interfaceValues.size(), std::numeric_limits<double>::infinity() );

#pragma omp parallel for
            for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
            {
                ( *values )[intersectionIdx] = static_cast<double>( interfaceValues[intersectionIdx] );
            }
        }
    }
    return RiaWellLogUnitTools<double>::barUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::calculateWbsParameterForAllSegments( const RigWbsParameter& parameter,
                                                                     WbsParameterSource     primarySource,
                                                                     int                    frameIndex,
                                                                     std::vector<double>*   outputValues,
                                                                     bool                   allowNormalization )
{
    RigFemPartResultsCollection* resultCollection = m_caseData->femPartResults();

    std::vector<WbsParameterSource> finalSourcesPerSegment( m_intersections.size(), RigWbsParameter::UNDEFINED );

    if ( primarySource == RigWbsParameter::UNDEFINED )
    {
        return finalSourcesPerSegment;
    }

    bool isPPResResult = parameter == RigWbsParameter::PP_Reservoir();
    bool isPPresult    = isPPResResult || parameter == RigWbsParameter::PP_NonReservoir();

    std::vector<WbsParameterSource> allSources = parameter.sources();
    auto                            primary_it = std::find( allSources.begin(), allSources.end(), primarySource );
    CVF_ASSERT( primary_it != allSources.end() );

    std::vector<double> gridValues;
    if ( std::find( allSources.begin(), allSources.end(), RigWbsParameter::GRID ) != allSources.end() ||
         parameter == RigWbsParameter::PP_Reservoir() )
    {
        RigFemResultAddress nativeAddr = parameter.femAddress( RigWbsParameter::GRID );

        const std::vector<float>& unscaledResultValues = resultCollection->resultValues( nativeAddr, 0, frameIndex );
        std::vector<float>        interpolatedInterfaceValues =
            interpolateInterfaceValues( nativeAddr, frameIndex, unscaledResultValues );
        gridValues.resize( m_intersections.size(), std::numeric_limits<double>::infinity() );

#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            float averageUnscaledValue = std::numeric_limits<float>::infinity();
            averageIntersectionValuesToSegmentValue( intersectionIdx,
                                                     interpolatedInterfaceValues,
                                                     std::numeric_limits<float>::infinity(),
                                                     &averageUnscaledValue );
            gridValues[intersectionIdx] = static_cast<double>( averageUnscaledValue );
        }
    }

    const std::vector<std::pair<double, double>>& lasFileValues    = m_lasFileValues.at( parameter );
    const double&                                 userDefinedValue = m_userDefinedValues.at( parameter );

    std::vector<float> elementPropertyValues;
    if ( std::find( allSources.begin(), allSources.end(), RigWbsParameter::ELEMENT_PROPERTY_TABLE ) != allSources.end() )
    {
        const std::vector<float>* elementPropertyValuesInput = nullptr;

        std::vector<float> tvdRKBs;
        for ( double tvdValue : m_intersectionTVDs )
        {
            tvdRKBs.push_back( tvdValue + m_wellPathGeometry->rkbDiff() );
        }
        RigFemResultAddress elementPropertyAddr = parameter.femAddress( RigWbsParameter::ELEMENT_PROPERTY_TABLE );
        elementPropertyValuesInput = &( resultCollection->resultValues( elementPropertyAddr, 0, frameIndex ) );
        if ( elementPropertyValuesInput )
        {
            RiaWellLogUnitTools<float>::convertValues( tvdRKBs,
                                                       *elementPropertyValuesInput,
                                                       &elementPropertyValues,
                                                       parameter.units( RigWbsParameter::ELEMENT_PROPERTY_TABLE ),
                                                       parameterInputUnits( parameter ) );
        }
    }

    std::vector<double> unscaledValues( m_intersections.size(), std::numeric_limits<double>::infinity() );

    double waterDensityGCM3 = m_userDefinedValues[RigWbsParameter::waterDensity()];

    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        // Loop from primary source and out for each value
        for ( auto it = primary_it; it != allSources.end(); ++it )
        {
            if ( *it == RigWbsParameter::GRID ) // Priority 0: Grid
            {
                if ( intersectionIdx < (int64_t)gridValues.size() &&
                     gridValues[intersectionIdx] != std::numeric_limits<double>::infinity() )
                {
                    unscaledValues[intersectionIdx]         = gridValues[intersectionIdx];
                    finalSourcesPerSegment[intersectionIdx] = RigWbsParameter::GRID;
                    break;
                }
            }
            else if ( *it == RigWbsParameter::LAS_FILE ) // Priority 1: Las-file value
            {
                if ( !lasFileValues.empty() )
                {
                    double lasValue = getWellLogIntersectionValue( intersectionIdx, lasFileValues );
                    // Only accept las-values for PP_reservoir if the grid result is valid
                    bool validLasRegion = true;
                    if ( isPPResResult )
                    {
                        validLasRegion = intersectionIdx < static_cast<int64_t>( gridValues.size() ) &&
                                         gridValues[intersectionIdx] != std::numeric_limits<double>::infinity();
                    }

                    if ( validLasRegion && lasValue != std::numeric_limits<double>::infinity() )
                    {
                        unscaledValues[intersectionIdx]         = lasValue;
                        finalSourcesPerSegment[intersectionIdx] = RigWbsParameter::LAS_FILE;
                        break;
                    }
                }
            }
            else if ( *it == RigWbsParameter::ELEMENT_PROPERTY_TABLE ) // Priority 2: Element property table value
            {
                if ( !elementPropertyValues.empty() )
                {
                    size_t elmIdx = m_intersectedCellsGlobIdx[intersectionIdx];
                    if ( elmIdx < elementPropertyValues.size() )
                    {
                        unscaledValues[intersectionIdx]         = elementPropertyValues[elmIdx];
                        finalSourcesPerSegment[intersectionIdx] = RigWbsParameter::ELEMENT_PROPERTY_TABLE;
                        break;
                    }
                }
            }
            else if ( *it == RigWbsParameter::HYDROSTATIC && isPPresult )
            {
                unscaledValues[intersectionIdx] =
                    userDefinedValue * hydroStaticPorePressureForIntersection( intersectionIdx, waterDensityGCM3 );
                finalSourcesPerSegment[intersectionIdx] = RigWbsParameter::HYDROSTATIC;
                break;
            }
            else if ( *it == RigWbsParameter::USER_DEFINED )
            {
                unscaledValues[intersectionIdx]         = userDefinedValue;
                finalSourcesPerSegment[intersectionIdx] = RigWbsParameter::USER_DEFINED;
                break;
            }
        }
    }

    if ( allowNormalization && parameter.normalizeByHydrostaticPP() )
    {
        outputValues->resize( unscaledValues.size(), std::numeric_limits<double>::infinity() );

#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            RigWbsParameter::Source source = finalSourcesPerSegment[intersectionIdx];

            if ( source == RigWbsParameter::ELEMENT_PROPERTY_TABLE || source == RigWbsParameter::GRID )
            {
                ( *outputValues )[intersectionIdx] =
                    unscaledValues[intersectionIdx] / hydroStaticPorePressureForSegment( intersectionIdx );
            }
            else
            {
                ( *outputValues )[intersectionIdx] =
                    unscaledValues[intersectionIdx] / hydroStaticPorePressureForIntersection( intersectionIdx );
            }
        }
    }
    else
    {
        outputValues->swap( unscaledValues );
    }
    return finalSourcesPerSegment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::calculateWbsParameterForAllSegments( const RigWbsParameter& parameter,
                                                                     int                    frameIndex,
                                                                     std::vector<double>*   outputValues,
                                                                     bool                   allowNormalization )
{
    return calculateWbsParameterForAllSegments( parameter,
                                                m_parameterSources.at( parameter ),
                                                frameIndex,
                                                outputValues,
                                                allowNormalization );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::calculateWbsParametersForAllSegments( const RigFemResultAddress& resAddr,
                                                                      int                        frameIndex,
                                                                      std::vector<double>*       values,
                                                                      bool                       allowNormalization )
{
    CVF_ASSERT( values );

    RigWbsParameter param;
    if ( !RigWbsParameter::findParameter( QString::fromStdString( resAddr.fieldName ), &param ) )
    {
        CVF_ASSERT( false && "wbsParameters() called on something that isn't a wbs parameter" );
    }

    return calculateWbsParameterForAllSegments( param, m_userDefinedValues.at( param ), values, allowNormalization );
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
    double           previousAzimuth = 0.0;
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        cvf::Vec3d wellPathTangent = calculateWellPathTangent( intersectionIdx, TangentFollowWellPathSegments );

        // Deviation from vertical. Since well path is tending downwards we compare with negative z.
        double inclination =
            cvf::Math::toDegrees( std::acos( cvf::Vec3d( 0.0, 0.0, -1.0 ) * wellPathTangent.getNormalized() ) );

        if ( resAddr.fieldName == "Azimuth" )
        {
            double azimuth = HUGE_VAL;

            // Azimuth is not defined when well path is vertical. We define it as infinite to avoid it showing up in the
            // plot.
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
            // Make azimuth continuous in most cases
            if ( azimuth - previousAzimuth > 300.0 )
            {
                azimuth -= 360.0;
            }
            else if ( previousAzimuth - azimuth > 300.0 )
            {
                azimuth += 360.0;
            }

            ( *values )[intersectionIdx] = azimuth;
            previousAzimuth              = azimuth;
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
std::vector<RigGeoMechWellLogExtractor::WbsParameterSource>
    RigGeoMechWellLogExtractor::wellPathScaledCurveData( const RigFemResultAddress& resAddr,
                                                         int                        frameIndex,
                                                         std::vector<double>*       values,
                                                         bool forceGridSourceForPPReservoir /*=false*/ )
{
    CVF_ASSERT( values );

    values->resize( m_intersections.size(), std::numeric_limits<double>::infinity() );
    std::vector<WbsParameterSource> sources( m_intersections.size(), RigWbsParameter::UNDEFINED );

    if ( resAddr.fieldName == RiaDefines::wbsPPResult().toStdString() )
    {
        // Las or element property table values
        std::vector<double> ppSandValues( m_intersections.size(), std::numeric_limits<double>::infinity() );
        std::vector<double> ppShaleValues( m_intersections.size(), std::numeric_limits<double>::infinity() );

        std::vector<WbsParameterSource> ppSandSources;
        if ( forceGridSourceForPPReservoir )
        {
            ppSandSources = calculateWbsParameterForAllSegments( RigWbsParameter::PP_Reservoir(),
                                                                 RigWbsParameter::GRID,
                                                                 frameIndex,
                                                                 &ppSandValues,
                                                                 true );
        }
        else
        {
            ppSandSources =
                calculateWbsParameterForAllSegments( RigWbsParameter::PP_Reservoir(), frameIndex, &ppSandValues, true );
        }

        std::vector<WbsParameterSource> ppShaleSources =
            calculateWbsParameterForAllSegments( RigWbsParameter::PP_NonReservoir(), 0, &ppShaleValues, true );

#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            if ( ( *values )[intersectionIdx] == std::numeric_limits<double>::infinity() )
            {
                if ( ppSandValues[intersectionIdx] != std::numeric_limits<double>::infinity() )
                {
                    ( *values )[intersectionIdx] = ppSandValues[intersectionIdx];
                    sources[intersectionIdx]     = ppSandSources[intersectionIdx];
                }
                else if ( ppShaleValues[intersectionIdx] != std::numeric_limits<double>::infinity() )
                {
                    ( *values )[intersectionIdx] = ppShaleValues[intersectionIdx];
                    sources[intersectionIdx]     = ppShaleSources[intersectionIdx];
                }
                else
                {
                    ( *values )[intersectionIdx] = 1.0;
                    sources[intersectionIdx]     = RigWbsParameter::HYDROSTATIC;
                }
            }
        }
    }
    else if ( resAddr.fieldName == RiaDefines::wbsOBGResult().toStdString() )
    {
        sources = calculateWbsParameterForAllSegments( RigWbsParameter::OBG(), frameIndex, values, true );
    }
    else
    {
        sources = calculateWbsParameterForAllSegments( RigWbsParameter::SH(), frameIndex, values, true );
    }

    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellBoreWallCurveData( const RigFemResultAddress& resAddr,
                                                        int                        frameIndex,
                                                        std::vector<double>*       values )
{
    CVF_ASSERT( values );
    CVF_ASSERT( resAddr.fieldName == RiaDefines::wbsFGResult().toStdString() ||
                resAddr.fieldName == RiaDefines::wbsSFGResult().toStdString() );

    // The result addresses needed
    RigFemResultAddress stressResAddr( RIG_ELEMENT_NODAL, "ST", "" );
    RigFemResultAddress porBarResAddr( RIG_ELEMENT_NODAL, "POR-Bar", "" );

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

    std::vector<caf::Ten3d> interpolatedInterfaceStressBar =
        interpolateInterfaceValues( stressResAddr, frameIndex, vertexStresses );

    values->resize( m_intersections.size(), std::numeric_limits<float>::infinity() );

    std::vector<double> ppSandAllSegments( m_intersections.size(), std::numeric_limits<double>::infinity() );
    std::vector<WbsParameterSource> ppSources = calculateWbsParameterForAllSegments( RigWbsParameter::PP_Reservoir(),
                                                                                     RigWbsParameter::GRID,
                                                                                     frameIndex,
                                                                                     &ppSandAllSegments,
                                                                                     false );

    std::vector<double> poissonAllSegments( m_intersections.size(), std::numeric_limits<double>::infinity() );
    calculateWbsParameterForAllSegments( RigWbsParameter::poissonRatio(), frameIndex, &poissonAllSegments, false );

    std::vector<double> ucsAllSegments( m_intersections.size(), std::numeric_limits<double>::infinity() );
    calculateWbsParameterForAllSegments( RigWbsParameter::UCS(), frameIndex, &ucsAllSegments, false );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        // FG is for sands, SFG for shale. Sands has valid PP, shale does not.
        bool isFGregion = ppSources[intersectionIdx] == RigWbsParameter::GRID;

        double hydroStaticPorePressureBar = hydroStaticPorePressureForSegment( intersectionIdx );

        double porePressureBar = ppSandAllSegments[intersectionIdx];
        if ( porePressureBar == std::numeric_limits<double>::infinity() )
        {
            porePressureBar = hydroStaticPorePressureBar;
        }

        double poissonRatio = poissonAllSegments[intersectionIdx];
        double ucsBar       = ucsAllSegments[intersectionIdx];

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
        if ( resAddr.fieldName == RiaDefines::wbsFGResult().toStdString() )
        {
            if ( isFGregion && validSegmentStress )
            {
                resultValue = sigmaCalculator.solveFractureGradient();
            }
        }
        else
        {
            CVF_ASSERT( resAddr.fieldName == RiaDefines::wbsSFGResult().toStdString() );
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
void RigGeoMechWellLogExtractor::wellBoreFGShale( int frameIndex, std::vector<double>* values )
{
    if ( values->empty() ) values->resize( m_intersections.size(), std::numeric_limits<double>::infinity() );

    WbsParameterSource source = m_parameterSources.at( RigWbsParameter::FG_Shale() );
    if ( source == RigWbsParameter::DERIVED_FROM_K0FG )
    {
        std::vector<double> PP0; // results
        std::vector<double> K0_FG, OBG0; // parameters

        RigFemResultAddress ppAddr( RIG_WELLPATH_DERIVED, RiaDefines::wbsPPResult().toStdString(), "" );
        wellPathScaledCurveData( ppAddr, 0, &PP0, true );

        calculateWbsParameterForAllSegments( RigWbsParameter::K0_FG(), frameIndex, &K0_FG, true );
        calculateWbsParameterForAllSegments( RigWbsParameter::OBG0(), 0, &OBG0, true );

#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            if ( !isValid( ( *values )[intersectionIdx] ) )
            {
                if ( isValid( PP0[intersectionIdx] ) && isValid( OBG0[intersectionIdx] ) &&
                     isValid( K0_FG[intersectionIdx] ) )
                {
                    ( *values )[intersectionIdx] =
                        ( K0_FG[intersectionIdx] * ( OBG0[intersectionIdx] - PP0[intersectionIdx] ) + PP0[intersectionIdx] );
                }
            }
        }
    }
    else
    {
        std::vector<double> SH;
        calculateWbsParameterForAllSegments( RigWbsParameter::SH(), frameIndex, &SH, true );
        CVF_ASSERT( SH.size() == m_intersections.size() );
        double multiplier = m_userDefinedValues.at( RigWbsParameter::FG_Shale() );
        CVF_ASSERT( multiplier != std::numeric_limits<double>::infinity() );
#pragma omp parallel for
        for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
        {
            if ( !isValid( ( *values )[intersectionIdx] ) )
            {
                if ( isValid( SH[intersectionIdx] ) )
                {
                    ( *values )[intersectionIdx] = SH[intersectionIdx] * multiplier;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::wellBoreSH_MatthewsKelly( int frameIndex, std::vector<double>* values )
{
    std::vector<double> PP, PP0; // results
    std::vector<double> K0_SH, OBG0, DF; // parameters

    RigFemResultAddress ppAddr( RIG_WELLPATH_DERIVED, RiaDefines::wbsPPResult().toStdString(), "" );

    curveData( ppAddr, frameIndex, &PP );
    curveData( ppAddr, 0, &PP0 );

    calculateWbsParameterForAllSegments( RigWbsParameter::K0_SH(), frameIndex, &K0_SH, true );
    calculateWbsParameterForAllSegments( RigWbsParameter::OBG0(), 0, &OBG0, true );
    calculateWbsParameterForAllSegments( RigWbsParameter::DF(), frameIndex, &DF, true );

    values->resize( m_intersections.size(), std::numeric_limits<double>::infinity() );

#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        if ( isValid( PP[intersectionIdx] ) && isValid( PP0[intersectionIdx] ) && isValid( OBG0[intersectionIdx] ) &&
             isValid( K0_SH[intersectionIdx] ) && isValid( DF[intersectionIdx] ) )
        {
            ( *values )[intersectionIdx] =
                ( K0_SH[intersectionIdx] * ( OBG0[intersectionIdx] - PP0[intersectionIdx] ) + PP0[intersectionIdx] +
                  DF[intersectionIdx] * ( PP[intersectionIdx] - PP0[intersectionIdx] ) );
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
void RigGeoMechWellLogExtractor::setWbsLasValues( const RigWbsParameter&                        parameter,
                                                  const std::vector<std::pair<double, double>>& values )
{
    m_lasFileValues[parameter] = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWbsParametersSource( RigWbsParameter parameter, WbsParameterSource source )
{
    m_parameterSources[parameter] = source;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechWellLogExtractor::setWbsUserDefinedValue( RigWbsParameter parameter, double userDefinedValue )
{
    m_userDefinedValues[parameter] = userDefinedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigGeoMechWellLogExtractor::parameterInputUnits( const RigWbsParameter& parameter )
{
    if ( parameter == RigWbsParameter::PP_NonReservoir() || parameter == RigWbsParameter::PP_Reservoir() ||
         parameter == RigWbsParameter::UCS() )
    {
        return RiaWellLogUnitTools<double>::barUnitString();
    }
    else if ( parameter == RigWbsParameter::poissonRatio() || parameter == RigWbsParameter::DF() )
    {
        return RiaWellLogUnitTools<double>::noUnitString();
    }
    else if ( parameter == RigWbsParameter::waterDensity() )
    {
        return RiaWellLogUnitTools<double>::gPerCm3UnitString();
    }
    return RiaWellLogUnitTools<double>::sg_emwUnitString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::porePressureSourceRegions( int frameIndex )
{
    RigFemResultAddress ppResAddr( RIG_ELEMENT_NODAL, RiaDefines::wbsPPResult().toStdString(), "" );

    std::vector<double>             values;
    std::vector<WbsParameterSource> sources = wellPathScaledCurveData( ppResAddr, frameIndex, &values );

    std::vector<double> doubleSources( sources.size(), 0.0 );
#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        doubleSources[intersectionIdx] = static_cast<double>( sources[intersectionIdx] );
    }
    return doubleSources;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::poissonSourceRegions( int frameIndex )
{
    std::vector<double>             outputValues;
    std::vector<WbsParameterSource> sources =
        calculateWbsParameterForAllSegments( RigWbsParameter::poissonRatio(), frameIndex, &outputValues, false );

    std::vector<double> doubleSources( sources.size(), 0.0 );
#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        doubleSources[intersectionIdx] = static_cast<double>( sources[intersectionIdx] );
    }
    return doubleSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigGeoMechWellLogExtractor::ucsSourceRegions( int frameIndex )
{
    std::vector<double>             outputValues;
    std::vector<WbsParameterSource> sources =
        calculateWbsParameterForAllSegments( RigWbsParameter::UCS(), frameIndex, &outputValues, true );

    std::vector<double> doubleSources( sources.size(), 0.0 );
#pragma omp parallel for
    for ( int64_t intersectionIdx = 0; intersectionIdx < (int64_t)m_intersections.size(); ++intersectionIdx )
    {
        doubleSources[intersectionIdx] = static_cast<double>( sources[intersectionIdx] );
    }
    return doubleSources;
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
            return std::numeric_limits<T>::infinity(); // undefined value. ELEMENT_NODAL_FACE values are only defined on
                                                       // a face.
        }
        // TODO: Should interpolate within the whole hexahedron. This requires converting to locals coordinates.
        // For now just pick the average value for the cell.
        size_t gridResultValueIdx =
            femPart->resultValueIdxFromResultPosType( resultPosType, static_cast<int>( elmIdx ), 0 );
        T sumOfVertexValues = gridResultValues[gridResultValueIdx];
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

    for ( size_t wpp = 0; wpp < m_wellPathGeometry->wellPathPoints().size() - 1; ++wpp )
    {
        std::vector<HexIntersectionInfo> intersections;
        cvf::Vec3d                       p1 = m_wellPathGeometry->wellPathPoints()[wpp];
        cvf::Vec3d                       p2 = m_wellPathGeometry->wellPathPoints()[wpp + 1];

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

        double md1 = m_wellPathGeometry->measuredDepths()[wpp];
        double md2 = m_wellPathGeometry->measuredDepths()[wpp + 1];

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
        m_wellPathGeometry->twoClosestPoints( m_intersections[intersectionIdx], &segmentStart, &segmentEnd );
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
    int            elementNodeCount = RigFemTypes::elementNodeCount( elmType );

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
double RigGeoMechWellLogExtractor::getWellLogIntersectionValue( size_t intersectionIdx,
                                                                const std::vector<std::pair<double, double>>& wellLogValues ) const
{
    const double eps = 1.0e-4;

    double intersection_md = m_intersectionMeasuredDepths[intersectionIdx];
    for ( size_t i = 0; i < wellLogValues.size() - 1; ++i )
    {
        double las_md_i   = wellLogValues[i].first;
        double las_md_ip1 = wellLogValues[i + 1].first;
        if ( cvf::Math::valueInRange( intersection_md, las_md_i, las_md_ip1 ) )
        {
            double dist_i   = std::abs( intersection_md - las_md_i );
            double dist_ip1 = std::abs( intersection_md - las_md_ip1 );

            if ( dist_i < eps )
            {
                return wellLogValues[i].second;
            }
            else if ( dist_ip1 < eps )
            {
                return wellLogValues[i + 1].second;
            }
            else
            {
                RiaWeightedMeanCalculator<double> averageCalc;
                averageCalc.addValueAndWeight( wellLogValues[i].second, 1.0 / dist_i );
                averageCalc.addValueAndWeight( wellLogValues[i + 1].second, 1.0 / dist_ip1 );
                return averageCalc.weightedMean();
            }
        }
    }

    // If we found no match, check first and last value within a threshold.
    if ( !wellLogValues.empty() )
    {
        const double relativeEps = 1.0e-3 * std::max( 1.0, intersection_md );
        if ( std::abs( wellLogValues.front().first - intersection_md ) < relativeEps )
        {
            return wellLogValues.front().second;
        }
        else if ( std::abs( wellLogValues.back().first - intersection_md ) < relativeEps )
        {
            return wellLogValues.back().second;
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
                                                                       int                   frameIndex,
                                                                       const std::vector<T>& unscaledResultValues )
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

        interpolatedInterfaceValues[intersectionIdx] =
            interpolateGridResultValue<T>( nativeAddr.resultPosType, unscaledResultValues, intersectionIdx );
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
        double diffSh =
            std::fabs( interfaceShValues[i + 1] - interfaceShValues[i] ) / std::max( eps, interfaceShValues[i] );

        bool leapSh = diffSh > smoothingThreshold && diffMd < eps;
        if ( smoothSegment )
        {
            if ( leapSh )
            {
                // Update depth of current
                if ( i == 1 )
                {
                    ( *mds )[i] = maxOriginalMd;
                    if ( !tvds->empty() )
                    {
                        ( *tvds )[i] = maxOriginalTvd;
                    }
                }
                else
                {
                    ( *mds )[i] = 0.5 * ( ( *mds )[i] + maxOriginalMd );
                    if ( !tvds->empty() )
                    {
                        ( *tvds )[i] = 0.5 * ( ( *tvds )[i] + maxOriginalTvd );
                    }
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::hydroStaticPorePressureForIntersection( size_t intersectionIdx,
                                                                           double waterDensityGCM3 ) const
{
    double trueVerticalDepth    = m_intersectionTVDs[intersectionIdx];
    double effectiveDepthMeters = trueVerticalDepth + m_wellPathGeometry->rkbDiff();
    return hydroStaticPorePressureAtDepth( effectiveDepthMeters, waterDensityGCM3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::hydroStaticPorePressureForSegment( size_t intersectionIdx, double waterDensityGCM3 ) const
{
    cvf::Vec3f centroid             = cellCentroid( intersectionIdx );
    double     trueVerticalDepth    = -centroid.z();
    double     effectiveDepthMeters = trueVerticalDepth + m_wellPathGeometry->rkbDiff();
    return hydroStaticPorePressureAtDepth( effectiveDepthMeters, waterDensityGCM3 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::hydroStaticPorePressureAtDepth( double effectiveDepthMeters, double waterDensityGCM3 )
{
    double hydroStaticPorePressurePascal = effectiveDepthMeters * GRAVITY_ACCEL * waterDensityGCM3 * 1000;
    double hydroStaticPorePressureBar    = pascalToBar( hydroStaticPorePressurePascal );
    return hydroStaticPorePressureBar;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechWellLogExtractor::wbsCurveValuesAtMsl() const
{
    double waterDensityGCM3 = m_userDefinedValues.at( RigWbsParameter::waterDensity() );
    double waterDepth       = std::abs( m_wellPathGeometry->wellPathPoints().front().z() );

    double rkbDiff = m_wellPathGeometry->rkbDiff();
    if ( rkbDiff == std::numeric_limits<double>::infinity() )
    {
        rkbDiff = 0.0;
    }

    if ( waterDepth + rkbDiff < 1.0e-8 )
    {
        return waterDensityGCM3;
    }

    return waterDensityGCM3 * waterDepth / ( waterDepth + rkbDiff );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechWellLogExtractor::isValid( double value )
{
    return value != std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigGeoMechWellLogExtractor::isValid( float value )
{
    return value != std::numeric_limits<float>::infinity();
}
