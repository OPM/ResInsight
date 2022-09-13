/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022    Equinor ASA
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

#include "RicOpenSummaryPlotEditorFeature.h"

#include "RiaSummaryTools.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicNewSummaryEnsembleCurveSetFeature.h"
#include "RicSummaryPlotEditorDialog.h"
#include "RicSummaryPlotEditorUi.h"
#include "RicSummaryPlotFeatureImpl.h"

#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimObservedDataCollection.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicOpenSummaryPlotEditorFeature, "RicOpenSummaryPlotEditorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicOpenSummaryPlotEditorFeature::isCommandEnabled()
{
    RimSummaryMultiPlot*                  multiPlot               = nullptr;
    RimCustomObjectiveFunctionCollection* customObjFuncCollection = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( !selObj ) return false;

    selObj->firstAncestorOrThisOfType( customObjFuncCollection );

    auto ensembleFilter     = dynamic_cast<RimEnsembleCurveFilter*>( selObj );
    auto ensembleFilterColl = dynamic_cast<RimEnsembleCurveFilterCollection*>( selObj );
    auto legendConfig       = dynamic_cast<RimRegularLegendConfig*>( selObj );
    auto sumPlot            = dynamic_cast<RimSummaryPlot*>( selObj );

    if ( ensembleFilter || ensembleFilterColl || legendConfig || customObjFuncCollection || sumPlot ) return false;

    multiPlot = RiaSummaryTools::parentSummaryMultiPlot( selObj );
    if ( multiPlot ) return true;

    auto summaryCase     = dynamic_cast<RimSummaryCase*>( selObj );
    auto summaryCaseColl = dynamic_cast<RimSummaryCaseCollection*>( selObj );
    auto obsColl         = dynamic_cast<RimObservedDataCollection*>( selObj );

    if ( summaryCase || summaryCaseColl || obsColl ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicOpenSummaryPlotEditorFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimSummaryMultiPlot* multiPlot =
        dynamic_cast<RimSummaryMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );

    std::vector<RimSummaryCase*>           selectedCases  = caf::selectedObjectsByType<RimSummaryCase*>();
    std::vector<RimSummaryCaseCollection*> selectedGroups = caf::selectedObjectsByType<RimSummaryCaseCollection*>();

    std::vector<caf::PdmObject*> sourcesToSelect( selectedCases.begin(), selectedCases.end() );

    if ( sourcesToSelect.empty() && selectedGroups.empty() )
    {
        const auto allSingleCases = project->firstSummaryCaseMainCollection()->topLevelSummaryCases();
        const auto allGroups      = project->summaryGroups();
        std::vector<RimSummaryCaseCollection*> allEnsembles;
        for ( const auto group : allGroups )
            if ( group->isEnsemble() ) allEnsembles.push_back( group );

        if ( !allSingleCases.empty() )
        {
            sourcesToSelect.push_back( allSingleCases.front() );
        }
        else if ( !allEnsembles.empty() )
        {
            sourcesToSelect.push_back( allEnsembles.front() );
        }
    }

    // Append grouped cases
    for ( auto group : selectedGroups )
    {
        if ( group->isEnsemble() )
        {
            sourcesToSelect.push_back( group );
        }
        else
        {
            auto groupCases = group->allSummaryCases();
            sourcesToSelect.insert( sourcesToSelect.end(), groupCases.begin(), groupCases.end() );
        }
    }

    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog( true );

    if ( !dialog->isVisible() )
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    if ( multiPlot )
    {
        if ( multiPlot->summaryPlots().size() > 0 )
        {
            dialog->updateFromSummaryPlot( multiPlot->summaryPlots()[0] );
        }
        else
        {
            dialog->updateFromSummaryMultiPlot( multiPlot );
        }
    }
    else
    {
        dialog->updateFromDefaultCases( sourcesToSelect );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicOpenSummaryPlotEditorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Open Summary Plot Editor" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
