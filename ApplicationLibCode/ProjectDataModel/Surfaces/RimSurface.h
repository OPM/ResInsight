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

#include "Rim3dPropertiesInterface.h"

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RimRegularLegendConfig;
class RigSurface;

class RimSurface : public caf::PdmObject, public Rim3dPropertiesInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SurfaceType
    {
        DEFAULT,
        ENSEMBLE_SOURCE,
        ENSEMBLE_STATISTICS,
        // TODO: Add fault reactivation surfaces

    };

public:
    RimSurface();
    ~RimSurface() override;

    void         setColor( const cvf::Color3f& color );
    cvf::Color3f color() const;

    std::pair<bool, float> opacity() const;
    void                   setOpacity( bool useOpacity, float opacity );

    virtual bool showIntersectionCellResults();

    RigSurface* surfaceData();
    QString     userDescription();
    void        setUserDescription( const QString& description );

    virtual void   loadSurfaceDataForTimeStep( int timeStep );
    virtual size_t timeStepCount() const;

    virtual QString     fullName() const;
    virtual bool        onLoadData() = 0;
    virtual RimSurface* createCopy() = 0;

    void loadDataIfRequired();
    void reloadData();

    virtual void updateMinMaxValues( RimRegularLegendConfig* legend, const QString& propertyName, int currentTimeStep ) const;

    virtual bool isMeshLinesEnabledDefault() const;

protected:
    void setSurfaceData( RigSurface* surface );

    void   applyDepthOffsetIfNeeded( std::vector<cvf::Vec3d>* vertices ) const;
    double depthOffset() const;
    void   setDepthOffset( double depthoffset );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    static void applyDepthOffset( const cvf::Vec3d& offset, std::vector<cvf::Vec3d>* vertices );

    caf::PdmFieldHandle* userDescriptionField() override;

    virtual bool updateSurfaceData()     = 0;
    virtual void clearCachedNativeData() = 0;

    cvf::BoundingBox boundingBoxInDomainCoords() const override;

protected:
    caf::PdmField<QString> m_userDescription;

    caf::PdmField<cvf::Color3f> m_color;
    caf::PdmField<bool>         m_enableOpacity;
    caf::PdmField<bool>         m_showMeshLines;
    caf::PdmField<double>       m_opacity;
    caf::PdmField<double>       m_depthOffset;

    cvf::ref<RigSurface> m_surfaceData;

private:
    caf::PdmProxyValueField<QString> m_nameProxy;
};
