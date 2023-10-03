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

#include "RicCreateCrossPlotFeature.h"

#include "RiaPreferencesSummary.h"

#include "RicPasteSummaryCrossPlotFeature.h"

#include "RimSummaryCrossPlot.h"
#include "RimSummaryCrossPlotCollection.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "RifEclipseSummaryAddress.h"
#include <QAction>
#include <QMenu>

CAF_CMD_SOURCE_INIT( RicCreateCrossPlotFeature, "RicCreateCrossPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateCrossPlotFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::onActionTriggered( bool isChecked )
{
    /*
        std::vector<RimSummaryCrossPlot*> selectedObjects = caf::selectedObjectsByType<RimSummaryCrossPlot*>();

        if ( selectedObjects.size() == 1 )
        {
            RicPasteSummaryCrossPlotFeature::copyPlotAndAddToCollection( selectedObjects[0] );
        }
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Summary Cross Plot" );
    // actionToSetup->setIcon( QIcon( ":/SummaryXPlotLight16x16.png" ) );

    auto myMenu = actionToSetup->menu();

    QMenu* submenu = new QMenu( "Create Cross Plot" );

    auto text = RiaPreferencesSummary::current()->crossPlotAddressCombinations();

    auto textList = text.split( ";" );
    for ( const auto& s : textList )
    {
        auto action = submenu->addAction( s );
        connect( action, &QAction::triggered, this, &RicCreateCrossPlotFeature::onActionTriggered );
    }

    actionToSetup->setMenu( submenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateCrossPlotFeature::onSubMenuActionTriggered( bool isChecked )
{
    QString address1;
    QString address2;

    QAction* action = qobject_cast<QAction*>( sender() );

    if ( action )
    {
        QString text         = action->text();
        auto    addressTexts = text.split( " " );

        if ( !addressTexts.empty() ) address1 = addressTexts[0];
        if ( addressTexts.size() > 1 ) address2 = addressTexts[1];
    }

    RifEclipseSummaryAddress adr1 = RifEclipseSummaryAddress::fromEclipseTextAddress( address1.toStdString() );
    RifEclipseSummaryAddress adr2 = RifEclipseSummaryAddress::fromEclipseTextAddress( address2.toStdString() );

    auto newPlot = RicSummaryPlotBuilder::createPlot( eclipseAddresses, selectedCases, selectedEnsembles );
    RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( newPlot );
}
