/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <QIcon>
#include <vector>

namespace caf
{
class CmdFeatureMenuBuilder;
}

class QString;
class QStringList;
class QMenu;
class RimWellPath;
class RimScriptCollection;
class RimPlotTemplateFolderItem;

//==================================================================================================
///
///
//==================================================================================================
class RimContextCommandBuilder
{
public:
    static caf::CmdFeatureMenuBuilder commandsFromSelection();

private:
    static std::vector<RimWellPath*> allWellPaths();
    static void                      createExecuteScriptForCasesFeatureMenu( caf::CmdFeatureMenuBuilder& menuBuilder );

    static void appendScriptItems( caf::CmdFeatureMenuBuilder& menuBuilder, RimScriptCollection* scriptCollection );
    static void appendPlotTemplateItems( caf::CmdFeatureMenuBuilder& menuBuilder,
                                         RimPlotTemplateFolderItem*  plotTemplateRoot );

    static int appendImportMenu( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu = false );
    static int appendCreateCompletions( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu = false );
    static int appendExportCompletions( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu = false );
    static int appendExportWellPaths( caf::CmdFeatureMenuBuilder& menuBuilder, bool addSeparatorBeforeMenu = false );
    static int appendSubMenuWithCommands( caf::CmdFeatureMenuBuilder& menuBuilder,
                                          const QStringList&          commandCandidates,
                                          const QString&              menuLabel,
                                          const QIcon&                menuIcon               = QIcon(),
                                          bool                        addSeparatorBeforeMenu = false );
};
