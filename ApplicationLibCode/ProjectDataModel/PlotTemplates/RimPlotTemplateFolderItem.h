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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

namespace caf
{
class PdmUiEditorAttribute;
}

class RimPlotTemplateFileItem;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotTemplateFolderItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPlotTemplateFolderItem();
    ~RimPlotTemplateFolderItem() override;

    void createRootFolderItemsFromFolderPaths( const QStringList& folderPaths );

    std::vector<RimPlotTemplateFileItem*>   fileItems() const;
    std::vector<RimPlotTemplateFolderItem*> subFolders() const;

    static void appendOptionItemsForPlotTemplates( QList<caf::PdmOptionItemInfo>& options, RimPlotTemplateFolderItem* templateFolderItem );

    static void allPlotTemplates( std::vector<RimPlotTemplateFileItem*>& fileItems, RimPlotTemplateFolderItem* templateFolderItem );
    void        updateIconState() const;

private:
    void searchForFileAndFolderNames( int levelsLeft );
    void setFolderPath( const QString& path );
    void createSubFolderItemsFromFolderPaths( const QStringList& folderPaths, int levelsLeft );

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    static void appendOptionItemsForPlotTemplatesRecursively( QList<caf::PdmOptionItemInfo>& options,
                                                              RimPlotTemplateFolderItem*     templateFolderItem,
                                                              int                            menuLevel );

private:
    caf::PdmField<caf::FilePath>                        m_folderName;
    caf::PdmChildArrayField<RimPlotTemplateFileItem*>   m_fileNames;
    caf::PdmChildArrayField<RimPlotTemplateFolderItem*> m_subFolders;
};
