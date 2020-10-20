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

#include "RicUnstackSelectedCurvesFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmScriptResponse.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>

RICF_SOURCE_INIT( RicUnstackSelectedCurvesFeature, "RicUnstackSelectedCurvesFeature", "unstackCurves" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicUnstackSelectedCurvesFeature::RicUnstackSelectedCurvesFeature()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_curves, "curves", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicUnstackSelectedCurvesFeature::execute()
{
    if ( m_curves().empty() )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "No Curves Provided" );
    }

    for ( auto plotCurve : m_curves() )
    {
        plotCurve->setIsStacked( false );
    }
    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUnstackSelectedCurvesFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    auto plotCurves = RicStackSelectedCurvesFeature::plotCurvesFromSelection( selectedItems );

    if ( plotCurves.size() != selectedItems.size() ) return false;

    auto stackedPlotCurves = RicStackSelectedCurvesFeature::subsetOfPlotCurvesFromStacking( plotCurves, true );

    return !stackedPlotCurves.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnstackSelectedCurvesFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    auto plotCurves          = RicStackSelectedCurvesFeature::plotCurvesFromSelection( selectedItems );
    auto unstackedPlotCurves = RicStackSelectedCurvesFeature::subsetOfPlotCurvesFromStacking( plotCurves, true );

    m_curves.setValue( unstackedPlotCurves );

    caf::PdmScriptResponse response = execute();

    if ( response.status() != caf::PdmScriptResponse::COMMAND_OK )
    {
        QString displayMessage = response.messages().join( "\n" );
        RiaLogging::errorInMessageBox( nullptr, "Error when saving project file", displayMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUnstackSelectedCurvesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Unstack Selected Curves" );
}
