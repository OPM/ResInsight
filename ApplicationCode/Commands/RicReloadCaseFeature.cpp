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

#include "RicReloadCaseFeature.h"

#include "RiaGuiApplication.h"

#include "RimEclipseCase.h"
#include "RimReloadCaseTools.h"
#include "RimTimeStepFilter.h"

#include "Riu3dSelectionManager.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadCaseFeature, "RicReloadCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicReloadCaseFeature::isCommandEnabled()
{
    std::vector<caf::PdmObject*> selectedFormationNamesCollObjs;
    caf::SelectionManager::instance()->objectsByType( &selectedFormationNamesCollObjs );
    for ( caf::PdmObject* pdmObject : selectedFormationNamesCollObjs )
    {
        if ( dynamic_cast<RimEclipseCase*>( pdmObject ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadCaseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipseCase*> selectedEclipseCases;
    caf::SelectionManager::instance()->objectsByType( &selectedEclipseCases );

    RiaGuiApplication::clearAllSelections();

    for ( RimEclipseCase* selectedCase : selectedEclipseCases )
    {
        std::vector<RimTimeStepFilter*> timeStepFilter;
        selectedCase->descendantsIncludingThisOfType( timeStepFilter );
        if ( timeStepFilter.size() == 1 )
        {
            timeStepFilter[0]->clearFilteredTimeSteps();
        }

        RimReloadCaseTools::reloadAllEclipseData( selectedCase );
        selectedCase->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh-32.png" ) );
}
