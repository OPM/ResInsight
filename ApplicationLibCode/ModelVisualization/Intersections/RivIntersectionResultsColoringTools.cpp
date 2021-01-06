/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RivIntersectionResultsColoringTools.h"

#include "RiuGeoMechXfTensorResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersection.h"
#include "RimIntersectionResultDefinition.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessorFactory.h"

#include "RivScalarMapperUtils.h"
#include "RivTernaryTextureCoordsCreator.h"

#include "RiaOffshoreSphericalCoords.h"

#include "cvfGeometryTools.h"
#include "cvfStructGridGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculateIntersectionResultColors(
    size_t                                    timeStepIndex,
    bool                                      useSeparateIntersectionResDefTimeStep,
    RimIntersection*                          rimIntersectionHandle,
    const RivIntersectionGeometryGeneratorIF* intersectionGeomGenIF,
    const cvf::ScalarMapper*                  explicitScalarColorMapper,
    const RivTernaryScalarMapper*             explicitTernaryColorMapper,
    cvf::Part*                                intersectionFacesPart,
    cvf::Vec2fArray*                          intersectionFacesTextureCoords )
{
    if ( !intersectionGeomGenIF || !intersectionGeomGenIF->isAnyGeometryPresent() ) return;

    RimGridView* gridView = nullptr;
    rimIntersectionHandle->firstAncestorOrThisOfType( gridView );

    if ( !gridView ) return;

    bool isLightingDisabled = gridView->isLightingDisabled();

    RimEclipseResultDefinition*   eclipseResDef      = nullptr;
    RimGeoMechResultDefinition*   geomResultDef      = nullptr;
    const cvf::ScalarMapper*      scalarColorMapper  = explicitScalarColorMapper;
    const RivTernaryScalarMapper* ternaryColorMapper = explicitTernaryColorMapper;

    // Separate intersection result

    RimIntersectionResultDefinition* sepResDef = rimIntersectionHandle->activeSeparateResultDefinition();
    if ( sepResDef && sepResDef->activeCase() )
    {
        if ( sepResDef->isEclipseResultDefinition() )
        {
            eclipseResDef = sepResDef->eclipseResultDefinition();
        }
        else
        {
            geomResultDef = sepResDef->geoMechResultDefinition();
        }

        if ( !scalarColorMapper ) scalarColorMapper = sepResDef->regularLegendConfig()->scalarMapper();
        if ( !ternaryColorMapper ) ternaryColorMapper = sepResDef->ternaryLegendConfig()->scalarMapper();
        if ( useSeparateIntersectionResDefTimeStep )
        {
            timeStepIndex = sepResDef->timeStep();
        }
    }

    // Ordinary result

    if ( !eclipseResDef && !geomResultDef )
    {
        RimEclipseView* eclipseView = nullptr;
        rimIntersectionHandle->firstAncestorOrThisOfType( eclipseView );

        if ( eclipseView )
        {
            eclipseResDef = eclipseView->cellResult();
            if ( !scalarColorMapper ) scalarColorMapper = eclipseView->cellResult()->legendConfig()->scalarMapper();
            if ( !ternaryColorMapper )
                ternaryColorMapper = eclipseView->cellResult()->ternaryLegendConfig()->scalarMapper();
        }

        RimGeoMechView* geoView;
        rimIntersectionHandle->firstAncestorOrThisOfType( geoView );

        if ( geoView )
        {
            geomResultDef = geoView->cellResult();
            if ( !scalarColorMapper ) scalarColorMapper = geoView->cellResult()->legendConfig()->scalarMapper();
        }
    }

    if ( eclipseResDef )
    {
        if ( eclipseResDef->isTernarySaturationSelected() )
        {
            RivIntersectionResultsColoringTools::updateEclipseTernaryCellResultColors( eclipseResDef,
                                                                                       ternaryColorMapper,
                                                                                       timeStepIndex,
                                                                                       isLightingDisabled,
                                                                                       intersectionGeomGenIF
                                                                                           ->triangleToCellIndex(),
                                                                                       intersectionFacesPart,
                                                                                       intersectionFacesTextureCoords );
        }
        else
        {
            RivIntersectionResultsColoringTools::updateEclipseCellResultColors( eclipseResDef,
                                                                                scalarColorMapper,
                                                                                timeStepIndex,
                                                                                isLightingDisabled,
                                                                                intersectionGeomGenIF->triangleToCellIndex(),
                                                                                intersectionFacesPart,
                                                                                intersectionFacesTextureCoords );
        }
    }
    else if ( geomResultDef )
    {
        RivIntersectionResultsColoringTools::updateGeoMechCellResultColors( geomResultDef,
                                                                            timeStepIndex,
                                                                            scalarColorMapper,
                                                                            isLightingDisabled,
                                                                            intersectionGeomGenIF,
                                                                            intersectionFacesPart,
                                                                            intersectionFacesTextureCoords );
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::updateEclipseCellResultColors( const RimEclipseResultDefinition* eclipseResDef,
                                                                         const cvf::ScalarMapper* scalarColorMapper,
                                                                         size_t                   timeStepIndex,
                                                                         bool                     isLightingDisabled,
                                                                         const std::vector<size_t>& triangleToCellIndexMapping,
                                                                         cvf::Part*       intersectionFacesPart,
                                                                         cvf::Vec2fArray* intersectionFacesTextureCoords )
{
    RigEclipseCaseData* eclipseCaseData = eclipseResDef->eclipseCase()->eclipseCaseData();

    cvf::ref<RigResultAccessor> resultAccessor;

    if ( !RiaDefines::isPerCellFaceResult( eclipseResDef->resultVariable() ) )

    {
        resultAccessor =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData, 0, timeStepIndex, eclipseResDef );
    }

    if ( resultAccessor.isNull() )
    {
        resultAccessor = new RigHugeValResultAccessor;
    }

    RivIntersectionResultsColoringTools::calculateEclipseTextureCoordinates( intersectionFacesTextureCoords,
                                                                             triangleToCellIndexMapping,
                                                                             resultAccessor.p(),
                                                                             scalarColorMapper );

    RivScalarMapperUtils::applyTextureResultsToPart( intersectionFacesPart,
                                                     intersectionFacesTextureCoords,
                                                     scalarColorMapper,
                                                     1.0,
                                                     caf::FC_NONE,
                                                     isLightingDisabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::updateEclipseTernaryCellResultColors(
    const RimEclipseResultDefinition* eclipseResDef,
    const RivTernaryScalarMapper*     ternaryColorMapper,
    size_t                            timeStepIndex,
    bool                              isLightingDisabled,
    const std::vector<size_t>&        triangleToCellIndexMapping,
    cvf::Part*                        intersectionFacesPart,
    cvf::Vec2fArray*                  intersectionFacesTextureCoords )
{
    RivTernaryTextureCoordsCreator texturer( eclipseResDef, ternaryColorMapper, timeStepIndex );

    texturer.createTextureCoords( intersectionFacesTextureCoords, triangleToCellIndexMapping );

    RivScalarMapperUtils::applyTernaryTextureResultsToPart( intersectionFacesPart,
                                                            intersectionFacesTextureCoords,
                                                            ternaryColorMapper,
                                                            1.0,
                                                            caf::FC_NONE,
                                                            isLightingDisabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::updateGeoMechCellResultColors( const RimGeoMechResultDefinition* geomResultDef,
                                                                         size_t                   timeStepIndex,
                                                                         const cvf::ScalarMapper* scalarColorMapper,
                                                                         bool                     isLightingDisabled,
                                                                         const RivIntersectionGeometryGeneratorIF* geomGenerator,
                                                                         cvf::Part*       intersectionFacesPart,
                                                                         cvf::Vec2fArray* intersectionFacesTextureCoords )
{
    RigGeoMechCaseData* caseData = nullptr;
    RigFemResultAddress resVarAddress;
    {
        caseData      = geomResultDef->ownerCaseData();
        resVarAddress = geomResultDef->resultAddress();
    }

    if ( !caseData ) return;

    const std::vector<size_t>&                       triangleToCellIdx = geomGenerator->triangleToCellIndex();
    const cvf::Vec3fArray*                           triangelVxes      = geomGenerator->triangleVxes();
    const std::vector<RivIntersectionVertexWeights>& vertexWeights =
        geomGenerator->triangleVxToCellCornerInterpolationWeights();

    if ( resVarAddress.resultPosType == RIG_ELEMENT )
    {
        const std::vector<float>& resultValues =
            caseData->femPartResults()->resultValues( resVarAddress, 0, (int)timeStepIndex );

        RivIntersectionResultsColoringTools::calculateElementBasedGeoMechTextureCoords( intersectionFacesTextureCoords,
                                                                                        resultValues,
                                                                                        triangleToCellIdx,
                                                                                        scalarColorMapper );
    }
    else if ( resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE )
    {
        // Special direction sensitive result calculation

        if ( resVarAddress.componentName == "Pazi" || resVarAddress.componentName == "Pinc" )
        {
            RivIntersectionResultsColoringTools::calculatePlaneAngleTextureCoords( intersectionFacesTextureCoords,
                                                                                   triangelVxes,
                                                                                   resVarAddress,
                                                                                   scalarColorMapper );
        }
        else
        {
            RivIntersectionResultsColoringTools::calculateGeoMechTensorXfTextureCoords( intersectionFacesTextureCoords,
                                                                                        triangelVxes,
                                                                                        vertexWeights,
                                                                                        caseData,
                                                                                        resVarAddress,
                                                                                        (int)timeStepIndex,
                                                                                        scalarColorMapper );
        }
    }
    else
    {
        // Do a "Hack" to show elm nodal and not nodal POR results

        if ( resVarAddress.resultPosType == RIG_NODAL && resVarAddress.fieldName == "POR-Bar" )
        {
            resVarAddress.resultPosType = RIG_ELEMENT_NODAL;
        }

        const std::vector<float>& resultValues =
            caseData->femPartResults()->resultValues( resVarAddress, 0, (int)timeStepIndex );

        RigFemPart* femPart              = caseData->femParts()->part( 0 );
        bool        isElementNodalResult = !( resVarAddress.resultPosType == RIG_NODAL );

        RivIntersectionResultsColoringTools::calculateNodeOrElementNodeBasedGeoMechTextureCoords( intersectionFacesTextureCoords,
                                                                                                  vertexWeights,
                                                                                                  resultValues,
                                                                                                  isElementNodalResult,
                                                                                                  femPart,
                                                                                                  scalarColorMapper );
    }

    RivScalarMapperUtils::applyTextureResultsToPart( intersectionFacesPart,
                                                     intersectionFacesTextureCoords,
                                                     scalarColorMapper,
                                                     1.0,
                                                     caf::FC_NONE,
                                                     isLightingDisabled );
}

//--------------------------------------------------------------------------------------------------
/// Calculates the texture coordinates in a "nearly" one dimensional texture.
/// Undefined values are coded with a y-texturecoordinate value of 1.0 instead of the normal 0.5
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculateEclipseTextureCoordinates( cvf::Vec2fArray*           textureCoords,
                                                                              const std::vector<size_t>& triangleToCellIdxMap,
                                                                              const RigResultAccessor* resultAccessor,
                                                                              const cvf::ScalarMapper* mapper )
{
    if ( !resultAccessor ) return;

    size_t numVertices = triangleToCellIdxMap.size() * 3;

    textureCoords->resize( numVertices );
    cvf::Vec2f* rawPtr = textureCoords->ptr();

    int triangleCount = static_cast<int>( triangleToCellIdxMap.size() );

#pragma omp parallel for
    for ( int tIdx = 0; tIdx < triangleCount; tIdx++ )
    {
        double     cellScalarValue = resultAccessor->cellScalarGlobIdx( triangleToCellIdxMap[tIdx] );
        cvf::Vec2f texCoord        = mapper->mapToTextureCoord( cellScalarValue );
        if ( cellScalarValue == HUGE_VAL || cellScalarValue != cellScalarValue ) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
        }

        size_t j;
        for ( j = 0; j < 3; j++ )
        {
            rawPtr[tIdx * 3 + j] = texCoord;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculateElementBasedGeoMechTextureCoords( cvf::Vec2fArray* textureCoords,
                                                                                     const std::vector<float>& resultValues,
                                                                                     const std::vector<size_t>& triangleToCellIdx,
                                                                                     const cvf::ScalarMapper* mapper )
{
    textureCoords->resize( triangleToCellIdx.size() * 3 );

    if ( resultValues.size() == 0 )
    {
        textureCoords->setAll( cvf::Vec2f( 0.0, 1.0f ) );
    }
    else
    {
        cvf::Vec2f* rawPtr = textureCoords->ptr();

        for ( size_t triangleIdx = 0; triangleIdx < triangleToCellIdx.size(); triangleIdx++ )
        {
            size_t resIdx   = triangleToCellIdx[triangleIdx];
            float  resValue = resultValues[resIdx];

            size_t triangleVxIdx = triangleIdx * 3;

            if ( resValue == HUGE_VAL || resValue != resValue ) // a != a is true for NAN's
            {
                rawPtr[triangleVxIdx][1]     = 1.0f;
                rawPtr[triangleVxIdx + 1][1] = 1.0f;
                rawPtr[triangleVxIdx + 2][1] = 1.0f;
            }
            else
            {
                rawPtr[triangleVxIdx]     = mapper->mapToTextureCoord( resValue );
                rawPtr[triangleVxIdx + 1] = mapper->mapToTextureCoord( resValue );
                rawPtr[triangleVxIdx + 2] = mapper->mapToTextureCoord( resValue );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculateGeoMechTensorXfTextureCoords(
    cvf::Vec2fArray*                                 textureCoords,
    const cvf::Vec3fArray*                           triangelVertices,
    const std::vector<RivIntersectionVertexWeights>& vertexWeights,
    RigGeoMechCaseData*                              caseData,
    const RigFemResultAddress&                       resVarAddress,
    int                                              timeStepIdx,
    const cvf::ScalarMapper*                         mapper )
{
    RiuGeoMechXfTensorResultAccessor accessor( caseData->femPartResults(), resVarAddress, timeStepIdx );

    textureCoords->resize( vertexWeights.size() );
    cvf::Vec2f* rawPtr   = textureCoords->ptr();
    int         vxCount  = static_cast<int>( vertexWeights.size() );
    int         triCount = vxCount / 3;

#pragma omp parallel for schedule( dynamic )
    for ( int triangleIdx = 0; triangleIdx < triCount; ++triangleIdx )
    {
        int   triangleVxStartIdx = triangleIdx * 3;
        float values[3];

        accessor.calculateInterpolatedValue( &( ( *triangelVertices )[triangleVxStartIdx] ),
                                             &( vertexWeights[triangleVxStartIdx] ),
                                             values );

        rawPtr[triangleVxStartIdx + 0] = ( values[0] != std::numeric_limits<float>::infinity() )
                                             ? mapper->mapToTextureCoord( values[0] )
                                             : cvf::Vec2f( 0.0f, 1.0f );
        rawPtr[triangleVxStartIdx + 1] = ( values[1] != std::numeric_limits<float>::infinity() )
                                             ? mapper->mapToTextureCoord( values[1] )
                                             : cvf::Vec2f( 0.0f, 1.0f );
        rawPtr[triangleVxStartIdx + 2] = ( values[2] != std::numeric_limits<float>::infinity() )
                                             ? mapper->mapToTextureCoord( values[2] )
                                             : cvf::Vec2f( 0.0f, 1.0f );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionResultsColoringTools::calculatePlaneAngleTextureCoords( cvf::Vec2fArray*           textureCoords,
                                                                            const cvf::Vec3fArray*     triangelVertices,
                                                                            const RigFemResultAddress& resVarAddress,
                                                                            const cvf::ScalarMapper*   mapper )
{
    textureCoords->resize( triangelVertices->size() );
    cvf::Vec2f* rawPtr   = textureCoords->ptr();
    int         vxCount  = static_cast<int>( triangelVertices->size() );
    int         triCount = vxCount / 3;

    std::function<float( const RiaOffshoreSphericalCoords& )> operation;
    if ( resVarAddress.componentName == "Pazi" )
    {
        operation = []( const RiaOffshoreSphericalCoords& sphCoord ) { return (float)sphCoord.azi(); };
    }
    else if ( resVarAddress.componentName == "Pinc" )
    {
        operation = []( const RiaOffshoreSphericalCoords& sphCoord ) { return (float)sphCoord.inc(); };
    }

#pragma omp parallel for schedule( dynamic )
    for ( int triangleIdx = 0; triangleIdx < triCount; ++triangleIdx )
    {
        int triangleVxStartIdx = triangleIdx * 3;

        const cvf::Vec3f* triangle = &( ( *triangelVertices )[triangleVxStartIdx] );
        cvf::Mat3f        rotMx =
            cvf::GeometryTools::computePlaneHorizontalRotationMx( triangle[1] - triangle[0], triangle[2] - triangle[0] );

        RiaOffshoreSphericalCoords sphCoord(
            cvf::Vec3f( rotMx.rowCol( 2, 0 ), rotMx.rowCol( 2, 1 ), rotMx.rowCol( 2, 2 ) ) ); // Use Ez from the matrix
                                                                                              // as plane normal

        float      angle    = cvf::Math::toDegrees( operation( sphCoord ) );
        cvf::Vec2f texCoord = ( angle != std::numeric_limits<float>::infinity() ) ? mapper->mapToTextureCoord( angle )
                                                                                  : cvf::Vec2f( 0.0f, 1.0f );
        rawPtr[triangleVxStartIdx + 0] = texCoord;
        rawPtr[triangleVxStartIdx + 1] = texCoord;
        rawPtr[triangleVxStartIdx + 2] = texCoord;
    }
}
