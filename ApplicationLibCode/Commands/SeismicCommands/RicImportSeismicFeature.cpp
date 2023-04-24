/////////////////////////////////////////////////////////////////////////////////
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

#include "RicImportSeismicFeature.h"

#include "RiaApplication.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicImportSeismicFeature, "RicImportSeismicFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportSeismicFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSeismicFeature::onActionTriggered( bool isChecked )
{
    QString         filter     = "Seismic files (*.zgy *.vds);;SEG-Y files (*.sgy *.segy);;All Files (*.*)";
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "SEISMIC_GRID" );
    QString fileName = RiuFileDialogTools::getOpenFileName( Riu3DMainWindowTools::mainWindowWidget(), "Import Seismic", defaultDir, filter );

    QFileInfo fi( fileName );

    QString ext = fi.suffix().toLower();
    if ( ( ext == "segy" ) || ( ext == "sgy" ) )
    {
        QMessageBox::information( nullptr,
                                  QString( "SEG-Y import" ),
                                  QString( "SEG-Y file %1 will be converted to VDS format." ).arg( fileName ) );
        fileName = convertSEGYtoVDS( fileName );
    }

    if ( fileName.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "SEISMIC_GRID", QFileInfo( fileName ).absolutePath() );

    auto  proj     = RimProject::current();
    auto& seisColl = proj->activeOilField()->seismicCollection();

    if ( !seisColl ) return;

    RimSeismicData* newData = seisColl->importSeismicFromFile( fileName );

    // workaround to make tree selection work, otherwise "Cell Results" gets selected for some reason
    QApplication::processEvents();

    if ( newData )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( newData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportSeismicFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Seismic16x16.png" ) );
    actionToSetup->setText( "Import Seismic" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicImportSeismicFeature::convertSEGYtoVDS( QString filename )
{
    return "";
}
