/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicImportInputEclipseCaseFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicImportInputEclipseCaseFeature, "RicImportInputEclipseCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportInputEclipseCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseFeature::onActionTriggered( bool isChecked )
{
    RicImportGeneralDataFeature::openFileDialog( RiaDefines::ECLIPSE_INPUT_FILE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/EclipseInput48x48.png" ) );
    actionToSetup->setText( "Import Input Eclipse Case" );
}
