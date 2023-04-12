/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RivSimWellPipesPartMgr.h"

#include "RiaColorTables.h"
#include "RiaExtractionTools.h"
#include "RiaPreferences.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigMswCenterLineCalculator.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellResultFrame.h"
#include "RigWellResultPoint.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimVirtualPerforationResults.h"

#include "RivPipeGeometryGenerator.h"
#include "RivSectionFlattener.h"
#include "RivSimWellConnectionSourceInfo.h"
#include "RivSimWellPipeSourceInfo.h"
#include "RivWellConnectionFactorGeometryGenerator.h"
#include "RivWellConnectionSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryUtils.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"

#include <numbers>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::RivSimWellPipesPartMgr( RimSimWellInView* well )
    : m_simWellInView( well )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivSimWellPipesPartMgr::~RivSimWellPipesPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RivSimWellPipesPartMgr::viewWithSettings()
{
    Rim3dView* view = nullptr;
    if ( m_simWellInView ) m_simWellInView->firstAncestorOrThisOfType( view );

    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                size_t                            frameIndex,
                                                                const caf::DisplayCoordTransform* displayXf )
{
    if ( !viewWithSettings() ) return;

    if ( !m_simWellInView->isWellPipeVisible( frameIndex ) ) return;

    buildWellPipeParts( displayXf, false, 0.0, -1, frameIndex );

    std::list<RivPipeBranchData>::iterator it;
    for ( it = m_wellBranches.begin(); it != m_wellBranches.end(); ++it )
    {
        if ( it->m_surfacePart.notNull() )
        {
            model->addPart( it->m_surfacePart.p() );
        }

        if ( it->m_centerLinePart.notNull() )
        {
            model->addPart( it->m_centerLinePart.p() );
        }

        if ( it->m_connectionFactorsPart.notNull() )
        {
            model->addPart( it->m_connectionFactorsPart.p() );
        }

        for ( auto valvePart : it->m_valveParts )
        {
            model->addPart( valvePart.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                                         size_t                            frameIndex,
                                                                         const caf::DisplayCoordTransform* displayXf,
                                                                         double flattenedIntersectionExtentLength,
                                                                         int    branchIndex )
{
    if ( !viewWithSettings() ) return;

    if ( !m_simWellInView->isWellPipeVisible( frameIndex ) ) return;

    buildWellPipeParts( displayXf, true, flattenedIntersectionExtentLength, branchIndex, frameIndex );

    std::list<RivPipeBranchData>::iterator it;
    for ( it = m_wellBranches.begin(); it != m_wellBranches.end(); ++it )
    {
        if ( it->m_surfacePart.notNull() )
        {
            model->addPart( it->m_surfacePart.p() );
        }

        if ( it->m_centerLinePart.notNull() )
        {
            model->addPart( it->m_centerLinePart.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::buildWellPipeParts( const caf::DisplayCoordTransform* displayXf,
                                                 bool                              doFlatten,
                                                 double                            flattenedIntersectionExtentLength,
                                                 int                               branchIndex,
                                                 size_t                            frameIndex )
{
    if ( !this->viewWithSettings() ) return;

    m_wellBranches.clear();
    m_flattenedBranchWellHeadOffsets.clear();

    auto createSimWells = []( RimSimWellInView* simWellInView ) -> std::vector<SimulationWellCellBranch>
    {
        std::vector<SimulationWellCellBranch> simWellBranches;
        const RigSimWellData*                 simWellData = simWellInView->simWellData();
        if ( simWellData && simWellData->isMultiSegmentWell() )
        {
            simWellBranches = RigMswCenterLineCalculator::calculateMswWellPipeGeometry( simWellInView );
        }
        else
        {
            simWellBranches = RigSimulationWellCenterLineCalculator::calculateWellPipeStaticCenterline( simWellInView );
        }

        return simWellBranches;
    };

    auto simWells                   = createSimWells( m_simWellInView );
    const auto& [coords, wellCells] = RigSimulationWellCenterLineCalculator::extractBranchData( simWells );

    auto pipeBranchesCLCoords = coords;
    auto pipeBranchesCellIds  = wellCells;

    double pipeRadius              = m_simWellInView->pipeRadius();
    int    crossSectionVertexCount = m_simWellInView->pipeCrossSectionVertexCount();

    // Take branch selection into account
    size_t branchIdxStart = 0;
    size_t branchIdxStop  = pipeBranchesCellIds.size();
    if ( pipeBranchesCLCoords.size() > 1 )
    {
        if ( branchIndex >= 0 && branchIndex < static_cast<int>( branchIdxStop ) )
        {
            branchIdxStart = branchIndex;
            branchIdxStop  = branchIdxStart + 1;
        }
    }

    cvf::Vec3d flattenedStartOffset = cvf::Vec3d::ZERO;
    if ( pipeBranchesCLCoords.size() > branchIdxStart && !pipeBranchesCLCoords[branchIdxStart].empty() )
    {
        flattenedStartOffset = { 0.0, 0.0, pipeBranchesCLCoords[branchIdxStart][0].z() };
    }

    for ( size_t brIdx = branchIdxStart; brIdx < branchIdxStop; ++brIdx )
    {
        cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo( m_simWellInView, brIdx );

        m_wellBranches.emplace_back();
        RivPipeBranchData& pbd = m_wellBranches.back();

        pbd.m_cellIds = pipeBranchesCellIds[brIdx];

        pbd.m_pipeGeomGenerator = new RivPipeGeometryGenerator;

        pbd.m_pipeGeomGenerator->setRadius( pipeRadius );
        pbd.m_pipeGeomGenerator->setCrossSectionVertexCount( crossSectionVertexCount );

        cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray;
        cvfCoords->assign( pipeBranchesCLCoords[brIdx] );

        flattenedStartOffset.z() = pipeBranchesCLCoords[brIdx][0].z();

        m_flattenedBranchWellHeadOffsets.push_back( flattenedStartOffset.x() );

        if ( doFlatten )
        {
            std::vector<cvf::Mat4d> flatningCSs = RivSectionFlattener::calculateFlatteningCSsForPolyline( pipeBranchesCLCoords[brIdx],
                                                                                                          cvf::Vec3d::Z_AXIS,
                                                                                                          flattenedStartOffset,
                                                                                                          &flattenedStartOffset );
            for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
            {
                ( *cvfCoords )[cIdx] = ( ( *cvfCoords )[cIdx] ).getTransformedPoint( flatningCSs[cIdx] );
                ( *cvfCoords )[cIdx] = displayXf->scaleToDisplaySize( ( *cvfCoords )[cIdx] );
            }
        }
        else
        {
            // Scale the centerline coordinates using the Z-scale transform of the grid and correct for the display offset.

            for ( size_t cIdx = 0; cIdx < cvfCoords->size(); ++cIdx )
            {
                ( *cvfCoords )[cIdx] = displayXf->transformToDisplayCoord( ( *cvfCoords )[cIdx] );
            }
        }

        pbd.m_pipeGeomGenerator->setPipeCenterCoords( cvfCoords.p() );
        pbd.m_surfaceDrawable    = pbd.m_pipeGeomGenerator->createPipeSurface();
        pbd.m_centerLineDrawable = pbd.m_pipeGeomGenerator->createCenterLine();

        if ( pbd.m_surfaceDrawable.notNull() )
        {
            pbd.m_surfacePart = new cvf::Part( 0, "SimWellPipeSurface" );
            pbd.m_surfacePart->setDrawable( pbd.m_surfaceDrawable.p() );

            caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( m_simWellInView->wellPipeColor() ), caf::PO_1 );
            cvf::ref<cvf::Effect>       eff = surfaceGen.generateCachedEffect();

            pbd.m_surfacePart->setEffect( eff.p() );

            pbd.m_surfacePart->setSourceInfo( sourceInfo.p() );
        }

        if ( pbd.m_centerLineDrawable.notNull() )
        {
            pbd.m_centerLinePart = new cvf::Part( 0, "SimWellPipeCenterLine" );
            pbd.m_centerLinePart->setDrawable( pbd.m_centerLineDrawable.p() );

            caf::MeshEffectGenerator gen( m_simWellInView->wellPipeColor() );
            cvf::ref<cvf::Effect>    eff = gen.generateCachedEffect();

            pbd.m_centerLinePart->setEffect( eff.p() );
        }

        // Create slightly larger geometry for active (open) wells
        // This will avoid visual artifacts when two wells are located at the same position
        {
            pbd.m_pipeGeomGenerator->setRadius( pipeRadius * 1.1 );
            pbd.m_largeSurfaceDrawable = pbd.m_pipeGeomGenerator->createPipeSurface();
        }

        pbd.m_connectionFactorGeometryGenerator = nullptr;
        pbd.m_connectionFactorsPart             = nullptr;
        pbd.m_valveParts.clear();

        RimEclipseView* eclipseView = nullptr;
        m_simWellInView->firstAncestorOrThisOfType( eclipseView );

        appendVirtualConnectionFactorGeo( eclipseView, frameIndex, brIdx, displayXf, pipeRadius, pbd );
        appendValvesGeo( eclipseView, frameIndex, brIdx, displayXf, pipeRadius, pbd );

        if ( doFlatten ) flattenedStartOffset += { 2 * flattenedIntersectionExtentLength, 0.0, 0.0 };
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendVirtualConnectionFactorGeo( const RimEclipseView*             eclipseView,
                                                               size_t                            frameIndex,
                                                               size_t                            brIdx,
                                                               const caf::DisplayCoordTransform* displayXf,
                                                               double                            pipeRadius,
                                                               RivPipeBranchData&                pbd )
{
    if ( eclipseView && eclipseView->isVirtualConnectionFactorGeometryVisible() )
    {
        RigSimWellData* simWellData = m_simWellInView->simWellData();

        if ( simWellData && simWellData->hasWellResult( frameIndex ) )
        {
            const RigWellResultFrame* wResFrame = simWellData->wellResultFrame( frameIndex );

            std::vector<CompletionVizData> completionVizDataItems;

            RimVirtualPerforationResults* virtualPerforationResult = eclipseView->virtualPerforationResult();
            {
                auto wellPaths = m_simWellInView->wellPipeBranches();

                const RigWellPath* wellPath = wellPaths[brIdx];

                RigEclipseWellLogExtractor* extractor = RiaExtractionTools::findOrCreateSimWellExtractor( m_simWellInView, wellPath );
                if ( extractor )
                {
                    std::vector<WellPathCellIntersectionInfo> wellPathCellIntersections = extractor->cellIntersectionInfosAlongWellPath();

                    for ( const auto& intersectionInfo : wellPathCellIntersections )
                    {
                        size_t                    globalCellIndex = intersectionInfo.globCellIndex;
                        const RigWellResultPoint* wResCell        = wResFrame->findResultCellWellHeadIncluded( 0, globalCellIndex );

                        if ( !wResCell || !wResCell->isValid() )
                        {
                            continue;
                        }

                        if ( !virtualPerforationResult->showConnectionFactorsOnClosedConnections() && !wResCell->isOpen() )
                        {
                            continue;
                        }

                        double startMD = intersectionInfo.startMD;
                        double endMD   = intersectionInfo.endMD;

                        double middleMD = ( startMD + endMD ) / 2.0;

                        cvf::Vec3d domainCoord = wellPath->interpolatedPointAlongWellPath( middleMD );

                        cvf::Vec3d p1;
                        cvf::Vec3d p2;
                        wellPath->twoClosestPoints( domainCoord, &p1, &p2 );

                        cvf::Vec3d direction = ( p2 - p1 ).getNormalized();

                        cvf::Vec3d anchor = displayXf->transformToDisplayCoord( domainCoord );
                        {
                            CompletionVizData data( anchor, direction, wResCell->connectionFactor(), globalCellIndex );

                            completionVizDataItems.push_back( data );
                        }
                    }
                }
            }

            if ( !completionVizDataItems.empty() )
            {
                double radius = pipeRadius * virtualPerforationResult->geometryScaleFactor();
                radius *= 2.0; // Enlarge the radius slightly to make the connection factor visible if geometry
                               // scale factor is set to 1.0

                pbd.m_connectionFactorGeometryGenerator = new RivWellConnectionFactorGeometryGenerator( completionVizDataItems, radius );

                cvf::ScalarMapper*  scalarMapper = virtualPerforationResult->legendConfig()->scalarMapper();
                cvf::ref<cvf::Part> part =
                    pbd.m_connectionFactorGeometryGenerator->createSurfacePart( scalarMapper, eclipseView->isLightingDisabled() );
                if ( part.notNull() )
                {
                    cvf::ref<RivSimWellConnectionSourceInfo> simWellSourceInfo =
                        new RivSimWellConnectionSourceInfo( m_simWellInView, pbd.m_connectionFactorGeometryGenerator.p() );
                    part->setSourceInfo( simWellSourceInfo.p() );
                }

                pbd.m_connectionFactorsPart = part;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::appendValvesGeo( const RimEclipseView*             eclipseView,
                                              size_t                            frameIndex,
                                              size_t                            brIdx,
                                              const caf::DisplayCoordTransform* displayXf,
                                              double                            wellPathRadius,
                                              RivPipeBranchData&                pbd )
{
    if ( !m_simWellInView || !m_simWellInView->isWellValvesVisible( frameIndex ) ) return;
    if ( !eclipseView || !eclipseView->ownerCase() ) return;

    const auto characteristicCellSize = eclipseView->ownerCase()->characteristicCellSize();
    const auto coords                 = pbd.m_pipeGeomGenerator->pipeCenterCoords();

    std::set<std::pair<size_t, size_t>> resultPointWithValve;
    for ( size_t i = 0; i < pbd.m_cellIds.size(); i++ )
    {
        const auto resultPoint = pbd.m_cellIds[i];

        if ( resultPointWithValve.contains( { resultPoint.gridIndex(), resultPoint.cellIndex() } ) ) continue;

        const auto   segmentStartCoord = coords->get( i );
        const auto   segmentEndCoord   = coords->get( i + 1 );
        const auto   segmentLength     = ( segmentEndCoord - segmentStartCoord ).length();
        const double valveLength       = characteristicCellSize * 0.2;

        // Insert valve geometry if the length of the segment is sufficient
        if ( resultPoint.isConnectedToValve() && ( valveLength * 2 < segmentLength ) )
        {
            resultPointWithValve.insert( { resultPoint.gridIndex(), resultPoint.cellIndex() } );

            const auto segmentDirection = ( segmentEndCoord - segmentStartCoord ).getNormalized();

            // A segment ends at the center of a simulation cell
            auto valveCenterCoord = ( segmentEndCoord + segmentStartCoord ) / 2.0;

            const cvf::Color3f valveColor                      = cvf::Color3f::ORANGE;
            const auto         measuredDepthsRelativeToStartMD = { 0.0, 1.0, valveLength - 1.0, valveLength };

            const auto outerValveRadius = wellPathRadius * 1.3;
            const auto radii            = { wellPathRadius, outerValveRadius, outerValveRadius, wellPathRadius };

            // The location of the valve is adjusted to locate the valve at the center of the segment
            double locationAdjustment = -valveLength / 2.0;

            std::vector<cvf::Vec3d> displayCoords;
            for ( const auto& mdRelativeToStart : measuredDepthsRelativeToStartMD )
            {
                displayCoords.push_back( valveCenterCoord + ( locationAdjustment + mdRelativeToStart ) * segmentDirection );
            }

            RivPipeGeometryGenerator::tubeWithCenterLinePartsAndVariableWidth( &pbd.m_valveParts, displayCoords, radii, valveColor );

            auto computeRotationAxisAndAngle = []( const cvf::Vec3f& direction )
            {
                // Compute upwards normal based on direction
                // Compute the rotation axis and angle between up vector and Z_AXIS

                cvf::Vec3f crossBetweenZAndDirection;
                crossBetweenZAndDirection.cross( cvf::Vec3f::Z_AXIS, cvf::Vec3f( direction ) );

                cvf::Vec3f upVector;
                upVector.cross( cvf::Vec3f( direction ), crossBetweenZAndDirection );

                cvf::Vec3f rotationAxis;
                rotationAxis.cross( upVector, cvf::Vec3f::Z_AXIS );
                upVector.normalize();

                float angle = cvf::Math::acos( upVector * cvf::Vec3f::Z_AXIS );

                return std::make_pair( rotationAxis, angle );
            };

            const auto& [rotationAxis, angle] = computeRotationAxisAndAngle( cvf::Vec3f( segmentDirection ) );

            // Add visualization of valves openings for segments close to horizontal segments
            if ( !std::isnan( angle ) && ( std::fabs( angle ) < ( std::numbers::pi / 2.0 ) ) )
            {
                cvf::GeometryBuilderDrawableGeo builder;

                const float bottomRadius    = wellPathRadius * 0.4f;
                const float topRadius       = bottomRadius;
                const float height          = outerValveRadius * 2.1f;
                const float topOffsetX      = 0.0f;
                const float topOffsetY      = 0.0f;
                const uint  numSlices       = 12;
                const bool  normalsOutwards = true;
                const bool  closedBot       = true;
                const bool  closedTop       = true;
                const uint  numPolysZDir    = 1;

                cvf::GeometryUtils::createObliqueCylinder( bottomRadius,
                                                           topRadius,
                                                           height,
                                                           topOffsetX,
                                                           topOffsetY,
                                                           numSlices,
                                                           normalsOutwards,
                                                           closedBot,
                                                           closedTop,
                                                           numPolysZDir,
                                                           &builder );

                // Move center of cylinder to origo
                builder.transformVertexRange( 0, builder.vertexCount() - 1, cvf::Mat4f::fromTranslation( cvf::Vec3f( 0.0, 0.0, -height / 2.0 ) ) );

                // Rotate cylinder to match the be normal to the well path direction
                const cvf::Mat4f rotMat = cvf::Mat4f::fromRotation( rotationAxis, -angle );
                builder.transformVertexRange( 0, builder.vertexCount() - 1, rotMat );

                // Move the cylinder to display coordinate location
                builder.transformVertexRange( 0, builder.vertexCount() - 1, cvf::Mat4f::fromTranslation( cvf::Vec3f( valveCenterCoord ) ) );

                auto drawableGeo = builder.drawableGeo();
                auto part        = new cvf::Part;
                part->setName( "RivPipeGeometryGenerator::surface" );
                part->setDrawable( drawableGeo.p() );

                caf::SurfaceEffectGenerator surfaceGen( cvf::Color4f( cvf::Color3f::BLACK ), caf::PO_1 );
                auto                        eff = surfaceGen.generateCachedEffect();

                part->setEffect( eff.p() );

                pbd.m_valveParts.push_back( part );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivSimWellPipesPartMgr::updatePipeResultColor( size_t frameIndex )
{
    if ( m_simWellInView == nullptr ) return;

    RigSimWellData* simWellData = m_simWellInView->simWellData();
    if ( simWellData == nullptr ) return;

    if ( !simWellData->hasWellResult( frameIndex ) ) return; // Or reset colors or something

    const double defaultState       = -0.1; // Closed set to -0.1 instead of 0.5 to workaround bug in the scalar mapper.
    const double producerState      = 1.5;
    const double waterInjectorState = 2.5;
    const double hcInjectorState    = 3.5;
    const double closedState        = 4.5;

    const RigWellResultFrame* wResFrame = simWellData->wellResultFrame( frameIndex );

    // Setup a scalar mapper
    cvf::ref<cvf::ScalarMapperDiscreteLinear> scalarMapper = new cvf::ScalarMapperDiscreteLinear;
    {
        cvf::Color3ubArray legendColors;
        legendColors.resize( 5 );
        legendColors[0] = cvf::Color3ub( m_simWellInView->wellPipeColor() );
        legendColors[1] = cvf::Color3::GREEN;
        legendColors[2] = cvf::Color3::BLUE;
        legendColors[3] = cvf::Color3::RED;
        legendColors[4] = cvf::Color3ub( RiaColorTables::undefinedCellColor() );
        scalarMapper->setColors( legendColors );
        scalarMapper->setRange( 0.0, 5.0 );
        scalarMapper->setLevelCount( 5, true );
    }

    caf::ScalarMapperEffectGenerator surfEffGen( scalarMapper.p(), caf::PO_1 );

    if ( viewWithSettings() && viewWithSettings()->isLightingDisabled() )
    {
        surfEffGen.disableLighting( true );
    }

    cvf::ref<cvf::Effect> scalarMapperSurfaceEffect = surfEffGen.generateUnCachedEffect();

    caf::ScalarMapperMeshEffectGenerator meshEffGen( scalarMapper.p() );
    cvf::ref<cvf::Effect>                scalarMapperMeshEffect = meshEffGen.generateUnCachedEffect();

    for ( auto& wellBranch : m_wellBranches )
    {
        std::vector<double> wellCellStates;
        wellCellStates.resize( wellBranch.m_cellIds.size(), defaultState );

        RimSimWellInViewCollection* wellColl = nullptr;
        if ( m_simWellInView )
        {
            m_simWellInView->firstAncestorOrThisOfType( wellColl );
        }

        if ( wellColl && wellColl->showConnectionStatusColors() )
        {
            const std::vector<RigWellResultPoint>& cellIds = wellBranch.m_cellIds;
            for ( size_t wcIdx = 0; wcIdx < cellIds.size(); ++wcIdx )
            {
                // we need a faster lookup, I guess
                const RigWellResultPoint* wResCell = nullptr;

                if ( cellIds[wcIdx].isCell() )
                {
                    wResCell = wResFrame->findResultCellWellHeadExcluded( cellIds[wcIdx].gridIndex(), cellIds[wcIdx].cellIndex() );
                }

                if ( wResCell )
                {
                    double cellState = defaultState;

                    if ( wResCell->isOpen() )
                    {
                        switch ( wResFrame->productionType() )
                        {
                            case RiaDefines::WellProductionType::PRODUCER:
                                cellState = producerState;
                                break;
                            case RiaDefines::WellProductionType::OIL_INJECTOR:
                                cellState = hcInjectorState;
                                break;
                            case RiaDefines::WellProductionType::GAS_INJECTOR:
                                cellState = hcInjectorState;
                                break;
                            case RiaDefines::WellProductionType::WATER_INJECTOR:
                                cellState = waterInjectorState;
                                break;
                            case RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE:
                                cellState = defaultState;
                                break;
                        }
                    }
                    else
                    {
                        cellState = closedState;
                    }

                    wellCellStates[wcIdx] = cellState;
                }
            }
        }

        // Find or create texture coords array for pipe surface

        if ( wellBranch.m_surfaceDrawable.notNull() )
        {
            cvf::ref<cvf::Vec2fArray> surfTexCoords = const_cast<cvf::Vec2fArray*>( wellBranch.m_surfaceDrawable->textureCoordArray() );
            if ( surfTexCoords.isNull() )
            {
                surfTexCoords = new cvf::Vec2fArray;
            }

            wellBranch.m_pipeGeomGenerator->pipeSurfaceTextureCoords( surfTexCoords.p(), wellCellStates, scalarMapper.p() );

            wellBranch.m_surfaceDrawable->setTextureCoordArray( surfTexCoords.p() );
            wellBranch.m_largeSurfaceDrawable->setTextureCoordArray( surfTexCoords.p() );

            if ( wResFrame->isOpen() )
            {
                // Use slightly larger geometry for open wells to avoid z-fighting when two wells are located at the
                // same position

                wellBranch.m_surfacePart->setDrawable( wellBranch.m_largeSurfaceDrawable.p() );
            }
            else
            {
                wellBranch.m_surfacePart->setDrawable( wellBranch.m_surfaceDrawable.p() );
            }

            wellBranch.m_surfacePart->setEffect( scalarMapperSurfaceEffect.p() );
        }

        // Find or create texture coords array for pipe center line
        if ( wellBranch.m_centerLineDrawable.notNull() )
        {
            cvf::ref<cvf::Vec2fArray> lineTexCoords = const_cast<cvf::Vec2fArray*>( wellBranch.m_centerLineDrawable->textureCoordArray() );

            if ( lineTexCoords.isNull() )
            {
                lineTexCoords = new cvf::Vec2fArray;
            }

            // Calculate new texture coordinates
            wellBranch.m_pipeGeomGenerator->centerlineTextureCoords( lineTexCoords.p(), wellCellStates, scalarMapper.p() );

            // Set the new texture coordinates

            wellBranch.m_centerLineDrawable->setTextureCoordArray( lineTexCoords.p() );

            // Set effects

            wellBranch.m_centerLinePart->setEffect( scalarMapperMeshEffect.p() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RivSimWellPipesPartMgr::flattenedBranchWellHeadOffsets()
{
    return m_flattenedBranchWellHeadOffsets;
}
