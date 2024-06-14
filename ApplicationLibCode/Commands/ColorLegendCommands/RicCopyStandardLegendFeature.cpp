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

#include "RicCopyStandardLegendFeature.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimProject.h"

#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCopyStandardLegendFeature, "RicCopyStandardLegendFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCopyStandardLegendFeature::isCommandEnabled() const
{
    return selectedColorLegend() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyStandardLegendFeature::onActionTriggered( bool isChecked )
{
    RimColorLegend* standardLegend = selectedColorLegend();

    if ( standardLegend )
    {
        // perform deep copy of standard legend object via XML
        auto customLegend = standardLegend->copyObject<RimColorLegend>();
        customLegend->setColorLegendName( "Copy of " + standardLegend->colorLegendName() );

        RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;

        colorLegendCollection->appendCustomColorLegend( customLegend );
        colorLegendCollection->updateConnectedEditors();

        Riu3DMainWindowTools::setExpanded( customLegend );
        Riu3DMainWindowTools::selectAsCurrentItem( customLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCopyStandardLegendFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Legend.png" ) );
    actionToSetup->setText( "Copy to Custom Color Legends" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RicCopyStandardLegendFeature::selectedColorLegend()
{
    return caf::firstAncestorOfTypeFromSelectedObject<RimColorLegend>();
}
