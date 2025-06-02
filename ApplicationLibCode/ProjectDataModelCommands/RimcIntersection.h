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

#pragma once

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

#include "cvfVector3.h"

class RivIntersectionGeometryGeneratorInterface;
class RimExtrudedCurveIntersection;

//==================================================================================================
///
//==================================================================================================
class RimcTriangleGeometry : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class GeometryType
    {
        FULL_3D,
        PROJECTED_TO_PLANE,
    };

public:
    RimcTriangleGeometry();

    static RimcTriangleGeometry* createFromVertices( const std::vector<cvf::Vec3f>& vertices );
    static RimcTriangleGeometry* createFromVerticesAndConnections( const std::vector<cvf::Vec3f>& vertices,
                                                                   const std::vector<int>&        connections );

    void setMeshVertices( const std::vector<cvf::Vec3f>& meshVertices );
    void setFaultMeshVertices( const std::vector<cvf::Vec3f>& faultMeshVertices );
    void setDisplayModelOffset( const cvf::Vec3d& offset );

private:
    static std::tuple<std::vector<float>, std::vector<float>, std::vector<float>>
        assignCoordinatesToSeparateVectors( const std::vector<cvf::Vec3f>& vertices );

private:
    caf::PdmField<std::vector<float>> m_x;
    caf::PdmField<std::vector<float>> m_y;
    caf::PdmField<std::vector<float>> m_z;
    caf::PdmField<std::vector<int>>   m_connections;

    caf::PdmField<std::vector<float>> m_meshX;
    caf::PdmField<std::vector<float>> m_meshY;
    caf::PdmField<std::vector<float>> m_meshZ;

    caf::PdmField<std::vector<float>> m_faultMeshX;
    caf::PdmField<std::vector<float>> m_faultMeshY;
    caf::PdmField<std::vector<float>> m_faultMeshZ;

    caf::PdmField<cvf::Vec3d> m_displayModelOffset;
};

//==================================================================================================
///
//==================================================================================================
class RimcExtrudedCurveIntersection_geometry : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcExtrudedCurveIntersection_geometry( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

    static std::unique_ptr<RivIntersectionGeometryGeneratorInterface>
        createGeometryGenerator( RimExtrudedCurveIntersection* intersection, RimcTriangleGeometry::GeometryType geometryType );

private:
    caf::PdmField<caf::AppEnum<RimcTriangleGeometry::GeometryType>> m_geometryType;
};

//==================================================================================================
///
//==================================================================================================
class RimcExtrudedCurveIntersection_geometryResult : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcExtrudedCurveIntersection_geometryResult( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

private:
    caf::PdmField<caf::AppEnum<RimcTriangleGeometry::GeometryType>> m_geometryType;
};
