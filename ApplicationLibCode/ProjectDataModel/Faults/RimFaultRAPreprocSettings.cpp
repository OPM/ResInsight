/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021  Equinor ASA
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

#include "RimFaultRAPreprocSettings.h"

#include "RiaApplication.h"
#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimProject.h"
#include "RimTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimFaultRAPreprocSettings, "RimFaultRAPreprocSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPreprocSettings::RimFaultRAPreprocSettings()
{
    CAF_PDM_InitObject( "Fault RA Preproc Settings", ":/fault_react_24x24.png", "", "" );

    CAF_PDM_InitField( &m_startTimestep, "StartTimeStep", 0, "Start Time Step", "", "", "" );

    CAF_PDM_InitField( &m_endTimestep, "EndTimeStep", 0, "End Time Step", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case", "", "", "" );
    m_eclipseCase.setValue( nullptr );

    CAF_PDM_InitFieldNoDefault( &m_geomechCase, "GeomechCase", "GeoMech Case", "", "", "" );
    m_geomechCase.setValue( nullptr );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDir", "Output Directory", "", "", "" );
    m_baseDir.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_cleanBaseDir, "CleanBaseDir", false, "Clean Output Directory", "", "", "" );
    CAF_PDM_InitField( &m_smoothEclipseData, "SmoothEclipseData", false, "Smooth Eclipse Data", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAPreprocSettings::~RimFaultRAPreprocSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto eclipseGroup = uiOrdering.addNewGroup( "Eclipse Model" );
    eclipseGroup->add( &m_eclipseCase );
    eclipseGroup->add( &m_smoothEclipseData );

    if ( m_geomechCase() )
    {
        auto geomechGroup = uiOrdering.addNewGroup( "GeoMechanical Model" );
        geomechGroup->add( &m_geomechCase );
    }

    auto timeGroup = uiOrdering.addNewGroup( "Time Steps" );
    timeGroup->add( &m_startTimestep );
    timeGroup->add( &m_endTimestep );

    auto outputGroup = uiOrdering.addNewGroup( "Output Settings" );
    outputGroup->add( &m_baseDir );
    outputGroup->add( &m_cleanBaseDir );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimFaultRAPreprocSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    if ( fieldNeedingOptions == &m_geomechCase )
    {
        RimTools::geoMechCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_startTimestep )
    {
        RimTools::timeStepsForCase( startCase(), &options );
    }
    else if ( fieldNeedingOptions == &m_endTimestep )
    {
        RimTools::timeStepsForCase( startCase(), &options );
    }

    return options;
}

RimCase* RimFaultRAPreprocSettings::startCase() const
{
    if ( m_geomechCase() ) return m_geomechCase();
    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRAPreprocSettings::startTimeStepIndex() const
{
    return m_startTimestep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::startTimeStep() const
{
    if ( startCase() )
    {
        if ( ( m_startTimestep >= 0 ) && ( m_startTimestep <= startCase()->timeStepStrings().size() ) )
            return startCase()->timeStepStrings()[m_startTimestep];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFaultRAPreprocSettings::endTimeStepIndex() const
{
    return m_endTimestep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::endTimeStep() const
{
    if ( startCase() )
    {
        if ( ( m_endTimestep >= 0 ) && ( m_endTimestep <= startCase()->timeStepStrings().size() ) )
            return startCase()->timeStepStrings()[m_endTimestep];
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::eclipseCaseFilename() const
{
    if ( m_eclipseCase ) return m_eclipseCase->gridFileName();
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::geomechCaseFilename() const
{
    if ( m_geomechCase ) return m_geomechCase->gridFileName();
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::preprocParameterFilename() const
{
    return m_baseDir + "/pre_processing.json";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAPreprocSettings::outputBaseDirectory() const
{
    return m_baseDir();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRAPreprocSettings::cleanBaseDirectory() const
{
    return m_cleanBaseDir();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRAPreprocSettings::smoothEclipseData() const
{
    return m_smoothEclipseData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::setGeoMechCase( RimGeoMechCase* geomechCase )
{
    m_geomechCase = geomechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::setEclipseCase( RimEclipseResultCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAPreprocSettings::setOutputBaseDirectory( QString baseDir )
{
    m_baseDir = baseDir;
}
