/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"
#include "RimPolylineTarget.h"

#include "cafPdmChildField.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObject.h"

namespace
{
class PolygonInViewTestRoot : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PolygonInViewTestRoot()
    {
        CAF_PDM_InitObject( "PolygonInViewTestRoot" );
        CAF_PDM_InitFieldNoDefault( &m_polygon, "Polygon", "Polygon" );
        CAF_PDM_InitFieldNoDefault( &m_polygonInView, "PolygonInView", "Polygon In View" );
    }

    caf::PdmChildField<RimPolygon*>       m_polygon;
    caf::PdmChildField<RimPolygonInView*> m_polygonInView;
};

CAF_PDM_SOURCE_INIT( PolygonInViewTestRoot, "PolygonInViewTestRoot" );

} // namespace

//--------------------------------------------------------------------------------------------------
/// The targets of a polygon in view are not stored in the project file, and must be recreated from
/// the polygon when the project is read
//--------------------------------------------------------------------------------------------------
TEST( RimPolygonInView, TargetsAreRestoredFromPolygonWhenRead )
{
    const std::vector<cvf::Vec3d> points = { { 0.0, 0.0, -1000.0 }, { 100.0, 0.0, -1000.0 }, { 100.0, 100.0, -1000.0 } };

    QString xml;
    {
        PolygonInViewTestRoot source;
        source.m_polygon = new RimPolygon();
        source.m_polygon->setPointsInDomainCoords( points );
        source.m_polygonInView = new RimPolygonInView();
        source.m_polygonInView->setPolygon( source.m_polygon() );
        ASSERT_EQ( points.size(), source.m_polygonInView->activeTargets().size() );

        xml = source.writeObjectToXmlString();
    }

    PolygonInViewTestRoot restored;
    restored.readObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance() );
    restored.resolveReferencesRecursively();
    restored.initAfterReadRecursively();

    ASSERT_TRUE( restored.m_polygonInView() != nullptr );
    ASSERT_EQ( restored.m_polygon(), restored.m_polygonInView->polygon() );

    auto targets = restored.m_polygonInView->activeTargets();
    ASSERT_EQ( points.size(), targets.size() );
    for ( size_t i = 0; i < points.size(); i++ )
    {
        EXPECT_EQ( points[i], targets[i]->targetPointXYZ() );
    }
}