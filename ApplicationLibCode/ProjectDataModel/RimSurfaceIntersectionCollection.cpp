////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimSurfaceIntersectionCollection.h"

#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCurve.h"

CAF_PDM_SOURCE_INIT( RimSurfaceIntersectionCollection, "RimSurfaceIntersectionCollection_msj" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCollection::RimSurfaceIntersectionCollection()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "SurfaceIntersectionCollection_msj", ":/do_not_exist.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_intersectionBands, "IntersectionBands", "Intersection Bands", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_intersectionCurves, "IntersectionCurves", "Intersection Curves", "", "", "" );

    addIntersectionCurve();
    addIntersectionCurve();

    addIntersectionBand();
    addIntersectionBand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCollection::addIntersectionCurve()
{
    auto curve = new RimSurfaceIntersectionCurve;
    curve->objectChanged.connect( this, &RimSurfaceIntersectionCollection::onObjectChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCollection::addIntersectionBand()
{
    auto band = new RimSurfaceIntersectionBand;
    band->objectChanged.connect( this, &RimSurfaceIntersectionCollection::onObjectChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionCurve*> RimSurfaceIntersectionCollection::surfaceIntersectionCurves() const
{
    return m_intersectionCurves.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionBand*> RimSurfaceIntersectionCollection::surfaceIntersectionBands() const
{
    return m_intersectionBands.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCollection::onObjectChanged( const caf::SignalEmitter* emitter )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceIntersectionCollection::initAfterRead()
{
    for ( const auto& band : m_intersectionBands )
    {
        band->objectChanged.connect( this, &RimSurfaceIntersectionCollection::onObjectChanged );
    }

    for ( const auto& curve : m_intersectionCurves )
    {
        curve->objectChanged.connect( this, &RimSurfaceIntersectionCollection::onObjectChanged );
    }
}
