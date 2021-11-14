////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimPlotTemplateFolderItem.h"

#include "RiaPreferences.h"

#include "RimPlotTemplateFileItem.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafUtils.h"

#include <QDir>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RimPlotTemplateFolderItem, "PlotTemplateCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFolderItem::RimPlotTemplateFolderItem()
{
    CAF_PDM_InitObject( "PlotTemplateCollection", ":/Folder.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_folderName, "FolderName", "Folder" );
    CAF_PDM_InitFieldNoDefault( &m_fileNames, "FileNames", "" );
    m_fileNames.uiCapability()->setUiTreeHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_subFolders, "SubFolders", "" );
    m_subFolders.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFolderItem::~RimPlotTemplateFolderItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::createRootFolderItemsFromFolderPaths( const QStringList& folderPaths )
{
    m_fileNames.deleteAllChildObjects();
    m_subFolders.deleteAllChildObjects();

    createSubFolderItemsFromFolderPaths( folderPaths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotTemplateFileItem*> RimPlotTemplateFolderItem::fileNames() const
{
    return m_fileNames.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotTemplateFolderItem*> RimPlotTemplateFolderItem::subFolders() const
{
    return m_subFolders.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::appendOptionItemsForPlotTemplates( QList<caf::PdmOptionItemInfo>& options,
                                                                   RimPlotTemplateFolderItem*     templateFolderItem )
{
    appendOptionItemsForPlotTemplatesRecursively( options, templateFolderItem, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::setFolderPath( const QString& path )
{
    m_folderName.v().setPath( path );

    QFileInfo fi( path );
    this->uiCapability()->setUiName( fi.baseName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::searchForFileAndFolderNames()
{
    m_fileNames.deleteAllChildObjects();
    m_subFolders.deleteAllChildObjects();

    if ( m_folderName().path().isEmpty() )
    {
        for ( size_t i = 0; i < m_subFolders.size(); ++i )
        {
            if ( m_subFolders[i] ) m_subFolders[i]->searchForFileAndFolderNames();
        }
        return;
    }

    QDir myDir( this->m_folderName().path() );
    if ( !myDir.isReadable() )
    {
        return;
    }

    // Build a list of all scripts in the specified directory
    {
        QStringList nameFilters;
        nameFilters << "*.rpt";
        QStringList fileList = caf::Utils::getFilesInDirectory( m_folderName().path(), nameFilters, true );

        for ( int i = 0; i < fileList.size(); i++ )
        {
            const QString& fileName = fileList.at( i );

            if ( caf::Utils::fileExists( fileName ) )
            {
                RimPlotTemplateFileItem* fileItem = new RimPlotTemplateFileItem();
                fileItem->setFilePath( fileName );
                m_fileNames.push_back( fileItem );
            }
        }
    }

    if ( searchSubFoldersRecursively() )
    {
        QStringList folderPaths;

        QDir          dir( m_folderName().path() );
        QFileInfoList fileInfoList = dir.entryInfoList( QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable );

        for ( const auto& fi : fileInfoList )
        {
            folderPaths.push_back( fi.absoluteFilePath() );
        }

        createSubFolderItemsFromFolderPaths( folderPaths );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    if ( &m_folderName == changedField )
    {
        QFileInfo fi( m_folderName().path() );
        this->setUiName( fi.baseName() );

        this->searchForFileAndFolderNames();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_folderName )
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->m_selectDirectory = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::appendOptionItemsForPlotTemplatesRecursively( QList<caf::PdmOptionItemInfo>& options,
                                                                              RimPlotTemplateFolderItem* templateFolderItem,
                                                                              int                        menuLevel )
{
    {
        auto subFolders = templateFolderItem->subFolders();
        for ( auto sub : subFolders )
        {
            caf::PdmOptionItemInfo optionInfo = caf::PdmOptionItemInfo::createHeader( sub->uiName(), true );
            optionInfo.setLevel( menuLevel );
            options.push_back( optionInfo );

            appendOptionItemsForPlotTemplatesRecursively( options, sub, menuLevel + 1 );
        }
    }

    caf::IconProvider templateIcon( ":/SummaryTemplate16x16.png" );

    auto files = templateFolderItem->fileNames();
    for ( auto file : files )
    {
        caf::PdmOptionItemInfo optionInfo( file->uiName(), file, false, templateIcon );

        optionInfo.setLevel( menuLevel );

        options.push_back( optionInfo );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFolderItem::createSubFolderItemsFromFolderPaths( const QStringList& folderPaths )
{
    for ( const auto& path : folderPaths )
    {
        RimPlotTemplateFolderItem* scriptLocation = new RimPlotTemplateFolderItem();
        scriptLocation->setFolderPath( path );
        scriptLocation->searchForFileAndFolderNames();

        m_subFolders.push_back( scriptLocation );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotTemplateFolderItem::searchSubFoldersRecursively() const
{
    return RiaPreferences::current()->searchPlotTemplateFoldersRecursively();
}
