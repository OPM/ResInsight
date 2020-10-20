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

#include "RicInsertColorLegendFeature.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"

#include "RimProject.h"

#include "cafSelectionManager.h"

#include "Riu3DMainWindowTools.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicInsertColorLegendFeature, "RicInsertColorLegendFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicInsertColorLegendFeature::isCommandEnabled()
{
    if ( selectedColorLegendCollection() ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
/// TODO: insert new color legend prior to entry of selected color legend ?
/// C.f. RicNewAnalysisPlotFeature.cpp(70-71)
//--------------------------------------------------------------------------------------------------
void RicInsertColorLegendFeature::onActionTriggered( bool isChecked )
{
    RimColorLegendCollection* colorLegendCollection = selectedColorLegendCollection();

    if ( colorLegendCollection )
    {
        RimColorLegend* customLegend = new RimColorLegend();
        customLegend->setColorLegendName( "New Color Legend" );

        RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;

        colorLegendCollection->appendCustomColorLegend( customLegend );
        caf::SelectionManager::instance()->setSelectedItem( customLegend );

        colorLegendCollection->updateConnectedEditors();

        Riu3DMainWindowTools::setExpanded( customLegend );
        Riu3DMainWindowTools::selectAsCurrentItem( customLegend );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicInsertColorLegendFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/Legend.png" ) );
    actionToSetup->setText( "New Color Legend" );
}

//--------------------------------------------------------------------------------------------------
/// C.f. RicNewAnalysisPlotFeature.cpp(42)
//--------------------------------------------------------------------------------------------------
RimColorLegendCollection* RicInsertColorLegendFeature::selectedColorLegendCollection()
{
    caf::PdmObject* selectedObject = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );

    if ( !selectedObject ) return nullptr;

    RimColorLegendCollection* colorLegendCollection = nullptr;

    selectedObject->firstAncestorOrThisOfType( colorLegendCollection );
    if ( colorLegendCollection )
    {
        // Disable the menu for standard color legends
        RimColorLegend* colorLegend = dynamic_cast<RimColorLegend*>( selectedObject );
        if ( colorLegend && colorLegendCollection->isStandardColorLegend( colorLegend ) )
        {
            return nullptr;
        }

        return colorLegendCollection;
    }

    return nullptr;
}
