/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimEnsembleFileSetCollection.h"
#include "RimEnsembleFileSet.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT( RimEnsembleFileSetCollection, "EnsembleFileSetCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSetCollection::RimEnsembleFileSetCollection()
{
    CAF_PDM_InitObject( "Disc Data", ":/CreateGridCaseGroup16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fileSets, "FileSets", "File Sets", "", "", "" );
    m_fileSets.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::addFileSet( RimEnsembleFileSet* fileSet )
{
    if ( fileSet ) m_fileSets.push_back( fileSet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleFileSet*> RimEnsembleFileSetCollection::fileSets() const
{
    return m_fileSets.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::deleteAllFileSets()
{
    m_fileSets.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::deleteFileSetIfPossible( RimEnsembleFileSet* fileSet )
{
    if ( fileSet )
    {
        // Check if the file set is connected to any objects. If not, remove it from the collection and delete it
        auto connectedObjects = fileSet->objectsWithReferringPtrFields();
        if ( connectedObjects.empty() )
        {
            m_fileSets.removeChild( fileSet );
            delete fileSet;

            updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleFileSetCollection::ensembleFileSetOptions() const
{
    QList<caf::PdmOptionItemInfo> options;
    for ( const auto& fileset : m_fileSets )
    {
        options.push_back( caf::PdmOptionItemInfo( fileset->name(), fileset, false, fileset->uiIconProvider() ) );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::updateFileSetNames()
{
    std::set<std::string> key1;
    std::set<std::string> key2;

    for ( const auto& fileSet : fileSets() )
    {
        const auto keys = fileSet->nameKeys();
        key1.insert( keys.first );
        key2.insert( keys.second );
    }

    bool useKey1 = key1.size() > 1;
    bool useKey2 = key2.size() > 1;

    if ( !useKey1 && !useKey2 )
    {
        useKey2 = true;
    }

    std::set<QString> existingNames;
    for ( auto fileSet : fileSets() )
    {
        fileSet->setUsePathKey1( useKey1 );
        fileSet->setUsePathKey2( useKey2 );
        fileSet->updateName( existingNames );
        existingNames.insert( fileSet->name() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicImportEnsembleFileSetFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleFileSetCollection::initAfterRead()
{
    updateFileSetNames();
}
