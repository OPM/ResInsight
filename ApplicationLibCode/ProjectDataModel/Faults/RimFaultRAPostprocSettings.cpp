/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RimFaultRAPostprocSettings.h"

#include "RimDoubleParameter.h"
#include "RimFaultRASettings.h"
#include "RimParameterGroup.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPostprocSettings::RimFaultRAPostprocSettings()
{
    CAF_PDM_InitObject( "Reactivation Assessment Postproc Settings", ":/fault_react_24x24.png", "", "" );

    CAF_PDM_InitField( &m_baseDir, "BaseDir", QString( "" ), "Working Directory", "", "", "" );
    CAF_PDM_InitField( &m_startTimestepEclipse, "StartTimeStepEclipse", 0, "Start Time Step", "", "", "" );
    CAF_PDM_InitField( &m_endTimestepEclipse, "EndTimeStepEclipse", 0, "End Time Step", "", "", "" );
    CAF_PDM_InitField( &m_geomechEnabled, "GeomechEnabled", false, "GeoMechanical Input Available", "", "", "" );
    CAF_PDM_InitField( &m_basicMacrisDatabase, "BasicMacrisDatabase", QString( "" ), "Basic Macris Database", "", "", "" );
    CAF_PDM_InitField( &m_advancedMacrisDatabase, "AdvancedMacrisDatabase", QString( "" ), "Advanced Macris Database", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_postprocParameters, "PostprocParameters", "Post-Processing Parameters", "", "", "" );
    m_postprocParameters = new RimParameterGroup();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPostprocSettings::~RimFaultRAPostprocSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPostprocSettings::initFromSettings( RimFaultRASettings* settings )
{
    m_geomechEnabled = settings->geomechCase() != nullptr;

    m_baseDir              = settings->outputBaseDirectory();
    m_startTimestepEclipse = settings->startTimeStepEclipseIndex();
    m_endTimestepEclipse   = settings->endTimeStepEclipseIndex();

    m_basicMacrisDatabase    = settings->basicMacrisDatabase();
    m_advancedMacrisDatabase = settings->advancedMacrisDatabase();

    RimDoubleParameter* friction_angle =
        dynamic_cast<RimDoubleParameter*>( settings->getInputParameter( "friction_angle" ) );
    if ( friction_angle != nullptr )
    {
        m_postprocParameters->addParameter( "friction_coef", std::atan( friction_angle->value() ) );
    }

    RimDoubleParameter* rho_rock = dynamic_cast<RimDoubleParameter*>( settings->getInputParameter( "rho_rock" ) );
    if ( rho_rock != nullptr )
    {
        m_postprocParameters->addParameter( "rockdensity", rho_rock->value() );
    }

    RimDoubleParameter* k0_effective =
        dynamic_cast<RimDoubleParameter*>( settings->getInputParameter( "k0_effective" ) );
    if ( k0_effective != nullptr )
    {
        m_postprocParameters->addParameter( "k0", k0_effective->value() );
    }

    RimDoubleParameter* sh_ratio = dynamic_cast<RimDoubleParameter*>( settings->getInputParameter( "sh_ratio" ) );
    if ( sh_ratio != nullptr )
    {
        m_postprocParameters->addParameter( "sh_ratio", sh_ratio->value() );
    }

    RimDoubleParameter* s_azimuth = dynamic_cast<RimDoubleParameter*>( settings->getInputParameter( "s_azimuth" ) );
    if ( s_azimuth != nullptr )
    {
        m_postprocParameters->addParameter( "sh_azim", s_azimuth->value() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimFaultRAPostprocSettings::stepsToLoad()
{
    QStringList timesteps;
    timesteps.push_back( QString::number( m_startTimestepEclipse() ) );
    timesteps.push_back( QString::number( m_endTimestepEclipse() ) );

    return timesteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::postprocParameterFilename( int faultID ) const
{
    QString retval = m_baseDir;
    retval += QString( "/tmp/postproc_%1.json" ).arg( faultID, 3, 10, QChar( '0' ) );
    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::outputBaseDirectory() const
{
    return m_baseDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::advancedMacrisDatabase() const
{
    return m_advancedMacrisDatabase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPostprocSettings::basicMacrisDatabase() const
{
    return m_basicMacrisDatabase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterGroup* RimFaultRAPostprocSettings::parameters() const
{
    return m_postprocParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRAPostprocSettings::geomechEnabled() const
{
    return m_geomechEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimFaultRAPostprocSettings::postprocCommandParameters( int faultID ) const
{
    QStringList retlist;

    retlist << postprocParameterFilename( faultID );

    return retlist;
}
