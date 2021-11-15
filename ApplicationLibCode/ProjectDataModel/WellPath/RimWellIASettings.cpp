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

#include "RimWellIASettings.h"

#include "RiaApplication.h"
#include "RiaPreferencesGeoMech.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "RimDoubleParameter.h"
#include "RimGenericParameter.h"
#include "RimGeoMechCase.h"
#include "RimIntegerParameter.h"
#include "RimParameterGroup.h"
#include "RimProject.h"
#include "RimStringParameter.h"
#include "RimTools.h"
#include "RimWellIADataAccess.h"
#include "RimWellIAModelData.h"
#include "RimWellPath.h"

#include "RifParameterXmlReader.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDateEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTableViewEditor.h"

#include <QDebug>
#include <QFileInfo>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimWellIASettings, "RimWellIASettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettings::RimWellIASettings()
{
    CAF_PDM_InitObject( "Integrity Analysis Model Settings", ":/WellIntAnalysis.png", "", "" );

    setName( "Model" );

    CAF_PDM_InitFieldNoDefault( &m_geomechCase, "GeomechCase", "GeoMech Case" );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDir", "Working Directory" );
    m_baseDir.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_startMD, "StartMeasuredDepth", 0.0, "Start MD" );
    CAF_PDM_InitField( &m_endMD, "EndMeasuredDepth", 0.0, "End MD" );
    m_startMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_endMD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_bufferXY, "BufferXY", 5.0, "Model Size (XY)" );

    CAF_PDM_InitFieldNoDefault( &m_parameters, "ModelingParameters", "Modeling Parameters", ":/Bullet.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_csvParameters, "TimeStepParameters", "Time Step Parameters", ":/Bullet.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_nameProxy, "NameProxy", "Name Proxy" );
    m_nameProxy.registerGetMethod( this, &RimWellIASettings::fullName );
    m_nameProxy.uiCapability()->setUiReadOnly( true );
    m_nameProxy.uiCapability()->setUiHidden( true );
    m_nameProxy.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_showBox, "showBox", false, "Show model box" );

    CAF_PDM_InitFieldNoDefault( &m_geostaticDate, "startDate", "Start Date (geostatic):" );

    CAF_PDM_InitField( &m_boxValid, "boxValid", false, "Model box is valid" );
    m_boxValid.uiCapability()->setUiHidden( true );

    this->setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellIASettings::~RimWellIASettings()
{
    resetResInsightParameters();
    resetModelData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettings::initSettings( QString& outErrmsg )
{
    initCsvParameters();

    RifParameterXmlReader basicreader( RiaPreferencesGeoMech::current()->geomechWIADefaultXML() );
    if ( !basicreader.parseFile( outErrmsg ) ) return false;

    m_parameters.clear();
    for ( auto group : basicreader.parameterGroups() )
    {
        m_parameters.push_back( group );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::updateVisualization()
{
    generateModelBox();
    RiaApplication::instance()->project()->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( ( changedField == &m_startMD ) || ( changedField == &m_endMD ) || ( changedField == objectToggleField() ) ||
         ( changedField == &m_bufferXY ) || ( changedField == &m_showBox ) )
    {
        updateVisualization();
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellIASettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_geomechCase )
    {
        RimTools::geoMechCaseOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType( wellPath );
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_startMD.uiCapability()->setUiName( "Start MD [m]" );
            m_endMD.uiCapability()->setUiName( "End MD [m]" );
        }
        else if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_startMD.uiCapability()->setUiName( "Start MD [ft]" );
            m_endMD.uiCapability()->setUiName( "End MD [ft]" );
        }
    }

    auto generalGroup = uiOrdering.addNewGroup( "General" );
    generalGroup->add( nameField() );
    generalGroup->add( &m_baseDir );

    auto geoGroup = uiOrdering.addNewGroup( "GeoMechanical Settings" );
    geoGroup->add( &m_geomechCase );
    geoGroup->add( &m_geostaticDate );

    auto modelGroup = uiOrdering.addNewGroup( "Model Settings" );
    modelGroup->add( &m_startMD );
    modelGroup->add( &m_endMD );
    modelGroup->add( &m_bufferXY );
    modelGroup->add( &m_showBox );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_startMD || field == &m_endMD )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr )
        {
            RimWellPath* wellPath = nullptr;
            this->firstAncestorOrThisOfType( wellPath );
            if ( !wellPath ) return;

            myAttr->m_minimum = wellPath->uniqueStartMD();
            myAttr->m_maximum = wellPath->uniqueEndMD();
        }
    }
    else if ( field == &m_geostaticDate )
    {
        caf::PdmUiDateEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDateEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->dateFormat = "dd MMM yyyy";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Return the name to show in the tree selector
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::fullName() const
{
    return QString( "%1 - [%2 - %3]" ).arg( name() ).arg( m_startMD ).arg( m_endMD );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettings::modelBoxValid() const
{
    return m_boxValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellIASettings::modelBoxVertices() const
{
    return m_modelbox.vertices();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellIAModelData*> RimWellIASettings::modelData() const
{
    return m_modelData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellIASettings::userDescriptionField()
{
    return &m_nameProxy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::geomechCaseFilename() const
{
    if ( m_geomechCase ) return m_geomechCase->gridFileName();
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::geomechCaseName() const
{
    QFileInfo fi( geomechCaseFilename() );
    return fi.baseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase* RimWellIASettings::geomechCase() const
{
    return m_geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellIASettings::geostaticDate() const
{
    return m_geostaticDate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::outputBaseDirectory() const
{
    return m_baseDir();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::outputOdbFilename() const
{
    return m_baseDir() + "/" + name() + ".odb";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::outputHeatOdbFilename() const
{
    return m_baseDir() + "/" + name() + "_heat.odb";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::jsonInputFilename() const
{
    return m_baseDir() + "/model_input.json";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellIASettings::csvInputFilename() const
{
    return m_baseDir() + "/model_input.csv";
}

QStringList RimWellIASettings::commandParameters() const
{
    QStringList retlist;

    retlist << m_baseDir();
    retlist << jsonInputFilename();
    retlist << csvInputFilename();

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::setGeoMechCase( RimGeoMechCase* geomechCase )
{
    m_geomechCase = geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::setOutputBaseDirectory( QString baseDir )
{
    m_baseDir = baseDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::setShowBox( bool show )
{
    m_showBox = show;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettings::showBox() const
{
    return m_showBox && m_boxValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<RimParameterGroup*> RimWellIASettings::inputParameterGroups() const
{
    std::list<RimParameterGroup*> retlist;

    for ( auto& group : m_parameters )
        retlist.push_back( group );

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::list<RimParameterGroup*> RimWellIASettings::resinsightParameterGroups() const
{
    std::list<RimParameterGroup*> retlist;

    for ( auto& group : m_parametersRI )
        retlist.push_back( group );

    return retlist;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::resetResInsightParameters()
{
    for ( auto& group : m_parametersRI )
    {
        delete group;
    }
    m_parametersRI.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettings::updateResInsightParameters()
{
    resetResInsightParameters();

    RimParameterGroup* modeldim = new RimParameterGroup();
    modeldim->setName( "model_dimensions" );
    modeldim->setLabel( "Model Dimensions" );
    modeldim->addParameter( "model_size", m_bufferXY );
    modeldim->addParameter( "vertical_length_3D", m_endMD - m_startMD );
    m_parametersRI.push_back( modeldim );

    RimParameterGroup* wellcoords = new RimParameterGroup();
    wellcoords->setName( "well_coordinates" );
    wellcoords->setLabel( "Well Coordinates" );
    wellcoords->addParameter( "x_well", m_modelbox.center().x() );
    wellcoords->addParameter( "y_well", m_modelbox.center().y() );
    wellcoords->addParameter( "z_well", m_modelbox.center().z() );
    m_parametersRI.push_back( wellcoords );

    RimParameterGroup* initcond = new RimParameterGroup();
    initcond->setName( "BC_initial_conditions" );
    initcond->setLabel( "BC Initial Conditions" );
    initcond->addParameter( "analysis_depth", std::abs( m_modelbox.center().z() ) );

    m_parametersRI.push_back( initcond );

    RimParameterGroup* initialStress = new RimParameterGroup();
    initialStress->setName( "initial_stress" );
    initialStress->setLabel( "Initial Stress" );
    initialStress->setComment(
        "SXX is in North direction, SYY is East, SZZ is vertical; PP is the initial pore pressure in the "
        "formation, set to 0 for hydrostatic assumption; inclination is 0 for a vertical well" );

    cvf::Vec3d           position = m_modelbox.center();
    RimWellIADataAccess  dataAccess( m_geomechCase );
    std::vector<QString> nativeKeys{ "S11", "S22", "S33", "S12", "S13", "S23" };
    std::vector<QString> paramKeys{ "SXX", "SYY", "SZZ", "SXY", "SXZ", "SYZ" };

    for ( size_t i = 0; i < nativeKeys.size(); i++ )
    {
        double stressValue =
            dataAccess.interpolatedResultValue( "ST", nativeKeys[i], RigFemResultPosEnum::RIG_ELEMENT_NODAL, position, 0 );
        if ( std::isfinite( stressValue ) )
        {
            initialStress->addParameter( paramKeys[i], stressValue * 100000.0 );
        }
        else
        {
            return false;
        }
    }

    double ppValue = dataAccess.interpolatedResultValue( "POR-Bar", "", RigFemResultPosEnum::RIG_NODAL, position, 0 );
    if ( std::isfinite( ppValue ) )
    {
        initialStress->addParameter( "PP", ppValue * 100000.0 );
    }
    else
    {
        initialStress->addParameter( "PP", 0.0 );
    }

    auto angles = RigWellPathGeometryTools::calculateAzimuthAndInclinationAtMd( ( m_startMD + m_endMD ) / 2.0,
                                                                                wellPath()->wellPathGeometry() );
    initialStress->addParameter( "azimuth_well", angles.first );
    initialStress->addParameter( "inclination_well", angles.second );

    m_parametersRI.push_back( initialStress );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::addCsvGroup( QString name, QStringList timeSteps, double defaultValue /* = 0.0 */ )
{
    RimParameterGroup* group = new RimParameterGroup();
    group->setName( name );
    const int noParams = timeSteps.size();

    for ( int i = 0; i < noParams; i++ )
    {
        group->addParameter( timeSteps[i], defaultValue );
    }
    m_csvParameters.push_back( group );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::initCsvParameters()
{
    m_csvParameters.clear();

    QStringList timeSteps = m_geomechCase->timeStepStrings();

    addCsvGroup( m_csvGroupNames[(size_t)CSV_GROUPNAME::CASING_PRESSURE], timeSteps );
    addCsvGroup( m_csvGroupNames[(size_t)CSV_GROUPNAME::FORMATION_PRESSURE], timeSteps );
    addCsvGroup( m_csvGroupNames[(size_t)CSV_GROUPNAME::TEMPERATURE], timeSteps, 70.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::setDepthInterval( double startMD, double endMD )
{
    m_startMD = startMD;
    m_endMD   = endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIASettings::startMD()
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellIASettings::endMD()
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::resetModelData()
{
    for ( auto& modeldata : m_modelData )
    {
        delete modeldata;
    }
    m_modelData.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellIASettings::wellPath() const
{
    RimWellPath* wellpath = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( wellpath );
    return wellpath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellIASettings::generateModelBox()
{
    RimWellPath* path = wellPath();
    if ( !path ) return;

    RigWellPath* wellgeom = path->wellPathGeometry();
    if ( !wellgeom ) return;

    cvf::Vec3d startPos = wellgeom->interpolatedPointAlongWellPath( m_startMD );
    cvf::Vec3d endPos   = wellgeom->interpolatedPointAlongWellPath( m_endMD );

    m_boxValid = m_modelbox.updateBox( startPos, endPos, m_bufferXY, 0.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellIASettings::extractModelData()
{
    generateModelBox();
    resetModelData();
    if ( !updateResInsightParameters() )
    {
        return false;
    }

    QDateTime startDate = geostaticDate();

    int  timestep        = 0;
    auto timeStepStrings = m_geomechCase->timeStepStrings();

    for ( auto& date : timeStepDates() )
    {
        RimWellIAModelData* data = new RimWellIAModelData();
        data->setDayOffset( startDate.daysTo( date ) );

        for ( auto& group : m_csvParameters )
        {
            if ( group->name() == m_csvGroupNames[(size_t)CSV_GROUPNAME::CASING_PRESSURE] )
            {
                data->setCasingPressure( group->parameterValue( timeStepStrings[timestep] ).toDouble() );
            }
            else if ( group->name() == m_csvGroupNames[(size_t)CSV_GROUPNAME::FORMATION_PRESSURE] )
            {
                data->setFormationPressure( group->parameterValue( timeStepStrings[timestep] ).toDouble() );
            }
            else if ( group->name() == m_csvGroupNames[(size_t)CSV_GROUPNAME::TEMPERATURE] )
            {
                data->setTemperature( group->parameterValue( timeStepStrings[timestep] ).toDouble() );
            }
        }

        const std::vector<cvf::Vec3d> displacements = extractDisplacements( m_modelbox.vertices(), timestep );

        for ( size_t i = 0; i < displacements.size(); i++ )
        {
            data->setDisplacement( (int)i, displacements[i] );
        }
        m_modelData.push_back( data );
        timestep++;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimWellIASettings::timeStepDates()
{
    std::vector<QDateTime> dates = m_geomechCase->timeStepDates();

    if ( dates.size() < (size_t)m_geomechCase->timeStepStrings().size() )
        dates.insert( dates.begin(), m_geostaticDate );

    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellIASettings::extractDisplacements( std::vector<cvf::Vec3d> corners, int timeStep )
{
    RimWellIADataAccess dataAccess( m_geomechCase );

    std::vector<cvf::Vec3d> displacements;

    for ( auto& pos : corners )
    {
        double u1 = dataAccess.interpolatedResultValue( "U", "U1", RigFemResultPosEnum::RIG_NODAL, pos, timeStep );
        double u2 = dataAccess.interpolatedResultValue( "U", "U2", RigFemResultPosEnum::RIG_NODAL, pos, timeStep );
        double u3 = dataAccess.interpolatedResultValue( "U", "U3", RigFemResultPosEnum::RIG_NODAL, pos, timeStep );

        displacements.push_back( cvf::Vec3d( u1, u2, u3 ) );
    }
    return displacements;
}
