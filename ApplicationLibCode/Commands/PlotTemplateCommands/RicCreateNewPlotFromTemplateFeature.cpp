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

#include "RicCreateNewPlotFromTemplateFeature.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"

#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RicSelectCaseOrEnsembleUi.h"
#include "Summary/RiaSummaryPlotTemplateTools.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QString>

CAF_CMD_SOURCE_INIT( RicCreateNewPlotFromTemplateFeature, "RicCreateNewPlotFromTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateNewPlotFromTemplateFeature::isCommandEnabled() const
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1 ) return false;

    RimPlotTemplateFileItem* file = dynamic_cast<RimPlotTemplateFileItem*>( selectedItems[0] );
    return ( file != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateNewPlotFromTemplateFeature::onActionTriggered( bool isChecked )
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 1 ) return;

    RimPlotTemplateFileItem* file = dynamic_cast<RimPlotTemplateFileItem*>( selectedItems[0] );
    if ( file == nullptr ) return;

    RimSummaryMultiPlot* plot = nullptr;

    if ( file->isEnsembleTemplate() )
    {
        auto ensemble = selectEnsemble();
        if ( !ensemble ) return;

        plot = RicSummaryPlotTemplateTools::create( file->absoluteFilePath(), {}, { ensemble } );
    }
    else
    {
        auto sumCase = selectSummaryCase();
        if ( !sumCase ) return;

        plot = RicSummaryPlotTemplateTools::create( file->absoluteFilePath(), { sumCase }, {} );
    }

    if ( plot != nullptr )
    {
        RiaPreferences::current()->setLastUsedPlotTemplatePath( file->absoluteFilePath() );
        RiaPreferences::current()->writePreferencesToApplicationStore();
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateNewPlotFromTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create New Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RicCreateNewPlotFromTemplateFeature::selectSummaryCase()
{
    RiuPlotMainWindow*        plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    RicSelectCaseOrEnsembleUi ui;

    ui.setEnsembleSelectionMode( false );

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, &ui, "Create New Plot - Select Summary Case", "" );
    propertyDialog.resize( QSize( 400, 200 ) );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        return ui.selectedSummaryCase();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RicCreateNewPlotFromTemplateFeature::selectEnsemble()
{
    RiuPlotMainWindow*        plotwindow = RiaGuiApplication::instance()->mainPlotWindow();
    RicSelectCaseOrEnsembleUi ui;

    ui.setEnsembleSelectionMode( true );

    caf::PdmUiPropertyViewDialog propertyDialog( plotwindow, &ui, "Create New Plot - Select Ensemble", "" );
    propertyDialog.resize( QSize( 400, 200 ) );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        return ui.selectedEnsemble();
    }

    return nullptr;
}
