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
#include "RimWellIAModelBox.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfVector3.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <list>
#include <string>
#include <vector>

class RimGeoMechCase;
class RimParameterGroup;
class RimGenericParameter;
class RimWellPath;
class RimWellIAModelData;

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

    QDateTime geostaticDate() const;

    void    setOutputBaseDirectory( QString baseDir );
    QString outputBaseDirectory() const;
    QString outputOdbFilename() const;

    bool showBox() const;
    void setShowBox( bool show );

    const std::list<RimParameterGroup*> inputParameterGroups() const;
    const std::list<RimParameterGroup*> resinsightParameterGroups() const;

    void   setDepthInterval( double startMD, double endMD );
    double startMD();
    double endMD();

    QString jsonInputFilename() const;
    QString csvInputFilename() const;

    QStringList commandParameters() const;

    RimWellPath* wellPath() const;

    bool                             modelBoxValid() const;
    std::vector<cvf::Vec3d>          modelBoxVertices() const;
    std::vector<RimWellIAModelData*> modelData() const;

    void extractModelData();
    void updateVisualization();

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
    std::vector<QDateTime> timeStepDates();

    void initCsvParameters();
    void updateResInsightParameters();
    void generateModelBox();
    void resetModelData();
    void resetResInsightParameters();

    void addCsvGroup( QString name, QStringList timeSteps, double defaultValue = 0.0 );

    std::vector<cvf::Vec3d> extractDisplacments( std::vector<cvf::Vec3d> corners, int timeStep );

private:
    caf::PdmProxyValueField<QString> m_nameProxy;

    caf::PdmPtrField<RimGeoMechCase*> m_geomechCase;
    caf::PdmField<QString>            m_baseDir;

    caf::PdmField<bool>      m_showBox;
    caf::PdmField<bool>      m_boxValid;
    caf::PdmField<double>    m_startMD;
    caf::PdmField<double>    m_endMD;
    caf::PdmField<double>    m_bufferXY;
    caf::PdmField<double>    m_bufferZ;
    caf::PdmField<QDateTime> m_geostaticDate;

    caf::PdmChildArrayField<RimParameterGroup*> m_parameters;
    std::vector<RimParameterGroup*>             m_parametersRI;
    caf::PdmChildArrayField<RimParameterGroup*> m_csvParameters;

    RimWellIAModelBox                m_modelbox;
    std::vector<RimWellIAModelData*> m_modelData;

    enum class CSV_GROUPNAME
    {
        FORMATION_PRESSURE = 0,
        CASING_PRESSURE    = 1,
        TEMPERATURE        = 2
    };
    const std::vector<QString> m_csvGroupNames{ "Formation Pressure", "Casing Pressure", "Temperature" };
};
