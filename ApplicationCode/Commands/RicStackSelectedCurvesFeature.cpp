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

#include "RicStackSelectedCurvesFeature.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "cafPdmFieldIOScriptability.h"
#include "cafPdmObject.h"
#include "cafPdmScriptResponse.h"
#include "cafPdmUiItem.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

RICF_SOURCE_INIT( RicStackSelectedCurvesFeature, "RicStackSelectedCurvesFeature", "stackCurves" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicStackSelectedCurvesFeature::RicStackSelectedCurvesFeature()
{
    CAF_PDM_InitScriptableFieldWithIONoDefault( &m_curves, "curves", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*>
    RicStackSelectedCurvesFeature::plotCurvesFromSelection( const std::vector<caf::PdmUiItem*>& selectedItems )
{
    std::vector<RimPlotCurve*> selectedPlotCurves;

    for ( caf::PdmUiItem* uiItem : selectedItems )
    {
        auto plotCurve = dynamic_cast<RimPlotCurve*>( uiItem );
        if ( plotCurve )
        {
            selectedPlotCurves.push_back( plotCurve );
        }
    }

    return selectedPlotCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*>
    RicStackSelectedCurvesFeature::subsetOfPlotCurvesFromStacking( const std::vector<RimPlotCurve*>& plotCurves,
                                                                   bool                              isStacked )
{
    std::vector<RimPlotCurve*> matchingPlotCurves;

    for ( RimPlotCurve* plotCurve : plotCurves )
    {
        if ( plotCurve->isStacked() == isStacked )
        {
            matchingPlotCurves.push_back( plotCurve );
        }
    }

    return matchingPlotCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicStackSelectedCurvesFeature::execute()
{
    if ( m_curves().empty() )
    {
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, "No Curves Provided" );
    }

    for ( auto plotCurve : m_curves() )
    {
        plotCurve->setIsStacked( true );
    }
    return caf::PdmScriptResponse();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicStackSelectedCurvesFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    auto plotCurves = plotCurvesFromSelection( selectedItems );

    if ( plotCurves.size() != selectedItems.size() ) return false;

    auto unstackedPlotCurves = subsetOfPlotCurvesFromStacking( plotCurves, false );

    return !unstackedPlotCurves.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStackSelectedCurvesFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems( selectedItems );

    auto plotCurves          = plotCurvesFromSelection( selectedItems );
    auto unstackedPlotCurves = subsetOfPlotCurvesFromStacking( plotCurves, false );

    m_curves.setValue( unstackedPlotCurves );

    caf::PdmScriptResponse response = execute();

    if ( response.status() != caf::PdmScriptResponse::COMMAND_OK )
    {
        QString displayMessage = response.messages().join( "\n" );
        if ( RiaGuiApplication::isRunning() )
        {
            QMessageBox::warning( nullptr, "Error when saving project file", displayMessage );
        }
        RiaLogging::error( displayMessage );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicStackSelectedCurvesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Stack Selected Curves" );
}
