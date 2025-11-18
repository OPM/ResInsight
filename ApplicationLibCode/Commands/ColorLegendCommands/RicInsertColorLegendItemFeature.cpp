/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicInsertColorLegendItemFeature.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicInsertColorLegendItemFeature, "RicInsertColorLegendItemFeature" );

//--------------------------------------------------------------------------------------------------
/// Disallows insert of color legend item in standard color legends
//--------------------------------------------------------------------------------------------------
bool RicInsertColorLegendItemFeature::isCommandEnabled() const
{
    RimColorLegend* legend = selectedColorLegend();
    if ( !legend ) return false;

    RimColorLegendCollection* colorLegendCollection = legend->firstAncestorOrThisOfType<RimColorLegendCollection>();
    if ( !colorLegendCollection ) return true;

    return !colorLegendCollection->isStandardColorLegend( legend );
}

//--------------------------------------------------------------------------------------------------
/// TODO: insert new color legend item prior to selected color legend item?
/// C.f. RicNewAnalysisPlotFeature.cpp(70-71)
//--------------------------------------------------------------------------------------------------
void RicInsertColorLegendItemFeature::onActionTriggered( bool isChecked )
{
    RimColorLegend* legend = selectedColorLegend();

    if ( legend )
    {
        RimColorLegendItem* newLegendItem = new RimColorLegendItem();

        legend->appendColorLegendItem( newLegendItem );
        legend->updateConnectedEditors();

        caf::SelectionManager::instance()->setSelectedItem( newLegendItem );

        legend->updateConnectedEditors();

        Riu3DMainWindowTools::setExpanded( newLegendItem );
        Riu3DMainWindowTools::selectAsCurrentItem( newLegendItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicInsertColorLegendItemFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Legend.png" ) );
    actionToSetup->setText( "Append Color" );
}

//--------------------------------------------------------------------------------------------------
/// C.f. RicNewAnalysisPlotFeature.cpp(42)
//--------------------------------------------------------------------------------------------------
RimColorLegend* RicInsertColorLegendItemFeature::selectedColorLegend()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimColorLegend>();
}
