/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimSurface.h"

#include "cafFilePath.h"

#include <memory>

class RigTriangleMeshData;

//==================================================================================================
/// Surface with geometry and properties set from scripting, saved as a GOCAD TSurf file next to the project file
//==================================================================================================
class RimTriangulatedSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimTriangulatedSurface();
    ~RimTriangulatedSurface() override;

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    void setGeometry( const std::vector<cvf::Vec3d>& vertices, const std::vector<unsigned>& triangleIndices );
    void setProperty( const QString& key, const std::vector<float>& values );

    size_t               vertexCount() const;
    std::vector<float>   getProperty( const QString& key ) const;
    std::vector<QString> propertyNames() const;

    void onProjectBeingSaved();

private:
    bool updateSurfaceData() override;
    void clearCachedNativeData() override;

    bool    loadDataFromFile();
    QString fileNameInProjectFolder() const;

private:
    caf::PdmField<caf::FilePath> m_surfaceFilePath;

    std::unique_ptr<RigTriangleMeshData> m_triangleMeshData;
};
