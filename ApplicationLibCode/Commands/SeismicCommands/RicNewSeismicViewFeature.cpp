/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RicNewSeismicViewFeature.h"

#include "RiaApplication.h"
#include "RiaSeismicDefines.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"
#include "RimSeismicView.h"
#include "RimSeismicViewCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicNewSeismicViewFeature, "RicNewSeismicViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSeismicViewFeature::isCommandEnabled() const
{
    auto  proj     = RimProject::current();
    auto& seisColl = proj->activeOilField()->seismicDataCollection();
    return ( seisColl && !seisColl->isEmpty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSeismicViewFeature::onActionTriggered( bool isChecked )
{
    auto  proj         = RimProject::current();
    auto& seisViewColl = proj->activeOilField()->seismicViewCollection();
    auto& seisDataColl = proj->activeOilField()->seismicDataCollection();
    if ( !seisViewColl || !seisDataColl ) return;

    std::vector<caf::PdmUiItem*> uiItems;
    caf::SelectionManager::instance()->selectedItems( uiItems );

    RimSeismicData* selectedData = nullptr;

    if ( uiItems.size() > 0 )
    {
        selectedData = dynamic_cast<RimSeismicData*>( uiItems[0] );
    }

    if ( ( selectedData == nullptr ) && !seisDataColl->isEmpty() )
    {
        selectedData = seisDataColl->seismicData()[0];
    }

    if ( selectedData )
    {
        auto view = seisViewColl->addView( selectedData, RiaDefines::SeismicSectionType::SS_INLINE );

        if ( view )
        {
            seisViewColl->updateAllRequiredEditors();
            view->scheduleCreateDisplayModelAndRedraw();
            // view->defaultView view->zoomAll();

            Riu3DMainWindowTools::selectAsCurrentItem( view );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSeismicViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/SeismicView16x16.png" ) );
    actionToSetup->setText( "New Seismic View" );
}
