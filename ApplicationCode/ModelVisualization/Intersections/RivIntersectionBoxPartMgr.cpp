/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxPartMgr.h"

#include "RigCaseCellResultsData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionResultDefinition.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RivIntersectionBoxSourceInfo.h"
#include "RivIntersectionPartMgr.h"
#include "RivMeshLinesSourceInfo.h"
#include "RivPartPriority.h"
#include "RivResultToTextureMapper.h"
#include "RivScalarMapperUtils.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryTextureCoordsCreator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfRenderStateDepth.h"
#include "cvfRenderStatePoint.h"
#include "cvfRenderState_FF.h"
#include "cvfStructGridGeometryGenerator.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxPartMgr::RivIntersectionBoxPartMgr( RimIntersectionBox* intersectionBox )
    : m_rimIntersectionBox( intersectionBox )
    , m_defaultColor( cvf::Color3::WHITE )
{
    CVF_ASSERT( m_rimIntersectionBox );

    m_intersectionBoxFacesTextureCoords = new cvf::Vec2fArray;

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = intersectionBox->createHexGridInterface();
    m_intersectionBoxGenerator = new RivIntersectionBoxGeometryGenerator( m_rimIntersectionBox, hexGrid.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::applySingleColorEffect()
{
    m_defaultColor = cvf::Color3f::OLIVE; // m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::updateCellResultColor( size_t timeStepIndex )
{
    RivIntersectionBoxPartMgr::updateCellResultColorStatic( timeStepIndex,
                                                            m_rimIntersectionBox,
                                                            m_intersectionBoxGenerator.p(),
                                                            m_intersectionBoxFaces.p(),
                                                            m_intersectionBoxFacesTextureCoords.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::updateCellResultColorStatic(
    size_t                                    timeStepIndex,
    RimIntersectionHandle*                    rimIntersectionHandle,
    const RivIntersectionGeometryGeneratorIF* intersectionGeomGenIF,
    cvf::Part*                                intersectionFacesPart,
    cvf::Vec2fArray*                          intersectionFacesTextureCoords )
{
    if ( !intersectionGeomGenIF->isAnyGeometryPresent() ) return;

    RimGridView* gridView = nullptr;
    rimIntersectionHandle->firstAncestorOrThisOfType( gridView );

    if ( !gridView ) return;

    bool isLightingDisabled = gridView->isLightingDisabled();

    RimEclipseResultDefinition*   eclipseResDef      = nullptr;
    RimGeoMechResultDefinition*   geomResultDef      = nullptr;
    const cvf::ScalarMapper*      scalarColorMapper  = nullptr;
    const RivTernaryScalarMapper* ternaryColorMapper = nullptr;

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

        scalarColorMapper  = sepResDef->regularLegendConfig()->scalarMapper();
        ternaryColorMapper = sepResDef->ternaryLegendConfig()->scalarMapper();
        timeStepIndex      = sepResDef->timeStep();
    }

    // Ordinary result

    if ( !eclipseResDef && !geomResultDef )
    {
        RimEclipseView* eclipseView = nullptr;
        rimIntersectionHandle->firstAncestorOrThisOfType( eclipseView );

        if ( eclipseView )
        {
            eclipseResDef      = eclipseView->cellResult();
            scalarColorMapper  = eclipseView->cellResult()->legendConfig()->scalarMapper();
            ternaryColorMapper = eclipseView->cellResult()->ternaryLegendConfig()->scalarMapper();
            timeStepIndex      = eclipseView->currentTimeStep();
        }

        RimGeoMechView* geoView;
        rimIntersectionHandle->firstAncestorOrThisOfType( geoView );

        if ( geoView )
        {
            geomResultDef     = geoView->cellResult();
            scalarColorMapper = geoView->cellResult()->legendConfig()->scalarMapper();
            timeStepIndex     = geoView->currentTimeStep();
        }
    }

    if ( eclipseResDef )
    {
        if ( eclipseResDef->isTernarySaturationSelected() )
        {
            updateEclipseTernaryCellResultColors( eclipseResDef,
                                                  ternaryColorMapper,
                                                  timeStepIndex,
                                                  isLightingDisabled,
                                                  intersectionGeomGenIF->triangleToCellIndex(),
                                                  intersectionFacesPart,
                                                  intersectionFacesTextureCoords );
        }
        else
        {
            updateEclipseCellResultColors( eclipseResDef,
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
        updateGeoMechCellResultColors( geomResultDef,
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
void RivIntersectionBoxPartMgr::updateEclipseCellResultColors( const RimEclipseResultDefinition* eclipseResDef,
                                                               const cvf::ScalarMapper*          scalarColorMapper,
                                                               size_t                            timeStepIndex,
                                                               bool                              isLightingDisabled,
                                                               const std::vector<size_t>& triangleToCellIndexMapping,
                                                               cvf::Part*                 intersectionFacesPart,
                                                               cvf::Vec2fArray* intersectionFacesTextureCoords )
{
    RigEclipseCaseData* eclipseCaseData = eclipseResDef->eclipseCase()->eclipseCaseData();

    cvf::ref<RigResultAccessor> resultAccessor;

    if ( RiaDefines::isPerCellFaceResult( eclipseResDef->resultVariable() ) )
    {
        resultAccessor = new RigHugeValResultAccessor;
    }
    else
    {
        resultAccessor = RigResultAccessorFactory::createFromResultDefinition( eclipseCaseData,
                                                                               0,
                                                                               timeStepIndex,
                                                                               eclipseResDef );
    }

    RivIntersectionPartMgr::calculateEclipseTextureCoordinates( intersectionFacesTextureCoords,
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
void RivIntersectionBoxPartMgr::updateEclipseTernaryCellResultColors( const RimEclipseResultDefinition* eclipseResDef,
                                                                      const RivTernaryScalarMapper* ternaryColorMapper,
                                                                      size_t                        timeStepIndex,
                                                                      bool                          isLightingDisabled,
                                                                      const std::vector<size_t>& triangleToCellIndexMapping,
                                                                      cvf::Part*                 intersectionFacesPart,
                                                                      cvf::Vec2fArray* intersectionFacesTextureCoords )
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
void RivIntersectionBoxPartMgr::updateGeoMechCellResultColors( const RimGeoMechResultDefinition* geomResultDef,
                                                               size_t                            timeStepIndex,
                                                               const cvf::ScalarMapper*          scalarColorMapper,
                                                               bool                              isLightingDisabled,
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
        const std::vector<float>& resultValues = caseData->femPartResults()->resultValues( resVarAddress,
                                                                                           0,
                                                                                           (int)timeStepIndex );

        RivIntersectionPartMgr::calculateElementBasedGeoMechTextureCoords( intersectionFacesTextureCoords,
                                                                           resultValues,
                                                                           triangleToCellIdx,
                                                                           scalarColorMapper );
    }
    else if ( resVarAddress.resultPosType == RIG_ELEMENT_NODAL_FACE )
    {
        // Special direction sensitive result calculation

        if ( resVarAddress.componentName == "Pazi" || resVarAddress.componentName == "Pinc" )
        {
            RivIntersectionPartMgr::calculatePlaneAngleTextureCoords( intersectionFacesTextureCoords,
                                                                      triangelVxes,
                                                                      resVarAddress,
                                                                      scalarColorMapper );
        }
        else
        {
            RivIntersectionPartMgr::calculateGeoMechTensorXfTextureCoords( intersectionFacesTextureCoords,
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

        const std::vector<float>& resultValues = caseData->femPartResults()->resultValues( resVarAddress,
                                                                                           0,
                                                                                           (int)timeStepIndex );

        RigFemPart* femPart              = caseData->femParts()->part( 0 );
        bool        isElementNodalResult = !( resVarAddress.resultPosType == RIG_NODAL );

        RivIntersectionPartMgr::calculateNodeOrElementNodeBasedGeoMechTextureCoords( intersectionFacesTextureCoords,
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
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::generatePartGeometry()
{
    bool useBufferObjects = true;
    // Surface geometry
    {
        cvf::ref<cvf::DrawableGeo> geo = m_intersectionBoxGenerator->generateSurface();
        if ( geo.notNull() )
        {
            geo->computeNormals();

            if ( useBufferObjects )
            {
                geo->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Intersection Box" );
            part->setDrawable( geo.p() );

            // Set mapping from triangle face index to cell index
            cvf::ref<RivIntersectionBoxSourceInfo> si = new RivIntersectionBoxSourceInfo( m_intersectionBoxGenerator.p() );
            part->setSourceInfo( si.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellFaceBit );
            part->setPriority( RivPartPriority::PartType::Intersection );

            m_intersectionBoxFaces = part;
        }
    }

    // Mesh geometry
    {
        cvf::ref<cvf::DrawableGeo> geoMesh = m_intersectionBoxGenerator->createMeshDrawable();
        if ( geoMesh.notNull() )
        {
            if ( useBufferObjects )
            {
                geoMesh->setRenderMode( cvf::DrawableGeo::BUFFER_OBJECT );
            }

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName( "Intersection box mesh" );
            part->setDrawable( geoMesh.p() );

            part->updateBoundingBox();
            part->setEnableMask( intersectionCellMeshBit );
            part->setPriority( RivPartPriority::PartType::MeshLines );

            part->setSourceInfo( new RivMeshLinesSourceInfo( m_rimIntersectionBox ) );

            m_intersectionBoxGridLines = part;
        }
    }

    updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::updatePartEffect()
{
    // Set deCrossSection effect
    caf::SurfaceEffectGenerator geometryEffgen( m_defaultColor, caf::PO_1 );

    cvf::ref<cvf::Effect> geometryOnlyEffect = geometryEffgen.generateCachedEffect();

    if ( m_intersectionBoxFaces.notNull() )
    {
        m_intersectionBoxFaces->setEffect( geometryOnlyEffect.p() );
    }

    // Update mesh colors as well, in case of change
    // RiaPreferences* prefs = RiaApplication::instance()->preferences();

    cvf::ref<cvf::Effect>    eff;
    caf::MeshEffectGenerator CrossSectionEffGen( cvf::Color3::WHITE ); // prefs->defaultCrossSectionGridLineColors());
    eff = CrossSectionEffGen.generateCachedEffect();

    if ( m_intersectionBoxGridLines.notNull() )
    {
        m_intersectionBoxGridLines->setEffect( eff.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::appendNativeCrossSectionFacesToModel( cvf::ModelBasicList* model,
                                                                      cvf::Transform*      scaleTransform )
{
    if ( m_intersectionBoxFaces.isNull() && m_intersectionBoxGridLines.isNull() )
    {
        generatePartGeometry();
    }

    if ( m_intersectionBoxFaces.notNull() )
    {
        m_intersectionBoxFaces->setTransform( scaleTransform );
        model->addPart( m_intersectionBoxFaces.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxPartMgr::appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
{
    if ( m_intersectionBoxFaces.isNull() && m_intersectionBoxGridLines.isNull() )
    {
        generatePartGeometry();
    }

    if ( m_intersectionBoxGridLines.notNull() )
    {
        m_intersectionBoxGridLines->setTransform( scaleTransform );
        model->addPart( m_intersectionBoxGridLines.p() );
    }
}
