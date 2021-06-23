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

#include "RiaDefines.h"

#include "RimStatisticsPlot.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimCase;
class RimPlot;
class RimGridView;
class RimEclipseResultDefinition;
class RimEclipseView;

//==================================================================================================
///
///
//==================================================================================================
class RimGridStatisticsPlot : public RimStatisticsPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridStatisticsPlot();
    ~RimGridStatisticsPlot() override;

    void cellFilterViewUpdated();

    void setPropertiesFromView( RimEclipseView* view );

protected:
    // Overridden PDM methods
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void initAfterRead() override;

    QString createAutoName() const override;
    QString timeStepString() const;

    void             setDefaults();
    bool             hasStatisticsData() const override;
    RigHistogramData createStatisticsData() const override;

protected:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimGridView*>                  m_cellFilterView;
    caf::PdmChildField<RimEclipseResultDefinition*> m_property;
};
