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

class RigGocadData;

class RimFileSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimFileSurface();
    ~RimFileSurface() override;

    void    setSurfaceFilePath( const QString& filePath );
    QString surfaceFilePath();

    bool onLoadData() override;

protected:
    bool updateSurfaceData() override;
    void clearCachedNativeData() override;

private:
    bool loadDataFromFile();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmField<caf::FilePath> m_surfaceDefinitionFilePath;

    std::vector<unsigned>   m_tringleIndices;
    std::vector<cvf::Vec3d> m_vertices;

    std::unique_ptr<RigGocadData> m_gocadData;
};
