/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicExportCarfin.h"

#include "RiaLogging.h"

#include "RicCellRangeUi.h"
#include "RicExportCarfinUi.h"
#include "RicExportFeatureImpl.h"

#include "RifTextDataTableFormatter.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFile>

CAF_CMD_SOURCE_INIT( RicExportCarfin, "RicExportCarfin" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCarfin::isCommandEnabled()
{
    if ( RicExportCarfin::selectedCase() != nullptr )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCarfin::onActionTriggered( bool isChecked )
{
    RimEclipseCase* rimCase = RicExportCarfin::selectedCase();
    CVF_ASSERT( rimCase );

    QString exportCarfinDataAsString = RimProject::current()->dialogData()->exportCarfinDataAsString();

    RicExportCarfinUi* exportCarfinObject = RimProject::current()->dialogData()->exportCarfin();

    exportCarfinObject->setCase( rimCase );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, exportCarfinObject, "Export CARFIN to Eclipse Data", "" );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        QString filePath = exportCarfinObject->exportFileName();
        QFile   exportFile( filePath );
        if ( !exportFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            RiaLogging::error( QString( "Export CARFIN: Could not open the file: %1" ).arg( filePath ) );
            return;
        }

        QTextStream               stream( &exportFile );
        RifTextDataTableFormatter formatter( stream );

        std::vector<RifTextDataTableColumn> header = { RifTextDataTableColumn( "I1" ),
                                                       RifTextDataTableColumn( "I2" ),
                                                       RifTextDataTableColumn( "J1" ),
                                                       RifTextDataTableColumn( "J2" ),
                                                       RifTextDataTableColumn( "K1" ),
                                                       RifTextDataTableColumn( "K2" ),
                                                       RifTextDataTableColumn( "NX" ),
                                                       RifTextDataTableColumn( "NY" ),
                                                       RifTextDataTableColumn( "NZ" ),
                                                       RifTextDataTableColumn( "NWMAX" ),
                                                       RifTextDataTableColumn( "Parent LGR" ) };

        formatter.keyword( "CARFIN" );
        formatter.header( header );

        formatter.add( exportCarfinObject->cellRange()->start().i() );
        formatter.add( exportCarfinObject->cellRange()->start().i() + exportCarfinObject->cellRange()->count().i() );

        formatter.add( exportCarfinObject->cellRange()->start().j() );
        formatter.add( exportCarfinObject->cellRange()->start().j() + exportCarfinObject->cellRange()->count().j() );

        formatter.add( exportCarfinObject->cellRange()->start().k() );
        formatter.add( exportCarfinObject->cellRange()->start().k() + exportCarfinObject->cellRange()->count().k() );

        formatter.add( exportCarfinObject->lgrCellCount().i() );
        formatter.add( exportCarfinObject->lgrCellCount().j() );
        formatter.add( exportCarfinObject->lgrCellCount().k() );

        formatter.add( exportCarfinObject->maxWellCount() );

        if ( !exportCarfinObject->gridName().isEmpty() )
        {
            formatter.add( exportCarfinObject->gridName() );
        }

        formatter.rowCompleted();
        formatter.tableCompleted();
    }
    else
    {
        RimProject::current()->dialogData()->setExportCarfinDataFromString( exportCarfinDataAsString );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCarfin::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export CARFIN ..." );
    actionToSetup->setIcon( QIcon( ":/Save.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicExportCarfin::selectedCase()
{
    std::vector<RimEclipseCase*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );

    if ( selectedObjects.size() == 1 )
    {
        return selectedObjects[0];
    }

    return nullptr;
}
