////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

    std::vector<QString>                wellNames;
    std::vector<QString>                wellGroupNames;
    std::vector<QString>                regions;
    std::set<RimSummaryCase*>           caseSet;
    std::set<RimSummaryCaseCollection*> caseCollectionSet;

    for ( auto a : summaryAddressCollections )
    {
        if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL )
        {
            wellNames.push_back( a->name() );
        }
        else if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::WELL_GROUP )
        {
            wellGroupNames.push_back( a->name() );
        }
        else if ( a->contentType() == RimSummaryAddressCollection::CollectionContentType::REGION )
        {
            regions.push_back( a->name() );
        }

        auto sumCase = RiaSummaryTools::summaryCaseById( a->caseId() );
        if ( sumCase ) caseSet.insert( sumCase );

        auto ensemble = RiaSummaryTools::ensembleById( a->ensembleId() );
        if ( ensemble ) caseCollectionSet.insert( ensemble );
    }

    for ( auto sumCase : caseSet )
    {
        sumCases.push_back( sumCase );
    }
    for ( auto sumCaseCollection : caseCollectionSet )
    {
        sumCaseCollections.push_back( sumCaseCollection );
    }

    auto proj        = RimProject::current();
    auto collections = proj->mainPlotCollection()->summaryMultiPlotCollection();

    auto newSummaryPlot = RicSummaryPlotTemplateTools::createMultiPlotFromTemplateFile( fileName );
    if ( !newSummaryPlot ) return;

    collections->addSummaryMultiPlot( newSummaryPlot );
    newSummaryPlot->resolveReferencesRecursively();

    RicSummaryPlotTemplateTools::setValuesForPlaceholders( newSummaryPlot,
                                                           sumCases,
                                                           sumCaseCollections,
                                                           wellNames,
                                                           wellGroupNames,
                                                           regions );
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
