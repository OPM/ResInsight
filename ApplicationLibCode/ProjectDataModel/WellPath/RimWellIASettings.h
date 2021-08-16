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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <list>
#include <string>

class RimGeoMechCase;
class RimParameterGroup;
class RimGenericParameter;

class RimWellIASettings : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellIASettings();
    ~RimWellIASettings() override;

    bool initSettings( QString& outErrmsg );

    void            setGeoMechCase( RimGeoMechCase* geomechCase );
    RimGeoMechCase* geomechCase() const;
    QString         geomechCaseFilename() const;
    QString         geomechCaseName() const;

    void    setOutputBaseDirectory( QString baseDir );
    QString outputBaseDirectory() const;

    std::list<RimGenericParameter*> basicParameters();

    RimGenericParameter* getInputParameter( QString name ) const;

    void   setDepthInterval( double startMD, double endMD );
    double startMD();
    double endMD();

    QString modelName() const;
    void    setModelName( const QString name );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    QString fullName() const;

private:
    void setupResInsightParameters();

private:
    caf::PdmProxyValueField<QString> m_nameProxy;

    caf::PdmPtrField<RimGeoMechCase*> m_geomechCase;
    caf::PdmField<QString>            m_baseDir;
    caf::PdmField<double>             m_startMD;
    caf::PdmField<double>             m_endMD;

    caf::PdmChildArrayField<RimParameterGroup*> m_basicParameters;
    caf::PdmPtrField<RimParameterGroup*>        m_basicParametersRI;
};
