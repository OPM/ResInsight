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
class RimCase;

class RimFaultRAPreprocSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultRAPreprocSettings();
    ~RimFaultRAPreprocSettings() override;

    void            setGeoMechCase( RimGeoMechCase* geomechCase );
    RimGeoMechCase* geoMechCase() const;

    void setEclipseCase( RimEclipseResultCase* eclipseCase );

    void    setOutputBaseDirectory( QString baseDir );
    QString outputBaseDirectory() const;

    int     startTimeStepGeoMechIndex() const;
    QString startTimeStepGeoMech() const;
    int     endTimeStepGeoMechIndex() const;
    QString endTimeStepGeoMech() const;

    int     startTimeStepEclipseIndex() const;
    QString startTimeStepEclipse() const;
    int     endTimeStepEclipseIndex() const;
    QString endTimeStepEclipse() const;

    QString eclipseCaseFilename() const;
    QString geomechCaseFilename() const;
    bool    cleanBaseDirectory() const;
    bool    smoothEclipseData() const;

    bool geoMechSelected() const;
    bool validatePreferences() const;

    QString preprocParameterFilename() const;
    QString outputEclipseFilename() const;
    QString outputEclipseDirectory() const;

    QStringList preprocParameterList() const;
    QStringList macrisPrepareParameterList() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    RimCase* startCase() const;

    caf::PdmField<int>                      m_startTimestepEclipse;
    caf::PdmField<int>                      m_endTimestepEclipse;
    caf::PdmField<int>                      m_startTimestepGeoMech;
    caf::PdmField<int>                      m_endTimestepGeoMech;
    caf::PdmPtrField<RimEclipseResultCase*> m_eclipseCase;
    caf::PdmField<bool>                     m_smoothEclipseData;
    caf::PdmPtrField<RimGeoMechCase*>       m_geomechCase;
    caf::PdmField<QString>                  m_baseDir;
    caf::PdmField<bool>                     m_cleanBaseDir;
};
