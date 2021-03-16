/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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
#include "cafPdmPtrField.h"

class RimEclipseResultCase;
class RimGeoMechCase;

class RimFaultRAPreprocSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultRAPreprocSettings();
    ~RimFaultRAPreprocSettings() override;

    void setGeoMechCase( RimGeoMechCase* geomechCase );
    void setOutputBaseDirectory( QString baseDir );

    int     startTimeStepIndex() const;
    QString startTimeStep() const;
    int     endTimeStepIndex() const;
    QString endTimeStep() const;

    QString eclipseCaseFilename() const;
    QString geomechCaseFilename() const;
    QString outputBaseDirectory() const;
    bool    cleanBaseDirectory() const;

    QString preprocParameterFilename() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    caf::PdmField<int>                      m_startTimestep;
    caf::PdmField<int>                      m_endTimestep;
    caf::PdmPtrField<RimEclipseResultCase*> m_eclipseCase;
    caf::PdmPtrField<RimGeoMechCase*>       m_geomechCase;
    caf::PdmField<QString>                  m_baseDir;
    caf::PdmField<bool>                     m_cleanBaseDir;
};
