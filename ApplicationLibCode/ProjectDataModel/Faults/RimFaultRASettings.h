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
class RimEclipseCase;
class RimGeoMechCase;
class RimParameterGroup;
class RimFaultRAPreprocSettings;
class RimGenericParameter;

class RimFaultRASettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultRASettings();
    ~RimFaultRASettings() override;

    void initFromPreprocSettings( RimFaultRAPreprocSettings* preprocsettings, RimEclipseInputCase* eclipseCase );

    void            setGeoMechCase( RimGeoMechCase* geomechCase );
    RimGeoMechCase* geomechCase() const;
    QString         geomechCaseFilename() const;

    RimEclipseInputCase* eclipseFRAGeneratedCase() const;
    RimEclipseCase*      eclipseCase() const;
    QString              eclipseCaseFilename() const;

    void    setOutputBaseDirectory( QString baseDir );
    QString outputBaseDirectory() const;

    int     startTimeStepGeoMechIndex() const;
    QString startTimeStepGeoMech() const;
    int     endTimeStepGeoMechIndex() const;
    QString endTimeStepGeoMech() const;

    void setEclipseTimeStepIndexes( int start, int stop );
    void setGeomechTimeStepIndexes( int start, int stop );

    int     startTimeStepEclipseIndex() const;
    QString startTimeStepEclipse() const;
    QString loadStepStart() const;
    int     endTimeStepEclipseIndex() const;
    QString endTimeStepEclipse() const;
    QString loadStepEnd() const;

    std::list<RimGenericParameter*> basicParameters( int faultID );
    std::list<RimGenericParameter*> advancedParameters( int faultID );

    QString elasticPropertiesFilename() const;
    QString stressStartFilename() const;
    QString stressEndFilename() const;
    QString basicMacrisDatabase() const;
    QString advancedMacrisDatabase() const;

    QString basicParameterXMLFilename( int faultID ) const;
    QString advancedParameterXMLFilename( int faultID ) const;
    QString postprocParameterFilename( int faultID ) const;

    QStringList basicMacrisParameters( int faultID ) const;
    QStringList advancedMacrisParameters( int faultID ) const;
    QStringList postprocParameters( int faultID ) const;

    QString tsurfOutputDirectory() const;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    void setupResInsightParameters();

private:
    caf::PdmPtrField<RimEclipseInputCase*> m_eclipseFRAGeneratedCase;
    caf::PdmPtrField<RimEclipseCase*>      m_eclipseCase;
    caf::PdmPtrField<RimGeoMechCase*>      m_geomechCase;
    caf::PdmField<QString>                 m_baseDir;

    caf::PdmField<int> m_startTimestepEclipse;
    caf::PdmField<int> m_endTimestepEclipse;
    caf::PdmField<int> m_startTimestepGeoMech;
    caf::PdmField<int> m_endTimestepGeoMech;

    caf::PdmChildArrayField<RimParameterGroup*> m_basicParameters;
    caf::PdmChildArrayField<RimParameterGroup*> m_advancedParameters;
    caf::PdmPtrField<RimParameterGroup*>        m_basicParametersRI;
    caf::PdmPtrField<RimParameterGroup*>        m_advancedParametersRI;
};
