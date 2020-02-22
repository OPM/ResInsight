/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimScriptCollection.h"

#include "RimCalcScript.h"
#include "RiuMainWindow.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafUtils.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RimScriptCollection, "ScriptLocation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimScriptCollection::RimScriptCollection()
{
    CAF_PDM_InitObject( "ScriptLocation", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &directory, "ScriptDirectory", "Folder", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &calcScripts, "CalcScripts", "", "", "", "" );
    calcScripts.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &subDirectories, "SubDirectories", "", "", "", "" );
    subDirectories.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_searchSubFolders, "SearchSubFolders", false, "Add Subfolders", "", "", "" );

    directory.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimScriptCollection::~RimScriptCollection()
{
    calcScripts.deleteAllChildObjects();
    subDirectories.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::readContentFromDisc()
{
    calcScripts.deleteAllChildObjects();

    if ( directory().isEmpty() )
    {
        for ( size_t i = 0; i < subDirectories.size(); ++i )
        {
            if ( subDirectories[i] ) subDirectories[i]->readContentFromDisc();
        }
        return;
    }

    QDir myDir( this->directory() );
    if ( !myDir.isReadable() )
    {
        return;
    }

    // Build a list of all scripts in the specified directory
    {
        QStringList nameFilters;
        nameFilters << "*.m"
                    << "*.py";
        QStringList fileList = caf::Utils::getFilesInDirectory( directory, nameFilters, true );

        int i;
        for ( i = 0; i < fileList.size(); i++ )
        {
            const QString& fileName = fileList.at( i );

            if ( caf::Utils::fileExists( fileName ) )
            {
                RimCalcScript* calcScript    = new RimCalcScript;
                calcScript->absoluteFileName = fileName;

                QFileInfo fi( fileName );
                calcScript->setUiName( fi.baseName() );

                calcScripts.push_back( calcScript );
            }
        }
    }

    subDirectories.deleteAllChildObjects();

    if ( m_searchSubFolders() )
    {
        QDir          dir( directory );
        QFileInfoList fileInfoList = dir.entryInfoList( QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable );

        QListIterator<QFileInfo> it( fileInfoList );
        while ( it.hasNext() )
        {
            QFileInfo fi = it.next();
            if ( fi.baseName() != "__pycache__" )
            {
                RimScriptCollection* scriptLocation = new RimScriptCollection;
                scriptLocation->directory           = fi.absoluteFilePath();
                scriptLocation->setUiName( fi.baseName() );
                scriptLocation->readContentFromDisc();

                subDirectories.push_back( scriptLocation );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    if ( &directory == changedField )
    {
        QFileInfo fi( directory );
        this->setUiName( fi.baseName() );
        this->readContentFromDisc();
    }
    else if ( &m_searchSubFolders == changedField )
    {
        this->readContentFromDisc();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &directory )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
}
