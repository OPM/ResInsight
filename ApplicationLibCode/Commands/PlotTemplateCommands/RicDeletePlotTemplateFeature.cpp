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

#include "RicDeletePlotTemplateFeature.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"

#include "RicReloadPlotTemplatesFeature.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicDeletePlotTemplateFeature, "RicDeletePlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeletePlotTemplateFeature::isCommandEnabled() const
{
    auto object = caf::SelectionManager::instance()->selectedItemOfType<RimPlotTemplateFileItem>();
    return object != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePlotTemplateFeature::onActionTriggered( bool isChecked )
{
    auto file = caf::SelectionManager::instance()->selectedItemOfType<RimPlotTemplateFileItem>();
    if ( file == nullptr ) return;

    QWidget* parent = RiuPlotMainWindow::instance();

    QString question = "Do you want to delete the plot template file\n\n" + file->absoluteFilePath();

    if ( QMessageBox::question( parent, "Delete Template File?", question ) == QMessageBox::No ) return;

    if ( !QFile::remove( file->absoluteFilePath() ) )
    {
        QMessageBox::critical( parent, "Delete failed", "Unable to remove the selected plot template.", QMessageBox::Ok );
        return;
    }

    RicReloadPlotTemplatesFeature::rebuildFromDisc();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeletePlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
}
