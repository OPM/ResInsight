/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

class RimRegularSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimRegularSurface();

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    void setOriginX( double originX );
    void setOriginY( double originY );
    void setNx( int nx );
    void setNy( int ny );
    void setDepth( double depth );
    void setIncrementX( double incrementX );
    void setIncrementY( double incrementY );
    void setRotation( double rotation );

    void setProperty( const QString& key, const std::vector<float>& values );
    bool setPropertyAsDepth( const QString& key );

    int nx() const;
    int ny() const;

private:
    bool updateSurfaceData() override;
    void clearCachedNativeData() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<int>    m_nx;
    caf::PdmField<int>    m_ny;
    caf::PdmField<double> m_originX;
    caf::PdmField<double> m_originY;
    caf::PdmField<double> m_depth;
    caf::PdmField<double> m_incrementX;
    caf::PdmField<double> m_incrementY;
    caf::PdmField<double> m_rotation;

    caf::PdmField<QString> m_depthProperty;

    std::vector<unsigned>   m_triangleIndices;
    std::vector<cvf::Vec3d> m_vertices;

    std::map<QString, std::vector<float>> m_properties;
};
