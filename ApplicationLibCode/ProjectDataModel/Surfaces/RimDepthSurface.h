/////////////////////////////////////////////////////////////////////////////////
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

#pragma once

#include "RimSurface.h"

#include "cafPdmCoreVec3d.h"

class RimDepthSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimDepthSurface();
    ~RimDepthSurface() override;

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    bool showIntersectionCellResults() override;

    void setPlaneExtent( double minX, double minY, double maxX, double maxY );
    void setDepth( double depth );
    void setDepthSliderLimits( double lower, double upper );
    void setAreaOfInterest( cvf::Vec3d min, cvf::Vec3d max );

private:
    bool    updateSurfaceData() override;
    void    clearCachedNativeData() override;
    QString fullName() const override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<double> m_minX;
    caf::PdmField<double> m_maxX;
    caf::PdmField<double> m_minY;
    caf::PdmField<double> m_maxY;

    caf::PdmField<double> m_depth;
    caf::PdmField<double> m_depthLowerLimit;
    caf::PdmField<double> m_depthUpperLimit;

    caf::PdmField<cvf::Vec3d> m_areaOfInterestMin;
    caf::PdmField<cvf::Vec3d> m_areaOfInterestMax;

    std::vector<unsigned>   m_triangleIndices;
    std::vector<cvf::Vec3d> m_vertices;
};
