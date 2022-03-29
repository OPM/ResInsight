////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicCreateMultiPlotFromSelectionFeature.h"

#include "RiaGuiApplication.h"
#include "RiaSummaryTools.h"

#include "RicSummaryPlotTemplateTools.h"

#include "PlotTemplates/RimPlotTemplateFileItem.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateMultiPlotFromSelectionFeature, "RicCreateMultiPlotFromSelectionFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateMultiPlotFromSelectionFeature::isCommandEnabled()
{
    return true;

    /*
        bool anySummaryCases           = !RicSummaryPlotTemplateTools::selectedSummaryCases().empty();
        bool anySummaryCaseCollections = !RicSummaryPlotTemplateTools::selectedSummaryCaseCollections().empty();

        return ( anySummaryCases || anySummaryCaseCollections );
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultiPlotFromSelectionFeature::onActionTriggered( bool isChecked )
{
    QString fileName           = RicSummaryPlotTemplateTools::selectPlotTemplatePath();
    auto    sumCases           = RicSummaryPlotTemplateTools::selectedSummaryCases();
    auto    sumCaseCollections = RicSummaryPlotTemplateTools::selectedSummaryCaseCollections();

    auto summaryAddressCollections = RicSummaryPlotTemplateTools::selectedSummaryAddressCollections();

    bool addSummaryCase = false;
    if ( sumCases.empty() && sumCaseCollections.empty() )
    {
        addSummaryCase = true;
    }

    std::vector<QString>      wellNames;
    std::set<RimSummaryCase*> casesDerivedFromWellName;
    for ( auto a : summaryAddressCollections )
    {
        if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
        {
            wellNames.push_back( a->name() );

            if ( addSummaryCase )
            {
                auto sumCase = RiaSummaryTools::summaryCaseById( a->caseId() );
                if ( sumCase ) casesDerivedFromWellName.insert( sumCase );
            }
        }
    }

    for ( auto s : casesDerivedFromWellName )
    {
        sumCases.push_back( s );
    }

    auto proj        = RimProject::current();
    auto collections = proj->mainPlotCollection()->summaryMultiPlotCollection();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( fileName );
    if ( !newSummaryPlot ) return;

    collections->addSummaryMultiPlot( newSummaryPlot );
    newSummaryPlot->resolveReferencesRecursively();

    RicSummaryPlotTemplateTools::fillPlaceholderValues( newSummaryPlot, sumCases, sumCaseCollections, wellNames );
    newSummaryPlot->initAfterReadRecursively();
    newSummaryPlot->loadDataAndUpdate();
    collections->updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( newSummaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultiPlotFromSelectionFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Summary MultiPlot from Template" );
    actionToSetup->setIcon( QIcon( ":/SummaryTemplate16x16.png" ) );
}
