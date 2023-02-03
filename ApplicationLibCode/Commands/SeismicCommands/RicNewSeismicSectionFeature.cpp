/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewSeismicSectionFeature.h"

#include "RimGridView.h"
#include "RimSeismicSection.h"
#include "RimSeismicSectionCollection.h"
#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSeismicSectionFeature, "RicNewSeismicSectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSeismicSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSeismicSectionFeature::onActionTriggered( bool isChecked )
{
    // Find the selected Cell Filter Collection
    std::vector<RimSeismicSectionCollection*> colls = caf::selectedObjectsByTypeStrict<RimSeismicSectionCollection*>();
    if ( colls.empty() ) return;
    RimSeismicSectionCollection* seisColl = colls[0];

    RimSeismicSection* newSection = seisColl->addNewSection();
    if ( newSection )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( newSection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSeismicSectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Seismic16x16.png" ) );
    actionToSetup->setText( "New Seismic Section" );
}
