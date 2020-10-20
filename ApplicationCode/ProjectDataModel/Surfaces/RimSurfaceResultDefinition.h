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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RimRegularLegendConfig;
class RimSurfaceInView;
class RigSurface;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimSurfaceResultDefinition : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceResultDefinition();
    ~RimSurfaceResultDefinition() override;

    void    setSurfaceInView( RimSurfaceInView* surfaceInView );
    QString propertyName() const;

    RimRegularLegendConfig* legendConfig();

    void updateMinMaxValues();
    void assignDefaultProperty();

private:
    void         fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly ) override;

    RigSurface* surfaceData();

private:
    caf::PdmField<QString>                      m_propertyName;
    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    caf::PdmPtrField<RimSurfaceInView*> m_surfaceInView;
};
