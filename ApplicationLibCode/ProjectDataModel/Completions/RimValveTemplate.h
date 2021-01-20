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

#include "RiaDefines.h"

#include "RimCheckableNamedObject.h"

#include "cafPdmChildField.h"

class RimWellPathAicdParameters;

class RimValveTemplate : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimValveTemplate();
    ~RimValveTemplate() override;

    void loadDataAndUpdate();
    void setUnitSystem( caf::AppEnum<RiaDefines::EclipseUnitSystem> unitSystem );
    void setDefaultValuesFromUnits();

    RiaDefines::WellPathComponentType           type() const;
    void                                        setType( RiaDefines::WellPathComponentType type );
    caf::AppEnum<RiaDefines::EclipseUnitSystem> templateUnits() const;
    double                                      orificeDiameter() const;
    double                                      flowCoefficient() const;
    const RimWellPathAicdParameters*            aicdParameters() const;
    QString                                     typeLabel() const;
    QString                                     fullLabel() const;
    void                                        setUserLabel( const QString& userLabel );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    typedef caf::AppEnum<RiaDefines::WellPathComponentType> CompletionTypeEnum;

    caf::PdmField<caf::AppEnum<RiaDefines::EclipseUnitSystem>> m_valveTemplateUnit;

    caf::PdmField<CompletionTypeEnum> m_type;
    caf::PdmField<QString>            m_userLabel;

    // ICD and ICVs only
    caf::PdmField<double> m_orificeDiameter;
    caf::PdmField<double> m_flowCoefficient;
    // AICDs
    caf::PdmChildField<RimWellPathAicdParameters*> m_aicdParameters;
};
