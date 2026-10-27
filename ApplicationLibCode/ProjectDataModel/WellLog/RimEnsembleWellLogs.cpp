/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleWellLogs.h"

#include "RiaLogging.h"

#include "RimWellLogChannel.h"
#include "RimWellLogLasFile.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <map>

CAF_PDM_SOURCE_INIT( RimEnsembleWellLogs, "EnsembleWellLogs" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogs::RimEnsembleWellLogs()
{
    CAF_PDM_InitScriptableObject( "Ensemble Well Logs", ":/WellLogFile.svg" );
    setDeletable( true );

    CAF_PDM_InitFieldNoDefault( &m_wellLogFiles, "WellLogFiles", "" );

    CAF_PDM_InitFieldNoDefault( &m_summary, "Summary", "Summary" );
    m_summary.registerGetMethod( this, &RimEnsembleWellLogs::summaryText );
    m_summary.uiCapability()->setUiReadOnly( true );
    m_summary.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_summary.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_summary.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleWellLogs::summaryText() const
{
    const auto files = wellLogFiles();

    std::map<QString, size_t> fileCountPerChannel;
    for ( const auto* file : files )
    {
        for ( const auto* channel : file->wellLogChannels() )
        {
            fileCountPerChannel[channel->name()]++;
        }
    }

    QString text = QString( "Number of files: %1\n" ).arg( files.size() );
    text += QString( "Number of channels: %1\n" ).arg( fileCountPerChannel.size() );

    if ( !fileCountPerChannel.empty() )
    {
        text += "\nChannels (number of files containing channel):\n";
        for ( const auto& [name, count] : fileCountPerChannel )
        {
            text += QString( "  %1 (%2)\n" ).arg( name ).arg( count );
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_summary );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    // List the LAS files, but not their child objects (well log channels)
    for ( auto* file : wellLogFiles() )
    {
        uiTreeOrdering.add( file )->skipRemainingChildren( true );
    }
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::removeWellLogFile( RimWellLogLasFile* summaryCase )
{
    m_wellLogFiles.removeChild( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::addWellLogFile( RimWellLogLasFile* summaryCase )
{
    m_wellLogFiles.push_back( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogLasFile*> RimEnsembleWellLogs::wellLogFiles() const
{
    return m_wellLogFiles().childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::loadDataAndUpdate()
{
    for ( auto& w : m_wellLogFiles )
    {
        QString errorMessage;
        if ( !w->readFile( &errorMessage ) )
        {
            RiaLogging::warning( errorMessage.toStdString() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewEnsembleWellLogPlotFeature";
    menuBuilder << "Separator";
    menuBuilder << "RicDeleteItemFeature";
}
