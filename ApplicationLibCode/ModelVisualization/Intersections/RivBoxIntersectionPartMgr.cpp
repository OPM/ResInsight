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

#include "RivBoxIntersectionPartMgr.h"

#include "RigCaseCellResultsData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimBoxIntersection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimIntersectionResultDefinition.h"

#include "RivBoxIntersectionSourceInfo.h"
#include "RivExtrudedCurveIntersectionPartMgr.h"
#include "RivIntersectionGeometryGeneratorInterface.h"
#include "RivIntersectionHexGridInterface.h"
#include "RivIntersectionResultsColoringTools.h"
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
RivBoxIntersectionPartMgr::RivBoxIntersectionPartMgr( RimBoxIntersection* intersectionBox )
    : m_rimIntersectionBox( intersectionBox )
    , m_defaultColor( cvf::Color3::WHITE )
{
    CVF_ASSERT( m_rimIntersectionBox );

    m_intersectionBoxFacesTextureCoords = new cvf::Vec2fArray;

    cvf::ref<RivIntersectionHexGridInterface> hexGrid = intersectionBox->createHexGridInterface();
    m_intersectionBoxGenerator = new RivBoxIntersectionGeometryGenerator( m_rimIntersectionBox, hexGrid.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivBoxIntersectionPartMgr::applySingleColorEffect()
{
    m_defaultColor = cvf::Color3f::OLIVE; // m_rimCrossSection->CrossSectionColor();
    this->updatePartEffect();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivBoxIntersectionPartMgr::updateCellResultColor( size_t timeStepIndex )
{
    RivIntersectionResultsColoringTools::calculateIntersectionResultColors( timeStepIndex,
                                                                            true,
                                                                            m_rimIntersectionBox,
                                                                            m_intersectionBoxGenerator.p(),
                                                                            nullptr,
                                                                            nullptr,
                                                                            m_intersectionBoxFaces.p(),
                                                                            m_intersectionBoxFacesTextureCoords.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivBoxIntersectionPartMgr::generatePartGeometry()
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
            cvf::ref<RivBoxIntersectionSourceInfo> si = new RivBoxIntersectionSourceInfo( m_intersectionBoxGenerator.p() );
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
void RivBoxIntersectionPartMgr::updatePartEffect()
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
void RivBoxIntersectionPartMgr::appendNativeIntersectionFacesToModel( cvf::ModelBasicList* model,
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
void RivBoxIntersectionPartMgr::appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RivIntersectionGeometryGeneratorInterface* RivBoxIntersectionPartMgr::intersectionGeometryGenerator() const
{
    if ( m_intersectionBoxGenerator.notNull() ) return m_intersectionBoxGenerator.p();

    return nullptr;
}
