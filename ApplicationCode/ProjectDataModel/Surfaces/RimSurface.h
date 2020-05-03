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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfObject.h"

#include "cafPdmFieldCvfColor.h"

class RigSurface;

class RimSurface : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurface();
    ~RimSurface() override;

    void         setColor( const cvf::Color3f& color );
    cvf::Color3f color() const;

    RigSurface* surfaceData();

    QString userDescription();

    virtual bool loadData();

protected:
    void setUserDescription( const QString& description );
    void setSurfaceData( RigSurface* surface );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmFieldHandle* userDescriptionField() override;

    caf::PdmField<QString>      m_userDescription;
    caf::PdmField<cvf::Color3f> m_color;

    cvf::ref<RigSurface> m_surfaceData;
};
