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

#include "RicSetAsDefaultTemplateFeature.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"

#include "RiaPreferencesSummary.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT( RicSetAsDefaultTemplateFeature, "RicSetAsDefaultTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSetAsDefaultTemplateFeature::isCommandEnabled() const
{
    return selectedTemplate() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetAsDefaultTemplateFeature::onActionTriggered( bool isChecked )
{
    RimPlotTemplateFileItem* file = selectedTemplate();
    if ( file == nullptr ) return;

    if ( isChecked )
        RiaPreferencesSummary::current()->addToDefaultPlotTemplates( file->absoluteFilePath() );
    else
        RiaPreferencesSummary::current()->removeFromDefaultPlotTemplates( file->absoluteFilePath() );

    file->updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetAsDefaultTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Default Template" );
    // actionToSetup->setIcon( QIcon( ":/plot-template-standard.svg" ) );

    RimPlotTemplateFileItem* file = selectedTemplate();
    if ( file != nullptr )
    {
        actionToSetup->setCheckable( true );
        actionToSetup->setChecked( file->isDefaultTemplate() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFileItem* RicSetAsDefaultTemplateFeature::selectedTemplate() const
{
    return caf::SelectionManager::instance()->selectedItemOfType<RimPlotTemplateFileItem>();
}
