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

    CAF_PDM_InitField( &m_replaceWells, "ReplaceWells", true, "Wells" );
    CAF_PDM_InitField( &m_replaceWellGroups, "ReplaceWellGroups", true, "Well Groups" );
    CAF_PDM_InitField( &m_replaceRegions, "ReplaceRegions", true, "Regions" );
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
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForWells() const
{
    return m_replaceWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForWellGroups() const
{
    return m_replaceWellGroups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSaveMultiPlotTemplateFeatureSettings::usePlacholderForRegions() const
{
    return m_replaceRegions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveMultiPlotTemplateFeatureSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );

    {
        auto group = uiOrdering.addNewGroup( "Use Placeholders for Objects" );
        group->add( &m_replaceWells );
        group->add( &m_replaceWellGroups );
        group->add( &m_replaceRegions );
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
            attr->m_selectSaveFileName  = true;
            attr->m_fileSelectionFilter = "Plot Template Files(*.rpt);; All files(*.*)";
        }
    }
}
