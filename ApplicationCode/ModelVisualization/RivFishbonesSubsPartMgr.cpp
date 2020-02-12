/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RivFishbonesSubsPartMgr.h"

#include "RigWellPath.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"

#include "RivObjectSourceInfo.h"
#include "RivPipeGeometryGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::RivFishbonesSubsPartMgr( RimFishbonesMultipleSubs* subs )
    : m_rimFishbonesSubs( subs )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::~RivFishbonesSubsPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::appendGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                          const caf::DisplayCoordTransform* displayCoordTransform,
                                                          double                            characteristicCellSize )
{
    clearGeometryCache();

    if ( !m_rimFishbonesSubs->isActive() ) return;

    if ( m_parts.size() == 0 )
    {
        buildParts( displayCoordTransform, characteristicCellSize );
    }

    for ( auto part : m_parts )
    {
        model->addPart( part.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::clearGeometryCache()
{
    m_parts.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::buildParts( const caf::DisplayCoordTransform* displayCoordTransform,
                                          double                            characteristicCellSize )
{
    RimWellPath* wellPath = nullptr;
    m_rimFishbonesSubs->firstAncestorOrThisOfTypeAsserted( wellPath );

    RivPipeGeometryGenerator geoGenerator;

    for ( auto& sub : m_rimFishbonesSubs->installedLateralIndices() )
    {
        for ( size_t lateralIndex : sub.lateralIndices )
        {
            std::vector<cvf::Vec3d> lateralDomainCoords = m_rimFishbonesSubs->coordsForLateral( sub.subIndex,
                                                                                                lateralIndex );

            std::vector<cvf::Vec3d> displayCoords = displayCoordTransform->transformToDisplayCoords( lateralDomainCoords );

            geoGenerator.cylinderWithCenterLineParts( &m_parts,
                                                      displayCoords,
                                                      m_rimFishbonesSubs->fishbonesColor(),
                                                      wellPath->combinedScaleFactor() * characteristicCellSize * 0.5 );
        }
    }

    cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo( m_rimFishbonesSubs );
    for ( auto part : m_parts )
    {
        part->setSourceInfo( objectSourceInfo.p() );
    }
}
