/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Rim3dWellLogCurve.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimWellLogFile;
class RimWellLogFileCurveNameConfig;

//==================================================================================================
///
///
//==================================================================================================
class Rim3dWellLogFileCurve : public Rim3dWellLogCurve
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dWellLogFileCurve();
    ~Rim3dWellLogFileCurve() override;

    void    setDefaultFileCurveDataInfo();
    void    curveValuesAndMds( std::vector<double>* values, std::vector<double>* measuredDepthValues ) const override;
    QString resultPropertyString() const override;
    QString name() const override;
    QString createAutoName() const override;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmPtrField<RimWellLogFile*>                  m_wellLogFile;
    caf::PdmField<QString>                             m_wellLogChannelName;
    caf::PdmChildField<RimWellLogFileCurveNameConfig*> m_nameConfig;
};
