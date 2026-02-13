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

#include "RicNewStatisticsCaseFeature.h"

#include "RimCaseCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseStatisticsCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimProject.h"
#include "RimReservoirGridEnsembleBase.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewStatisticsCaseFeature, "RicNewStatisticsCaseFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStatisticsCaseFeature::isCommandEnabled() const
{
    return selectedValidUIItem() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsCaseFeature::onActionTriggered( bool isChecked )
{
    caf::PdmUiItem* uiItem = selectedValidUIItem();
    if ( uiItem )
    {
        RimEclipseStatisticsCase* newCase = addStatisticalCalculation( uiItem );
        if ( newCase )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( newCase );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStatisticsCaseFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Statistics Case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiItem* RicNewStatisticsCaseFeature::selectedValidUIItem()
{
    const auto statisticsCaseCollections = caf::SelectionManager::instance()->objectsByType<RimEclipseStatisticsCaseCollection>();
    if ( !statisticsCaseCollections.empty() )
    {
        return statisticsCaseCollections[0];
    }

    const auto caseCollections = caf::SelectionManager::instance()->objectsByType<RimCaseCollection>();
    if ( !caseCollections.empty() )
    {
        if ( RimIdenticalGridCaseGroup::isStatisticsCaseCollection( caseCollections[0] ) )
        {
            return caseCollections[0];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RicNewStatisticsCaseFeature::addStatisticalCalculation( caf::PdmUiItem* uiItem )
{
    RimCaseCollection* caseCollection = nullptr;

    if ( auto* statCase = dynamic_cast<RimEclipseStatisticsCase*>( uiItem ) )
    {
        caseCollection = statCase->parentStatisticsCaseCollection();
    }
    else if ( auto* caseColl = dynamic_cast<RimCaseCollection*>( uiItem ) )
    {
        caseCollection = caseColl;
    }

    if ( !caseCollection ) return nullptr;

    auto* ensembleBase = caseCollection->parentGridEnsembleBase();
    if ( !ensembleBase ) return nullptr;

    auto* createdObject = ensembleBase->createAndAppendStatisticsCase();
    RimProject::current()->assignCaseIdToCase( createdObject );

    caseCollection->parentField()->ownerObject()->uiCapability()->updateConnectedEditors();

    return createdObject;
}
