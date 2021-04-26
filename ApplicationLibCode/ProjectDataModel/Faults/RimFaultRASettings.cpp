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
#include "RimDoubleParameter.h"
#include "RimEclipseCase.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseResultCase.h"
#include "RimGenericParameter.h"
#include "RimGeoMechCase.h"
#include "RimIntegerParameter.h"
#include "RimParameterGroup.h"
#include "RimProject.h"
#include "RimStringParameter.h"
#include "RimTools.h"

#include "RifParameterXmlReader.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
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

    CAF_PDM_InitFieldNoDefault( &m_eclipseFRAGeneratedCase, "EclipseFRACase", "Eclipse FRA Case", "", "", "" );
    m_eclipseFRAGeneratedCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDir", "Working Directory", "", "", "" );
    m_baseDir.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_startTimestepEclipse, "StartTimeStepEclipse", 0, "Start Time Step", "", "", "" );
    m_startTimestepEclipse.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_endTimestepEclipse, "EndTimeStepEclipse", 0, "End Time Step", "", "", "" );
    m_endTimestepEclipse.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_startTimestepGeoMech, "StartTimeStepGeoMech", 0, "Start Time Step", "", "", "" );
    m_startTimestepGeoMech.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_startTimestepGeoMech.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_endTimestepGeoMech, "EndTimeStepGeoMech", 0, "End Time Step", "", "", "" );
    m_endTimestepGeoMech.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_endTimestepGeoMech.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_basicParameters, "BasicParameters", "Basic Processing Parameters", ":/Bullet.png", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_advancedParameters,
                                "AdvancedParameters",
                                "Advanced Processing Parameters",
                                ":/Bullet.png",
                                "",
                                "" );

    CAF_PDM_InitFieldNoDefault( &m_basicParametersRI, "BasicParametersRI", "Basic ResInsight Parameters", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_advancedParametersRI, "AdvancedParametersRI", "Advanced ResInsight Parameters", "", "", "" );
    setupResInsightParameters();
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
QList<caf::PdmOptionItemInfo> RimFaultRASettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_geomechCase )
    {
        RimTools::geoMechCaseOptionItems( &options );
    }

    if ( m_geomechCase() )
    {
        if ( fieldNeedingOptions == &m_startTimestepGeoMech )
        {
            RimTools::timeStepsForCase( m_geomechCase, &options );
        }
        else if ( fieldNeedingOptions == &m_endTimestepGeoMech )
        {
            RimTools::timeStepsForCase( m_geomechCase, &options );
        }
    }

    if ( m_eclipseCase() )
    {
        if ( fieldNeedingOptions == &m_startTimestepEclipse )
        {
            RimTools::timeStepsForCase( m_eclipseCase(), &options );
        }
        else if ( fieldNeedingOptions == &m_endTimestepEclipse )
        {
            RimTools::timeStepsForCase( m_eclipseCase(), &options );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_baseDir );
    auto eclipseGroup = uiOrdering.addNewGroup( "Eclipse Time Steps" );
    eclipseGroup->add( &m_startTimestepEclipse );
    eclipseGroup->add( &m_endTimestepEclipse );

    if ( m_geomechCase() != nullptr )
    {
        auto geomechGroup = uiOrdering.addNewGroup( "GeoMech Time Steps" );
        geomechGroup->add( &m_startTimestepGeoMech );
        geomechGroup->add( &m_endTimestepGeoMech );
    }
    uiOrdering.skipRemainingFields( true );
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
RimEclipseCase* RimFaultRASettings::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase* RimFaultRASettings::eclipseFRAGeneratedCase() const
{
    return m_eclipseFRAGeneratedCase;
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
void RimFaultRASettings::initFromPreprocSettings( RimFaultRAPreprocSettings* preprocSettings,
                                                  RimEclipseInputCase*       eclipseCase )
{
    m_geomechCase             = preprocSettings->geoMechCase();
    m_eclipseCase             = preprocSettings->eclipseCase();
    m_eclipseFRAGeneratedCase = eclipseCase;
    m_baseDir                 = preprocSettings->outputBaseDirectory();
    m_startTimestepEclipse    = preprocSettings->startTimeStepEclipseIndex();
    m_endTimestepEclipse      = preprocSettings->endTimeStepEclipseIndex();
    m_startTimestepGeoMech    = preprocSettings->startTimeStepGeoMechIndex();
    m_endTimestepGeoMech      = preprocSettings->endTimeStepGeoMechIndex();

    QString errorText;

    RifParameterXmlReader basicreader( RiaPreferences::current()->geomechFRADefaultBasicXML() );
    if ( !basicreader.parseFile( errorText ) ) return;

    m_basicParameters.clear();
    for ( auto group : basicreader.parameterGroups() )
    {
        m_basicParameters.push_back( group );
    }

    if ( geomechCase() != nullptr )
    {
        RifParameterXmlReader advreader( RiaPreferences::current()->geomechFRADefaultAdvXML() );
        if ( !advreader.parseFile( errorText ) ) return;

        m_advancedParameters.clear();
        for ( auto group : advreader.parameterGroups() )
        {
            m_advancedParameters.push_back( group );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRASettings::startTimeStepEclipseIndex() const
{
    return m_startTimestepEclipse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::startTimeStepEclipse() const
{
    if ( m_eclipseCase() )
    {
        if ( ( m_startTimestepEclipse >= 0 ) && ( m_startTimestepEclipse <= m_eclipseCase->timeStepStrings().size() ) )
            return m_eclipseCase->timeStepStrings()[m_startTimestepEclipse];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRASettings::endTimeStepEclipseIndex() const
{
    return m_endTimestepEclipse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::endTimeStepEclipse() const
{
    if ( m_eclipseCase() )
    {
        if ( ( m_endTimestepEclipse >= 0 ) && ( m_endTimestepEclipse <= m_eclipseCase->timeStepStrings().size() ) )
            return m_eclipseCase->timeStepStrings()[m_endTimestepEclipse];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRASettings::startTimeStepGeoMechIndex() const
{
    return m_startTimestepGeoMech();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::startTimeStepGeoMech() const
{
    if ( m_geomechCase() )
    {
        if ( ( m_startTimestepGeoMech >= 0 ) && ( m_startTimestepGeoMech <= m_geomechCase->timeStepStrings().size() ) )
            return m_geomechCase->timeStepStrings()[m_startTimestepEclipse];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRASettings::endTimeStepGeoMechIndex() const
{
    return m_endTimestepEclipse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::endTimeStepGeoMech() const
{
    if ( m_geomechCase() )
    {
        if ( ( m_endTimestepGeoMech >= 0 ) && ( m_endTimestepGeoMech <= m_geomechCase->timeStepStrings().size() ) )
            return m_geomechCase->timeStepStrings()[m_endTimestepGeoMech];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::setEclipseTimeStepIndexes( int start, int stop )
{
    m_startTimestepEclipse = start;
    m_endTimestepEclipse   = stop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::setGeomechTimeStepIndexes( int start, int stop )
{
    m_startTimestepGeoMech = start;
    m_endTimestepGeoMech   = stop;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<RimGenericParameter*> RimFaultRASettings::basicParameters( int faultID ) const
{
    m_basicParametersRI->setParameterValue( "eclipse_input_grid", eclipseCaseFilename() );
    m_basicParametersRI->setParameterValue( "faultid", faultID );

    std::list<RimGenericParameter*> retlist;
    for ( auto& group : m_basicParameters.childObjects() )
    {
        for ( auto& p : m_basicParametersRI->parameters() )
        {
            retlist.push_back( p );
        }
        for ( auto& p : group->parameters() )
        {
            retlist.push_back( p );
        }
    }

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<RimGenericParameter*> RimFaultRASettings::advancedParameters( int faultID ) const
{
    m_advancedParametersRI->setParameterValue( "eclipse_loadstep_start", startTimeStepEclipse() );
    m_advancedParametersRI->setParameterValue( "eclipse_loadstep_end", m_endTimestepEclipse() );
    m_advancedParametersRI->setParameterValue( "faultid_calibration", faultID );
    m_advancedParametersRI->setParameterValue( "abaqus_elastic_properties", elasticPropertiesFilename() );
    m_advancedParametersRI->setParameterValue( "abaqus_stress_start", stressStartFilename() );
    m_advancedParametersRI->setParameterValue( "abaqus_stress_end", stressEndFilename() );

    std::list<RimGenericParameter*> retlist;
    for ( auto& group : m_advancedParameters.childObjects() )
    {
        for ( auto& p : m_advancedParametersRI->parameters() )
        {
            retlist.push_back( p );
        }
        for ( auto& p : group->parameters() )
        {
            retlist.push_back( p );
        }
    }

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::setupResInsightParameters()
{
    m_basicParametersRI = new RimParameterGroup();
    m_basicParametersRI->addParameter( "eclipse_input_grid", "" );
    m_basicParametersRI->addParameter( "faultid", -1 );

    m_advancedParametersRI = new RimParameterGroup();
    m_advancedParametersRI->addParameter( "abaqus_elastic_properties", "" );
    m_advancedParametersRI->addParameter( "abaqus_stress_start", "" );
    m_advancedParametersRI->addParameter( "abaqus_stress_end", "" );
    m_advancedParametersRI->addParameter( "faultid_calibration", -1 );
    m_advancedParametersRI->addParameter( "eclipse_loadstep_start", "" );
    m_advancedParametersRI->addParameter( "eclipse_loadstep_end", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::elasticPropertiesFilename() const
{
    return m_baseDir + "/Abaqus/ELASTIC_TABLE_res.inp";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::stressStartFilename() const
{
    QString filename = QString( "/%1/%2_%3_stress.rpt" ).arg( "Abaqus", geomechCase()->uiName(), startTimeStepGeoMech() );
    return m_baseDir + filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::stressEndFilename() const
{
    QString filename = QString( "/%1/%2_%3_stress.rpt" ).arg( "Abaqus", geomechCase()->uiName(), endTimeStepGeoMech() );
    return m_baseDir + filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::loadStepStart() const
{
    QString retval = QString( "PRESSURE_%1" ).arg( startTimeStepEclipseIndex(), 2, 10, QChar( '0' ) );
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRASettings::loadStepEnd() const
{
    QString retval = QString( "PRESSURE_%1" ).arg( endTimeStepEclipseIndex(), 2, 10, QChar( '0' ) );
    return retval;
}
