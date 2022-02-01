/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RimcIntersection.h"

#include "RiaLogging.h"

#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "Rim2dIntersectionView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimIntersectionResultDefinition.h"

#include "RimcDataContainerDouble.h"

#include "RivExtrudedCurveIntersectionGeometryGenerator.h"
#include "RivExtrudedCurveIntersectionPartMgr.h"
#include "RivIntersectionGeometryGeneratorInterface.h"
#include "RivIntersectionHexGridInterface.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"

#include <memory>

namespace caf
{
template <>
void caf::AppEnum<RimcTriangleGeometry::GeometryType>::setUp()
{
    addItem( RimcTriangleGeometry::GeometryType::FULL_3D, "FULL_3D", "Full 3DDynamic" );
    addItem( RimcTriangleGeometry::GeometryType::PROJECTED_TO_PLANE, "PROJECTED_TO_PLANE", "Projected To Plane" );
    setDefault( RimcTriangleGeometry::GeometryType::FULL_3D );
}
}; // namespace caf

CAF_PDM_SOURCE_INIT( RimcTriangleGeometry, "TriangleGeometry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangleGeometry::RimcTriangleGeometry()
{
    CAF_PDM_InitScriptableObject( "Triangle Geometry" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_x, "xcoords", "X coords" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_y, "ycoords", "Y coords" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_z, "zcoords", "Z coords" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_connections, "connections", "Indices to triangle vertices" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_meshX, "meshxcoords", "Mesh X coords" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_meshY, "meshycoords", "Mesh Y coords" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_meshZ, "meshzcoords", "Mesh Z coords" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_displayModelOffset, "displaymodeloffset", "Display Model Offset" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangleGeometry* RimcTriangleGeometry::createFromVertices( const std::vector<cvf::Vec3f>& vertices )
{
    std::vector<int> conn;
    if ( !vertices.empty() )
    {
        conn.resize( vertices.size() );
        std::iota( conn.begin(), conn.end(), 0 );
    }

    return createFromVerticesAndConnections( vertices, conn );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangleGeometry* RimcTriangleGeometry::createFromVerticesAndConnections( const std::vector<cvf::Vec3f>& vertices,
                                                                              const std::vector<int>& connections )
{
    auto [xVals, yVals, zVals] = assignCoordinatesToSeparateVectors( vertices );

    auto obj = new RimcTriangleGeometry;
    obj->m_x = xVals;
    obj->m_y = yVals;
    obj->m_z = zVals;

    obj->m_connections = connections;

    return obj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcTriangleGeometry::setMeshVertices( const std::vector<cvf::Vec3f>& vertices )
{
    auto [xVals, yVals, zVals] = assignCoordinatesToSeparateVectors( vertices );

    m_meshX = xVals;
    m_meshY = yVals;
    m_meshZ = zVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcTriangleGeometry::setDisplayModelOffset( const cvf::Vec3d& offset )
{
    m_displayModelOffset = offset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<std::vector<float>, std::vector<float>, std::vector<float>>
    RimcTriangleGeometry::assignCoordinatesToSeparateVectors( const std::vector<cvf::Vec3f>& vertices )
{
    std::vector<float> xVals;
    std::vector<float> yVals;
    std::vector<float> zVals;

    if ( !vertices.empty() )
    {
        xVals.reserve( vertices.size() );
        yVals.reserve( vertices.size() );
        zVals.reserve( vertices.size() );

        for ( const auto& v : vertices )
        {
            xVals.push_back( v.x() );
            yVals.push_back( v.y() );
            zVals.push_back( v.z() );
        }
    }

    return { xVals, yVals, zVals };
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimExtrudedCurveIntersection, RimcExtrudedCurveIntersection_geometry, "geometry" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcExtrudedCurveIntersection_geometry::RimcExtrudedCurveIntersection_geometry( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Intersection Geometry" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_geometryType, "Geometrytype", "Geometry Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcExtrudedCurveIntersection_geometry::execute()
{
    auto intersection = self<RimExtrudedCurveIntersection>();

    auto geoGenerator = RimcExtrudedCurveIntersection_geometry::createGeometryGenerator( intersection, m_geometryType() );
    if ( geoGenerator && geoGenerator->isAnyGeometryPresent() )
    {
        std::vector<cvf::Vec3f> coords;
        geoGenerator->triangleVxes()->toStdVector( &coords );

        auto triangleGeometry = RimcTriangleGeometry::createFromVertices( coords );

        auto cellMeshVertices = geoGenerator->cellMeshVxes();
        if ( cellMeshVertices )
        {
            std::vector<cvf::Vec3f> meshCoords;
            cellMeshVertices->toStdVector( &meshCoords );
            triangleGeometry->setMeshVertices( meshCoords );
        }

        {
            RimEclipseView* eclView = nullptr;
            intersection->firstAncestorOfType( eclView );
            if ( eclView && eclView->eclipseCase() )
            {
                auto offset = eclView->eclipseCase()->displayModelOffset();
                triangleGeometry->setDisplayModelOffset( offset );
            }
        }

        return triangleGeometry;
    }

    return new RimcTriangleGeometry;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcExtrudedCurveIntersection_geometry::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcExtrudedCurveIntersection_geometry::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcTriangleGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RivIntersectionGeometryGeneratorInterface>
    RimcExtrudedCurveIntersection_geometry::createGeometryGenerator( RimExtrudedCurveIntersection*      intersection,
                                                                     RimcTriangleGeometry::GeometryType geometryType )
{
    bool isFlat = false;
    if ( geometryType == RimcTriangleGeometry::GeometryType::PROJECTED_TO_PLANE ) isFlat = true;

    cvf::Vec3d flattenedPolylineStartPoint;
    auto       polyLines = intersection->polyLines( &flattenedPolylineStartPoint );
    if ( !polyLines.empty() )
    {
        auto direction = intersection->extrusionDirection();
        auto hexGrid   = intersection->createHexGridInterface();
        auto intersectionGeoGenerator =
            std::make_unique<RivExtrudedCurveIntersectionGeometryGenerator>( intersection,
                                                                             polyLines,
                                                                             direction,
                                                                             hexGrid.p(),
                                                                             isFlat,
                                                                             flattenedPolylineStartPoint );

        intersectionGeoGenerator->ensureGeometryIsCalculated();

        return intersectionGeoGenerator;
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimExtrudedCurveIntersection,
                                   RimcExtrudedCurveIntersection_geometryResult,
                                   "geometryResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcExtrudedCurveIntersection_geometryResult::RimcExtrudedCurveIntersection_geometryResult( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Geometry Result" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_geometryType, "Geometrytype", "Geometry Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcExtrudedCurveIntersection_geometryResult::execute()
{
    auto intersection = self<RimExtrudedCurveIntersection>();

    auto geoGenerator = RimcExtrudedCurveIntersection_geometry::createGeometryGenerator( intersection, m_geometryType() );
    if ( geoGenerator && geoGenerator->isAnyGeometryPresent() )
    {
        RimEclipseView* eclView = nullptr;
        intersection->firstAncestorOfType( eclView );
        if ( !eclView )
        {
            RiaLogging::error( "No Eclipse view found. Extraction of intersection result is only supported for "
                               "Eclipse view." );
            return nullptr;
        }

        RimEclipseResultDefinition* eclResultDef = nullptr;

        auto intersectionResultDef = intersection->activeSeparateResultDefinition();
        if ( intersectionResultDef ) eclResultDef = intersectionResultDef->eclipseResultDefinition();

        if ( !eclResultDef ) eclResultDef = eclView->cellResult();

        RigEclipseCaseData* eclipseCase = eclView->eclipseCase()->eclipseCaseData();

        size_t                      gridIndex = 0;
        cvf::ref<RigResultAccessor> resultAccessor =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase,
                                                                  gridIndex,
                                                                  eclView->currentTimeStep(),
                                                                  eclResultDef );

        auto                triToCellIndex = geoGenerator->triangleToCellIndex();
        std::vector<double> values;
        values.reserve( triToCellIndex.size() );
        for ( const auto& i : triToCellIndex )
        {
            auto value = resultAccessor->cellScalar( i );
            values.push_back( value );
        }

        return RimcDataContainerDouble::create( values );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcExtrudedCurveIntersection_geometryResult::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcExtrudedCurveIntersection_geometryResult::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimcDataContainerDouble );
}
