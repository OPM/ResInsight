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

#include "RicNewSeismicDifferenceFeature.h"

#include "RiaGuiApplication.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimSeismicData.h"
#include "RimSeismicDataCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicNewSeismicDifferenceFeature, "RicNewSeismicDifferenceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSeismicDifferenceFeature::isCommandEnabled() const
{
    auto size = selectedSeismic().size();
    return size == 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSeismicDifferenceFeature::onActionTriggered( bool isChecked )
{
    auto  proj     = RimProject::current();
    auto& seisColl = proj->activeOilField()->seismicDataCollection();
    if ( !seisColl ) return;

    auto seismicInput = selectedSeismic();
    if ( seismicInput.size() != 2 )
    {
        QString warning = "The selected seismic data grids do not match. Cannot create seismic difference data.";
        QMessageBox::warning( nullptr, "Invalid input.", warning );
        return;
    }

    RimSeismicDataInterface* newData = seisColl->createDifferenceSeismicData( seismicInput[0], seismicInput[1] );

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
void RicNewSeismicDifferenceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Seismic16x16.png" ) );
    actionToSetup->setText( "Create Seismic Difference" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSeismicData*> RicNewSeismicDifferenceFeature::selectedSeismic()
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();
    if ( selectedItems.size() != 2 ) return {};

    std::vector<RimSeismicData*> seismicItems;

    RimSeismicData* seismic1 = dynamic_cast<RimSeismicData*>( selectedItems[0] );
    RimSeismicData* seismic2 = dynamic_cast<RimSeismicData*>( selectedItems[1] );

    if ( seismic1 == nullptr ) return {};
    if ( seismic2 == nullptr ) return {};

    if ( !seismic1->gridIsEqual( seismic2 ) ) return {};

    seismicItems.push_back( seismic1 );
    seismicItems.push_back( seismic2 );
    return seismicItems;
}
