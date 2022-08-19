////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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
#include "RicSaveMultiPlotTemplateFeatureSettings.h"
#include "cafPdmUiFilePathEditor.h"

CAF_PDM_SOURCE_INIT( RicSaveMultiPlotTemplateFeatureSettings, "RicSaveMultiPlotTemplateFeatureSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSaveMultiPlotTemplateFeatureSettings::RicSaveMultiPlotTemplateFeatureSettings()
{
    CAF_PDM_InitObject( "Save Summary Plot", ":/CrossSection16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );
    CAF_PDM_InitFieldNoDefault( &m_name, "Name", "Name" );

    CAF_PDM_InitField( &m_persistObjectNameForWells, "PersistObjectNameWells", false, "Wells" );
    CAF_PDM_InitField( &m_persistObjectNameGroups, "PersistObjectNameGroups", false, "Groups" );
    CAF_PDM_InitField( &m_persistObjectNameRegions, "PersistObjectNameRegions", false, "Regions" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeatureSettings::setFilePath( const QString& filePath )
{
    m_filePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSaveMultiPlotTemplateFeatureSettings::filePath() const
{
    return m_filePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeatureSettings::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicSaveMultiPlotTemplateFeatureSettings::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForWells() const
{
    return !m_persistObjectNameForWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForGroups() const
{
    return !m_persistObjectNameGroups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForRegions() const
{
    return !m_persistObjectNameRegions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeatureSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_name );

    {
        auto group = uiOrdering.addNewGroup( "Persist Object Names" );
        group->setCollapsedByDefault();
        group->add( &m_persistObjectNameForWells );
        group->add( &m_persistObjectNameGroups );
        group->add( &m_persistObjectNameRegions );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeatureSettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                                     QString                    uiConfigName,
                                                                     caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_filePath )
    {
        auto attr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->m_selectDirectory = true;
        }
    }
}
