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

#include "RivGeoMechPartMgr.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include <cstdlib>

#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgr::RivGeoMechPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RivGeoMechPartMgr::~RivGeoMechPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::clearAndSetReservoir( const RigGeoMechCaseData* geoMechCase )
{
    m_femPartPartMgrs.clear();

    if ( geoMechCase )
    {
        const RigFemPartCollection* femParts = geoMechCase->femParts();

        for ( int i = 0; i < femParts->partCount(); ++i )
        {
            m_femPartPartMgrs.push_back( new RivFemPartPartMgr( femParts->part( i ) ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::setTransform( cvf::Transform* scaleTransform )
{
    for ( size_t i = 0; i < m_femPartPartMgrs.size(); ++i )
    {
        m_femPartPartMgrs[i]->setTransform( scaleTransform );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::setCellVisibility( size_t partIndex, cvf::UByteArray* cellVisibilities )
{
    CVF_ASSERT( partIndex < m_femPartPartMgrs.size() );
    m_femPartPartMgrs[partIndex]->setCellVisibility( cellVisibilities );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray> RivGeoMechPartMgr::cellVisibility( size_t partIdx )
{
    CVF_ASSERT( partIdx < m_femPartPartMgrs.size() );
    return m_femPartPartMgrs[partIdx]->cellVisibility();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::updateCellColor( cvf::Color4f color )
{
    for ( size_t i = 0; i < m_femPartPartMgrs.size(); ++i )
    {
        m_femPartPartMgrs[i]->updateCellColor( color );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::updateCellResultColor( size_t timeStepIndex, RimGeoMechCellColors* cellResultColors )
{
    for ( size_t i = 0; i < m_femPartPartMgrs.size(); ++i )
    {
        m_femPartPartMgrs[i]->updateCellResultColor( timeStepIndex, cellResultColors );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::appendGridPartsToModel( cvf::ModelBasicList* model )
{
    for ( size_t i = 0; i < m_femPartPartMgrs.size(); ++i )
    {
        m_femPartPartMgrs[i]->appendPartsToModel( model );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RivGeoMechPartMgr::appendGridPartsToModel( cvf::ModelBasicList* model, const std::vector<size_t>& partIndices )
{
    for ( size_t i = 0; i < partIndices.size(); ++i )
    {
        if ( partIndices[i] < m_femPartPartMgrs.size() )
        {
            m_femPartPartMgrs[partIndices[i]]->appendPartsToModel( model );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::Collection<RivFemPartPartMgr> RivGeoMechPartMgr::femPartMgrs() const
{
    return m_femPartPartMgrs;
}
