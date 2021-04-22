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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <list>
#include <string>

class RimEclipseInputCase;
class RimGeoMechCase;
class RimParameterGroup;
class RimFaultRAPreprocSettings;

class RimFaultRASettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultRASettings();
    ~RimFaultRASettings() override;

    void initFromSettings( RimFaultRAPreprocSettings* preprocsettings, RimEclipseInputCase* eclipseCase );

    void            setGeoMechCase( RimGeoMechCase* geomechCase );
    RimGeoMechCase* geomechCase() const;
    QString         geomechCaseFilename() const;

    RimEclipseInputCase* eclipseCase() const;
    QString              eclipseCaseFilename() const;

    void    setOutputBaseDirectory( QString baseDir );
    QString outputBaseDirectory() const;

    QString basicCalcParameterFilename() const;
    QString advancedCalcParameterFilename() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmPtrField<RimEclipseInputCase*> m_eclipseCase;
    caf::PdmPtrField<RimGeoMechCase*>      m_geomechCase;
    caf::PdmField<QString>                 m_baseDir;

    caf::PdmChildArrayField<RimParameterGroup*> m_basicparameters;
    caf::PdmChildArrayField<RimParameterGroup*> m_advancedparameters;
};
