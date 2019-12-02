/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include <vector>

class RimWellMeasurement;
class RimRegularLegendConfig;
class RimWellPath;
class RimWellPathCollection;
class RiuViewer;

class RimWellMeasurementInView : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementInView();
    ~RimWellMeasurementInView() override;

    RimRegularLegendConfig* legendConfig();
    QString                 measurementKind() const;
    void                    setMeasurementKind( const QString& measurementKind );

    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    bool hasCategoryResult() const;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly );

    bool        updateLegendData();
    static bool hasMeasurementKindForWell( const RimWellPath*                      wellPath,
                                           const RimWellPathCollection*            wellPathCollection,
                                           const std::vector<RimWellMeasurement*>& measurements,
                                           const QString&                          measurementKind );

private:
    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
    caf::PdmField<QString>                      m_measurementKind;
    caf::PdmField<std::vector<QString>>         m_wells;
};
