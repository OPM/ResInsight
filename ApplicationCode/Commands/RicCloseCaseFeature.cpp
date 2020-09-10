/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicCloseCaseFeature.h"

#include "RimAdvancedSnapshotExportDefinition.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechModels.h"
#include "RimGridSummaryCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellLogPlotCollection.h"

#include "RiuMainWindow.h"

#include "cafPdmFieldHandle.h"
#include "cafPdmObjectGroup.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT( RicCloseCaseFeature, "RicCloseCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::isCommandEnabled()
{
    return !selectedCases().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimEclipseCase*> eclipseCases;
    std::vector<RimGeoMechCase*> geoMechCases;
    for ( RimCase* rimCase : selectedCases() )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
        if ( eclipseCase )
        {
            eclipseCases.push_back( eclipseCase );
        }
        else
        {
            RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( rimCase );
            if ( geoMechCase )
            {
                geoMechCases.push_back( geoMechCase );
            }
        }
    }

    if ( !eclipseCases.empty() )
    {
        if ( userConfirmedGridCaseGroupChange( eclipseCases ) )
        {
            for ( RimEclipseCase* eclipseCase : eclipseCases )
            {
                deleteEclipseCase( eclipseCase );
            }
            RiuMainWindow::instance()->cleanupGuiCaseClose();
        }
    }

    if ( !geoMechCases.empty() )
    {
        for ( RimGeoMechCase* geoMechCase : geoMechCases )
        {
            deleteGeoMechCase( geoMechCase );
        }
        RiuMainWindow::instance()->cleanupGuiCaseClose();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Close" );
    actionToSetup->setIcon( QIcon( ":/Erase.png" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCase*> RicCloseCaseFeature::selectedCases() const
{
    std::vector<RimCase*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );
    return selection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::removeCaseFromAllGroups( RimEclipseCase* eclipseCase )
{
    CVF_ASSERT( eclipseCase );

    RimProject*               proj           = RimProject::current();
    RimOilField*              activeOilField = proj ? proj->activeOilField() : nullptr;
    RimEclipseCaseCollection* analysisModels = ( activeOilField ) ? activeOilField->analysisModels() : nullptr;
    if ( analysisModels )
    {
        analysisModels->removeCaseFromAllGroups( eclipseCase );
        analysisModels->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteEclipseCase( RimEclipseCase* eclipseCase )
{
    CVF_ASSERT( eclipseCase );

    RimCaseCollection* caseCollection = eclipseCase->parentCaseCollection();
    if ( caseCollection )
    {
        if ( RimIdenticalGridCaseGroup::isStatisticsCaseCollection( caseCollection ) )
        {
            RimIdenticalGridCaseGroup* caseGroup = caseCollection->parentCaseGroup();
            CVF_ASSERT( caseGroup );

            caseGroup->statisticsCaseCollection()->reservoirs.removeChildObject( eclipseCase );
            caseGroup->updateConnectedEditors();
        }
        else
        {
            RimIdenticalGridCaseGroup* caseGroup = caseCollection->parentCaseGroup();
            if ( caseGroup )
            {
                // When deleting the last source case for statistics, remove any views on statistics cases.
                // This is done because the views do not work well
                if ( caseGroup->caseCollection()->reservoirs.size() == 1 )
                {
                    std::vector<caf::PdmObjectHandle*> children;
                    caseGroup->statisticsCaseCollection()->reservoirs.childObjects( &children );

                    for ( size_t i = children.size(); i-- > 0; )
                    {
                        caf::PdmObjectHandle* obj = children[i];
                        delete obj;
                        caseGroup->statisticsCaseCollection()->reservoirs.erase( i );
                    }

                    caseGroup->statisticsCaseCollection()->uiCapability()->updateConnectedEditors();
                }
            }
            removeCaseFromAllGroups( eclipseCase );
        }
    }
    else
    {
        removeCaseFromAllGroups( eclipseCase );
    }

    RimEclipseResultCase* resultCase = dynamic_cast<RimEclipseResultCase*>( eclipseCase );
    if ( resultCase )
    {
        RimProject*                   project = RimProject::current();
        RimSummaryCaseMainCollection* sumCaseColl =
            project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
        if ( sumCaseColl )
        {
            RimSummaryCase* summaryCase = sumCaseColl->findSummaryCaseFromEclipseResultCase( resultCase );
            if ( summaryCase )
            {
                RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>( summaryCase );
                sumCaseColl->convertGridSummaryCasesToFileSummaryCases( gridSummaryCase );
            }
        }
    }

    delete eclipseCase;

    {
        RimProject* project = RimProject::current();

        std::vector<RimCase*> cases;
        project->allCases( cases );

        if ( cases.empty() )
        {
            project->multiSnapshotDefinitions.deleteAllChildObjects();
        }
        else
        {
            for ( RimAdvancedSnapshotExportDefinition* msd : project->multiSnapshotDefinitions() )
            {
                msd->additionalCases.removePtr( nullptr );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCloseCaseFeature::deleteGeoMechCase( RimGeoMechCase* geoMechCase )
{
    CVF_ASSERT( geoMechCase );

    RimProject*       proj           = RimProject::current();
    RimOilField*      activeOilField = proj ? proj->activeOilField() : nullptr;
    RimGeoMechModels* models         = ( activeOilField ) ? activeOilField->geoMechModels() : nullptr;
    if ( models )
    {
        models->removeCase( geoMechCase );
        models->updateConnectedEditors();
    }

    delete geoMechCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::hasAnyStatisticsResults( RimIdenticalGridCaseGroup* gridCaseGroup )
{
    CVF_ASSERT( gridCaseGroup );

    for ( size_t i = 0; i < gridCaseGroup->statisticsCaseCollection()->reservoirs().size(); i++ )
    {
        RimEclipseStatisticsCase* rimStaticsCase =
            dynamic_cast<RimEclipseStatisticsCase*>( gridCaseGroup->statisticsCaseCollection()->reservoirs[i] );
        if ( rimStaticsCase )
        {
            if ( rimStaticsCase->hasComputedStatistics() )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCloseCaseFeature::userConfirmedGridCaseGroupChange( const std::vector<RimEclipseCase*>& casesToBeDeleted )
{
    std::vector<RimIdenticalGridCaseGroup*> gridCaseGroups;

    for ( auto caseToDelete : casesToBeDeleted )
    {
        RimIdenticalGridCaseGroup* gridCaseGroup = nullptr;
        caseToDelete->firstAncestorOrThisOfType( gridCaseGroup );

        if ( gridCaseGroup && hasAnyStatisticsResults( gridCaseGroup ) )
        {
            gridCaseGroups.push_back( gridCaseGroup );
        }
    }

    if ( !gridCaseGroups.empty() )
    {
        RiuMainWindow* mainWnd = RiuMainWindow::instance();

        QMessageBox msgBox( mainWnd );
        msgBox.setIcon( QMessageBox::Question );

        QString questionText;
        if ( gridCaseGroups.size() == 1 )
        {
            questionText = QString( "This operation will invalidate statistics results in grid case group\n\"%1\".\n" )
                               .arg( gridCaseGroups[0]->name() );
            questionText += "Computed results in this group will be deleted if you continue.";
        }
        else
        {
            questionText = "This operation will invalidate statistics results in grid case groups\n";
            for ( auto& gridCaseGroup : gridCaseGroups )
            {
                questionText += QString( "\"%1\"\n" ).arg( gridCaseGroup->name() );
            }

            questionText += "Computed results in these groups will be deleted if you continue.";
        }

        msgBox.setText( questionText );
        msgBox.setInformativeText( "Do you want to continue?" );
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

        int ret = msgBox.exec();
        if ( ret == QMessageBox::No )
        {
            return false;
        }
    }

    return true;
}
