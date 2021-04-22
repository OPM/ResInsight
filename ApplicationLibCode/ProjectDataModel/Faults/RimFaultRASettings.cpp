/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimFaultRASettings.h"
#include "RimFaultRAParameterItem.h"
#include "RimFaultRAPreprocSettings.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimEclipseInputCase.h"
#include "RimGenericParameter.h"
#include "RimGeoMechCase.h"
#include "RimParameterGroup.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RifParameterXmlReader.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTableViewEditor.h"

CAF_PDM_SOURCE_INIT( RimFaultRASettings, "RimFaultRASettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRASettings::RimFaultRASettings()
{
    CAF_PDM_InitObject( "Reactivation Assessment Settings", ":/fault_react_24x24.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case", "", "", "" );
    m_eclipseCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_geomechCase, "GeomechCase", "GeoMech Case", "", "", "" );
    m_geomechCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDir", "Output Directory", "", "", "" );
    m_baseDir.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_basicparameters, "BasicParameters", "Basic Processing Parameters", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_advancedparameters, "AdvancedParameters", "Advanced Processing Parameters", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRASettings::~RimFaultRASettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::eclipseCaseFilename() const
{
    if ( m_eclipseCase ) return m_eclipseCase->gridFileName();
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase* RimFaultRASettings::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::geomechCaseFilename() const
{
    if ( m_geomechCase ) return m_geomechCase->gridFileName();
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimFaultRASettings::geomechCase() const
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::basicCalcParameterFilename() const
{
    return m_baseDir + "/calc_parameters.xml";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::advancedCalcParameterFilename() const
{
    return m_baseDir + "/calib_parameters.xml";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::outputBaseDirectory() const
{
    return m_baseDir();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::setGeoMechCase( RimGeoMechCase* geomechCase )
{
    m_geomechCase = geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::setOutputBaseDirectory( QString baseDir )
{
    m_baseDir = baseDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::initFromSettings( RimFaultRAPreprocSettings* preprocSettings, RimEclipseInputCase* eclipseCase )
{
    m_geomechCase = preprocSettings->geoMechCase();
    m_eclipseCase = eclipseCase;
    m_baseDir     = preprocSettings->outputBaseDirectory();

    QString errorText;

    RifParameterXmlReader basicreader( RiaPreferences::current()->geomechFRADefaultBasicXML() );
    if ( !basicreader.parseFile( errorText ) ) return;

    RifParameterXmlReader advreader( RiaPreferences::current()->geomechFRADefaultAdvXML() );
    if ( !advreader.parseFile( errorText ) ) return;

    m_basicparameters.clear();
    for ( auto group : basicreader.parameterGroups() )
    {
        m_basicparameters.push_back( group );
    }

    m_advancedparameters.clear();
    for ( auto group : advreader.parameterGroups() )
    {
        m_advancedparameters.push_back( group );
    }
}
