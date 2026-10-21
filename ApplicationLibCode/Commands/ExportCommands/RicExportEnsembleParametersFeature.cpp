/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "RicExportEnsembleParametersFeature.h"

#include "RiaGuiApplication.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "RiuFileDialogTools.h"

#include "cafSelectionManager.h"

#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicExportEnsembleParametersFeature, "RicExportEnsembleParametersFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEnsembleParametersFeature::onActionTriggered( bool isChecked )
{
    auto parent = RiaGuiApplication::widgetToUseAsParent();

    auto ensemble = dynamic_cast<RimSummaryEnsemble*>( caf::SelectionManager::instance()->selectedItem() );
    if ( ensemble == nullptr ) return;

    if ( !ensemble->hasEnsembleParameters() )
    {
        QMessageBox::information( parent, "Export Ensemble Parameters", "No parameters found in the selected ensemble!" );
        return;
    }

    QString filePath = RiuFileDialogTools::getSaveFileName( parent, "Export Ensemble Parameters", "", "CSV files (*.csv);;All files(*.*)" );
    if ( filePath.isEmpty() ) return;

    RicExportEnsembleParametersFeature::exportParameters( ensemble, filePath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEnsembleParametersFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Parameter.svg" ) );
    actionToSetup->setText( "Export Ensemble Parameters..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEnsembleParametersFeature::exportParameters( RimSummaryEnsemble* ensemble, QString filename )
{
    if ( !ensemble || !ensemble->hasEnsembleParameters() || filename.isEmpty() ) return;

    std::map<std::string, std::vector<std::string>> parameters;

    std::set<QString> allNamesAvailable;

    for ( auto c : ensemble->allSummaryCases() )
    {
        if ( c->caseRealizationParameters() ) allNamesAvailable.merge( c->caseRealizationParameters()->parameterNames() );
    }
}
