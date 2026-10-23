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

#include <QFile>
#include <QMessageBox>
#include <QTextStream>

CAF_CMD_SOURCE_INIT( RicExportEnsembleParametersFeature, "RicExportEnsembleParametersFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEnsembleParametersFeature::onActionTriggered( bool isChecked )
{
    auto    parent = RiaGuiApplication::widgetToUseAsParent();
    QString dlgHeader( "Export Ensemble Parameters" );

    auto ensemble = dynamic_cast<RimSummaryEnsemble*>( caf::SelectionManager::instance()->selectedItem() );
    if ( ensemble == nullptr ) return;

    if ( !ensemble->hasEnsembleParameters() )
    {
        QMessageBox::information( parent, dlgHeader, "No parameters found in the selected ensemble!" );
        return;
    }

    QString filePath = RiuFileDialogTools::getSaveFileName( parent, dlgHeader, "", "CSV files (*.csv);;All files(*.*)" );
    if ( filePath.isEmpty() ) return;

    if ( RicExportEnsembleParametersFeature::exportParameters( ensemble, filePath ) )
    {
        QMessageBox::information( parent, dlgHeader, QString( "Parameters have been exported to file\n%1!\n" ).arg( filePath ) );
    }
    else
    {
        QMessageBox::critical( parent, dlgHeader, QString( "Export failed trying to write to file\n%1!\n" ).arg( filePath ) );
    }
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
bool RicExportEnsembleParametersFeature::exportParameters( RimSummaryEnsemble* ensemble, QString filename )
{
    if ( !ensemble || !ensemble->hasEnsembleParameters() || filename.isEmpty() ) return false;

    const QString realizationKey( "Realization" );
    const QString pathKey( "Path" );

    std::vector<QString> columnKeys;

    // first two columns should be realization number and path to the realization
    columnKeys.push_back( realizationKey );
    columnKeys.push_back( pathKey );

    // get the union of all parameter names
    std::set<QString> allNamesAvailable;
    for ( auto c : ensemble->allSummaryCases() )
    {
        if ( c->caseRealizationParameters() ) allNamesAvailable.merge( c->caseRealizationParameters()->parameterNames() );
    }

    // build our list of column keys, appending at end of list
    std::copy( allNamesAvailable.begin(), allNamesAvailable.end(), std::back_inserter( columnKeys ) );

    std::vector<QStringList> exportData;

    QStringList headerRow;
    for ( auto& key : columnKeys )
    {
        headerRow.append( key );
    }
    exportData.push_back( headerRow );

    for ( auto c : ensemble->allSummaryCases() )
    {
        // skip realizations missing all parameters
        auto params = c->caseRealizationParameters();
        if ( !params ) continue;

        QStringList row;
        for ( auto& key : columnKeys )
        {
            if ( key == realizationKey )
            {
                row.push_back( QString::number( params->realizationNumber() ) );
            }
            else if ( key == pathKey )
            {
                row.push_back( c->summaryHeaderFilename() );
            }
            else
            {
                // values not found in this case will be returned as invalid and get an empty string in valueAsString
                auto value = params->parameterValue( key );
                row.push_back( value.valueAsString() );
            }
        }

        exportData.push_back( row );
    }

    // write to file
    QFile outputFile( filename );
    if ( outputFile.open( QFile::WriteOnly | QFile::Text ) )
    {
        QTextStream output( &outputFile );

        for ( auto& row : exportData )
        {
            QString text = row.join( "," );
            output << text + "\n";
        }

        outputFile.close();
        return true;
    }

    return false;
}
