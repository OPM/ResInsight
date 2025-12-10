/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include <memory>

class RigTriangleMeshData;

class RimFileSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimFileSurface();
    ~RimFileSurface() override;

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    // File-based surface interface
    bool    isFileBased() const override { return true; }
    QString filePath() const override;
    void    setFilePath( const QString& path ) override;

protected:
    bool updateSurfaceData() override;
    void clearCachedNativeData() override;

private:
    bool loadDataFromFile();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmField<caf::FilePath> m_surfaceDefinitionFilePath;

    std::unique_ptr<RigTriangleMeshData> m_triangleMeshData;
};
