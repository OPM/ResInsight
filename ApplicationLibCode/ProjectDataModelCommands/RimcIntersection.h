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

private:
    caf::PdmField<std::vector<float>> m_x;
    caf::PdmField<std::vector<float>> m_y;
    caf::PdmField<std::vector<float>> m_z;

    caf::PdmField<std::vector<int>> m_connections;
};

//==================================================================================================
///
//==================================================================================================
class RimcExtrudedCurveIntersection_geometry : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcExtrudedCurveIntersection_geometry( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

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

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmField<caf::AppEnum<RimcTriangleGeometry::GeometryType>> m_geometryType;
};
