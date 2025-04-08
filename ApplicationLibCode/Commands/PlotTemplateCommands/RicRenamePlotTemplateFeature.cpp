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

#include "RicRenamePlotTemplateFeature.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicRenamePlotTemplateFeature, "RicRenamePlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRenamePlotTemplateFeature::isCommandEnabled() const
{
    auto file = caf::SelectionManager::instance()->selectedItemOfType<RimPlotTemplateFileItem>();
    return ( file != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenamePlotTemplateFeature::onActionTriggered( bool isChecked )
{
    auto file = caf::SelectionManager::instance()->selectedItemOfType<RimPlotTemplateFileItem>();
    if ( file == nullptr ) return;

    QFileInfo fi( file->absoluteFilePath() );

    QWidget* parent = RiuPlotMainWindow::instance();

    bool    ok;
    QString newname = QInputDialog::getText( parent, "Rename Plot Template", "Enter new name:", QLineEdit::Normal, fi.baseName(), &ok );

    if ( !ok ) return;

    newname = newname.trimmed();

    if ( newname.isEmpty() || newname.contains( "/" ) || newname.contains( "\\" ) )
    {
        QMessageBox::critical( parent, "Rename failed", "Invalid name given.", QMessageBox::Ok );
        return;
    }

    QString newPath = fi.absolutePath() + "/" + newname + "." + fi.completeSuffix();

    if ( !QFile::rename( file->absoluteFilePath(), newPath ) )
    {
        QMessageBox::critical( parent, "Rename failed", "Unable to rename the selected plot template.", QMessageBox::Ok );
        return;
    }

    file->setFilePath( newPath );
    file->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRenamePlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Rename" );
}
