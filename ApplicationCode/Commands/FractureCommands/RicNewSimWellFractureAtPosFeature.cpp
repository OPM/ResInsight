/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewSimWellFractureAtPosFeature.h"

#include "RiaApplication.h"
#include "RigEclipseCaseData.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimStimPlanColors.h"

#include "Riu3DMainWindowTools.h"
#include "Riu3dSelectionManager.h"
#include "RivSimWellPipeSourceInfo.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSimWellFractureAtPosFeature, "RicNewSimWellFractureAtPosFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::onActionTriggered( bool isChecked )
{
    RimProject* proj = RiaApplication::instance()->project();
    if ( proj->allFractureTemplates().empty() ) return;

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if ( !activeView ) return;

    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuSimWellSelectionItem* simWellItem = static_cast<RiuSimWellSelectionItem*>( selItem );
    if ( !simWellItem ) return;

    RimSimWellInView* simWell = simWellItem->m_simWell;
    if ( !simWell ) return;

    RimSimWellFractureCollection* fractureCollection = simWell->simwellFractureCollection();
    if ( !fractureCollection ) return;

    RimSimWellFracture* fracture = new RimSimWellFracture();
    if ( fractureCollection->simwellFractures.empty() )
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
        if ( eclipseView )
        {
            eclipseView->fractureColors()->setDefaultResultName();
        }
    }

    fractureCollection->simwellFractures.push_back( fracture );

    fracture->setClosestWellCoord( simWellItem->m_domainCoord, simWellItem->m_branchIndex );

    RimOilField* oilfield = nullptr;
    simWell->firstAncestorOrThisOfType( oilfield );
    if ( !oilfield ) return;

    std::vector<RimFracture*> oldFractures;
    oilfield->descendantsIncludingThisOfType( oldFractures );
    QString fracNum = QString( "%1" ).arg( oldFractures.size(), 2, 10, QChar( '0' ) );

    fracture->setName( QString( "Fracture_" ) + fracNum );

    auto unitSet = RiaEclipseUnitTools::UNITS_UNKNOWN;
    {
        RimEclipseResultCase* eclipseCase = nullptr;
        simWell->firstAncestorOrThisOfType( eclipseCase );
        if ( eclipseCase )
        {
            unitSet = eclipseCase->eclipseCaseData()->unitsType();
        }

        fracture->setFractureUnit( unitSet );
    }

    RimFractureTemplate* fracDef = oilfield->fractureDefinitionCollection()->firstFractureOfUnit( unitSet );
    fracture->setFractureTemplate( fracDef );

    simWell->updateConnectedEditors();

    activeView->scheduleCreateDisplayModelAndRedraw();

    RimEclipseCase* eclipseCase = nullptr;
    simWell->firstAncestorOrThisOfType( eclipseCase );
    if ( eclipseCase )
    {
        proj->reloadCompletionTypeResultsForEclipseCase( eclipseCase );
        proj->updateConnectedEditors();
    }
    Riu3DMainWindowTools::selectAsCurrentItem( fracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureSymbol16x16.png" ) );
    actionToSetup->setText( "Create Fracture" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureAtPosFeature::isCommandEnabled()
{
    RimProject* proj = RiaApplication::instance()->project();
    if ( proj->allFractureTemplates().empty() ) return false;

    auto objHandle = caf::SelectionManager::instance()->selectedItemOfType<caf::PdmObjectHandle>();
    if ( !objHandle ) return false;

    RimSimWellInView* eclipseWell = nullptr;
    objHandle->firstAncestorOrThisOfType( eclipseWell );

    if ( eclipseWell )
    {
        return true;
    }

    return false;
}
