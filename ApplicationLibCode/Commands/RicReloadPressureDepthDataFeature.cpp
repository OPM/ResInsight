/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RicReloadPressureDepthDataFeature.h"

#include "RimPressureDepthData.h"
#include "RimViewWindow.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicReloadPressureDepthDataFeature, "RicReloadPressureDepthDataFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPressureDepthDataFeature::onActionTriggered( bool isChecked )
{
    const auto pressureDepthDataObjects = caf::SelectionManager::instance()->objectsByType<RimPressureDepthData>();
    for ( RimPressureDepthData* pressureDepthData : pressureDepthDataObjects )
    {
        pressureDepthData->createRftReaderInterface();

        std::vector<caf::PdmObjectHandle*> referringObjects = pressureDepthData->objectsWithReferringPtrFields();
        for ( auto refObj : referringObjects )
        {
            if ( refObj )
            {
                RimViewWindow* viewWindow = refObj->firstAncestorOrThisOfType<RimViewWindow>();
                if ( viewWindow )
                {
                    viewWindow->loadDataAndUpdate();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicReloadPressureDepthDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Reload" );
    actionToSetup->setIcon( QIcon( ":/Refresh.svg" ) );
}
