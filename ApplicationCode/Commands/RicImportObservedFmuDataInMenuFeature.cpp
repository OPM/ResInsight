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

#include "RicImportObservedFmuDataInMenuFeature.h"

#include "RiaApplication.h"

#include "RicImportObservedFmuDataFeature.h"

#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryObservedDataFile.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportObservedFmuDataInMenuFeature, "RicImportObservedFmuDataInMenuFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicImportObservedFmuDataInMenuFeature::RicImportObservedFmuDataInMenuFeature()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportObservedFmuDataInMenuFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataInMenuFeature::onActionTriggered( bool isChecked )
{
    RicImportObservedFmuDataFeature::selectObservedDataPathInDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataInMenuFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/ObservedDataFile16x16.png" ) );
    actionToSetup->setText( "Import Observed FMU Data" );
}
