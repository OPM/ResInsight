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

#include "RiaGuiApplication.h"
#include "RiaSummaryTools.h"

#include "RicSavePlotTemplateFeature.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT( RicSavePlotTemplateFeature, "RicSavePlotTemplateFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSavePlotTemplateFeature::RicSavePlotTemplateFeature() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSavePlotTemplateFeature::isCommandEnabled()
{
    if ( selectedSummaryPlot() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSavePlotTemplateFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    QString startPath;
    if ( !app->project()->fileName().isEmpty() )
    {
        startPath = app->project()->fileName();
        startPath = startPath.replace( QString( ".rsp" ), QString( ".rpt" ) );
    }
    else
    {
        startPath = app->lastUsedDialogDirectory( "PLOT_TEMPLATE" );
        startPath += "/ri-plot-template.rpt";
    }

    QString fileName = QFileDialog::getSaveFileName( nullptr,
                                                     tr( "Save Plot Template To File" ),
                                                     startPath,
                                                     tr( "Plot Template Files (*.rpt);;All files(*.*)" ) );
    if ( !fileName.isEmpty() )
    {
        auto objectAsText = selectedSummaryPlot()->writeObjectToXmlString();

        QFile exportFile( fileName );
        if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            RiaLogging::error( QString( "Save Plot Template : Could not open the file: %1" ).arg( fileName ) );
            return;
        }

        QTextStream stream( &exportFile );
        stream << objectAsText;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSavePlotTemplateFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Save As Plot Template" );
    // actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSavePlotTemplateFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return sumPlot;
}
