/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

class RigTriangleMeshData;

class RimFractureSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureSurface();
    ~RimFractureSurface() override;

    void    setSurfaceFilePath( const QString& filePath );
    QString surfaceFilePath();

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    bool   loadSurfaceDataForTimeStep( int timeStep ) override;
    size_t timeStepCount() const override;

    std::vector<std::vector<double>> valuesForProperty( const QString& propertyName ) const;

    void updateMinMaxValues( RimRegularLegendConfig* legend, const QString& propertyName, int currentTimeStep ) const override;
    bool isMeshLinesEnabledDefault() const override;

protected:
    bool updateSurfaceData() override;
    void clearCachedNativeData() override;

private:
    bool loadDataFromFile();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmField<caf::FilePath> m_surfaceDefinitionFilePath;

    std::vector<size_t>                               m_secondsSinceSimulationStart;
    std::vector<std::unique_ptr<RigTriangleMeshData>> m_surfacePerTimeStep;
};
