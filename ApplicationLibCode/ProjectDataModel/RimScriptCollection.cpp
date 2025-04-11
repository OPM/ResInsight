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
    CAF_PDM_InitObject( "ScriptLocation", ":/Folder.png" );

    CAF_PDM_InitFieldNoDefault( &directory, "ScriptDirectory", "Folder" );
    directory.uiCapability()->setUiReadOnly( true );
    directory.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &calcScripts, "CalcScripts", "" );
    CAF_PDM_InitFieldNoDefault( &subDirectories, "SubDirectories", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::readContentFromDisc( int folderLevelsLeft )
{
    calcScripts.deleteChildren();
    subDirectories.deleteChildren();

    folderLevelsLeft--;
    if ( folderLevelsLeft < 0 ) return;

    if ( directory().isEmpty() )
    {
        for ( size_t i = 0; i < subDirectories.size(); ++i )
        {
            if ( subDirectories[i] ) subDirectories[i]->readContentFromDisc( folderLevelsLeft );
        }
        return;
    }

    QDir myDir( directory() );
    if ( !myDir.isReadable() )
    {
        return;
    }

    // Build a list of all scripts in the specified directory
    {
        QStringList nameFilters;
        nameFilters << "*.m" << "*.py";
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

    if ( folderLevelsLeft > 0 )
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
                scriptLocation->readContentFromDisc( folderLevelsLeft );

                subDirectories.push_back( scriptLocation );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimScriptCollection::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
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
