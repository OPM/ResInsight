////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicBasicPolygonFeature.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicBasicPolygonFeature::RicBasicPolygonFeature( bool multiSelectSupported )
    : m_multiSelectSupported( multiSelectSupported )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicBasicPolygonFeature::isCommandEnabled() const
{
    auto polygons = selectedPolygons();

    return m_multiSelectSupported ? !polygons.empty() : polygons.size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RicBasicPolygonFeature::selectedPolygons() const
{
    std::set<RimPolygon*> uniquePolygons;

    auto polygons   = caf::selectedObjectsByType<RimPolygon*>();
    auto polygonivs = caf::selectedObjectsByType<RimPolygonInView*>();
    for ( auto piv : polygonivs )
    {
        polygons.push_back( piv->polygon() );
    }

    // make sure we avoid duplicates
    for ( auto p : polygons )
    {
        uniquePolygons.insert( p );
    }

    std::vector<RimPolygon*> returnPolygons;
    for ( auto p : uniquePolygons )
    {
        returnPolygons.push_back( p );
    }

    return returnPolygons;
}
