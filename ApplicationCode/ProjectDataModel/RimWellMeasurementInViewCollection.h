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

#include <vector>

class RimWellMeasurementInView;
class RimWellMeasurement;

class RimWellMeasurementInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellMeasurementInViewCollection();
    ~RimWellMeasurementInViewCollection() override;

    std::vector<RimWellMeasurementInView*> measurements() const;

    void syncWithChangesInWellMeasurementCollection();

    RimWellMeasurementInView* getWellMeasurementInView( const RimWellMeasurement* measurement ) const;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmChildArrayField<RimWellMeasurementInView*> m_measurementsInView;
};
