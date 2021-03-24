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

#include "RiaApplication.h"
#include "RimEclipseResultCase.h"
#include "RimGenericParameter.h"
#include "RimGeoMechCase.h"
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
    : m_basicParameterNames( { "e_reservoir",
                               "e_overburden",
                               "e_underburden",
                               "v",
                               "eclipse_smooth_faults",
                               "octree_vertical_subdivision",
                               "octree_local_tree_levels",
                               "octree_minimum_resolution",
                               "peak_filter_factor",
                               "observation_points_resolution" } )
    , m_ignoreParameterNames( { "fault_id", "eclipse_input_grid" } )
{
    CAF_PDM_InitObject( "Reactivation Assessment Settings", ":/fault_react_24x24.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case", "", "", "" );
    m_eclipseCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_geomechCase, "GeomechCase", "GeoMech Case", "", "", "" );
    m_geomechCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_baseDir, "BaseDir", "Output Directory", "", "", "" );
    m_baseDir.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_basicParameters, "BasicParameters", "Basic Calculation Parameters", "", "", "" );
    m_basicParameters.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_basicParameters.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_basicParameters.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitFieldNoDefault( &m_additionalParameters, "AdditionalParameters", "Additional Calculation Parameters", "", "", "" );
    m_additionalParameters.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_additionalParameters.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_additionalParameters.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitFieldNoDefault( &m_advancedParameters, "AdvancedParameters", "Advanced Parameters", "", "", "" );
    m_advancedParameters.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_advancedParameters.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );
    m_advancedParameters.uiCapability()->setCustomContextMenuEnabled( true );
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
    if ( ( field == &m_basicParameters ) || ( field == &m_additionalParameters ) || ( field == &m_advancedParameters ) )
    {
        auto tvAttribute = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>( attribute );
        if ( tvAttribute )
        {
            tvAttribute->resizePolicy              = caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER;
            tvAttribute->alwaysEnforceResizePolicy = true;
            tvAttribute->minimumHeight             = 300;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRASettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto basicGroup = uiOrdering.addNewGroup( "Basic Parameters" );
    basicGroup->add( &m_basicParameters );

    auto advGroup = uiOrdering.addNewGroup( "Advanced Parameters" );
    advGroup->add( &m_advancedParameters );

    auto addGroup = uiOrdering.addNewGroup( "Additional Parameters" );
    addGroup->add( &m_additionalParameters );
    addGroup->setCollapsedByDefault( true );

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
RimEclipseResultCase* RimFaultRASettings::eclipseCase() const
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
void RimFaultRASettings::useDefaultValuesFromFile( QString xmlFilename )
{
    RifParameterXmlReader reader( xmlFilename );
    QString               errorText;
    if ( !reader.parseFile( errorText ) )
    {
        // todo - log warning?
        return;
    }

    for ( auto p : reader.parameters() )
    {
        if ( shouldIgnoreParameter( p->name() ) ) continue;

        if ( p->isAdvanced() )
            m_additionalParameters.push_back( p );
        else
            m_basicParameters.push_back( p );

        // TODO - fix memleak for ignored parameters, should switch to some sort of refcounting
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRASettings::isBasicParameter( QString name ) const
{
    return std::find( m_basicParameterNames.begin(), m_basicParameterNames.end(), name ) != m_basicParameterNames.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRASettings::shouldIgnoreParameter( QString name ) const
{
    return std::find( m_ignoreParameterNames.begin(), m_ignoreParameterNames.end(), name ) != m_ignoreParameterNames.end();
}
