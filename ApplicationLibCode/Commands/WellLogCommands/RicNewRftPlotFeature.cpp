/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RicNewRftPlotFeature.h"

#include "RicCreateRftPlotsFeature.h"

#include "RimMainPlotCollection.h"
#include "RimRftPlotCollection.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewRftPlotFeature, "RicNewRftPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRftPlotFeature::isCommandEnabled() const
{
    auto* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimRftPlotCollection>();
    if ( simWell ) return true;

    if ( selectedWellName().isEmpty() )
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::onActionTriggered( bool isChecked )
{
    RimRftPlotCollection* rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();
    if ( rftPlotColl )
    {
        QString wellName = selectedWellName();

        RicCreateRftPlotsFeature::appendRftPlotForWell( wellName, rftPlotColl );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New RFT Plot" );
    actionToSetup->setIcon( QIcon( ":/FlowCharPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicNewRftPlotFeature::selectedWellName()
{
    auto* simWell = caf::firstAncestorOfTypeFromSelectedObject<RimSimWellInView>();
    if ( simWell ) return simWell->name();

    auto* rimWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath>();
    if ( rimWellPath ) return rimWellPath->name();

    return {};
}
