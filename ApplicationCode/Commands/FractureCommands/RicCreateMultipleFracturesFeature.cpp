/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicCreateMultipleFracturesFeature.h"
#include "RicFractureNameGenerator.h"

#include "RiaApplication.h"

#include "RicCreateMultipleFracturesUi.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"

#include "RiaPorosityModel.h"
#include <QAction>
#include <QPushButton>

CAF_CMD_SOURCE_INIT( RicCreateMultipleFracturesFeature, "RicCreateMultipleFracturesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::appendFractures()
{
    slotAppendFractures();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::replaceFractures()
{
    slotDeleteAndAppendFractures();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RicCreateMultipleFracturesFeature::ijkRangeForGrid( RimEclipseCase* gridCase ) const
{
    cvf::Vec3st minIJK;
    cvf::Vec3st maxIJK;
    if ( gridCase && gridCase->eclipseCaseData() )
    {
        gridCase->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL )->IJKBoundingBox( minIJK, maxIJK );
        return std::make_pair( minIJK, maxIJK );
    }
    return std::make_pair( cvf::Vec3st(), cvf::Vec3st() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotDeleteAndAppendFractures()
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if ( !multipleFractionsUi ) return;

    auto items = multipleFractionsUi->locationsForNewFractures();
    for ( auto item : items )
    {
        if ( item.wellPath )
        {
            RimWellPathFractureCollection* fractureCollection = item.wellPath->fractureCollection();
            fractureCollection->deleteFractures();
        }
    }

    slotAppendFractures();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotAppendFractures()
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if ( !multipleFractionsUi ) return;

    auto items = multipleFractionsUi->locationsForNewFractures();
    for ( auto item : items )
    {
        if ( item.wellPath )
        {
            RimWellPathFractureCollection* fractureCollection = item.wellPath->fractureCollection();

            // If this is the first fracture, set default result name
            if ( fractureCollection->allFractures().empty() )
            {
                RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(
                    RiaApplication::instance()->activeReservoirView() );
                if ( activeView )
                {
                    activeView->fractureColors()->setDefaultResultName();
                }
            }

            RimWellPathFracture* fracture = new RimWellPathFracture();
            fractureCollection->addFracture( fracture );

            fracture->setFractureUnit( item.wellPath->unitSystem() );
            fracture->setMeasuredDepth( item.measuredDepth );
            fracture->setFractureTemplate( item.fractureTemplate );

            QString fractureName = RicFractureNameGenerator::nameForNewFracture();
            if ( item.fractureTemplate )
            {
                fractureName = QString( "%1_%2" ).arg( item.fractureTemplate->name() ).arg( item.measuredDepth );
            }

            fracture->setName( fractureName );
        }
    }

    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    proj->updateConnectedEditors();
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotClose()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::onActionTriggered( bool isChecked )
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if ( multipleFractionsUi )
    {
        m_copyOfObject = multipleFractionsUi->writeObjectToXmlString();

        if ( multipleFractionsUi->options().empty() )
        {
            RiaApplication* app  = RiaApplication::instance();
            RimProject*     proj = app->project();

            RimEclipseCase* firstSourceCase = nullptr;
            if ( !proj->eclipseCases().empty() )
            {
                firstSourceCase = proj->eclipseCases().front();

                auto ijkRange = ijkRangeForGrid( firstSourceCase );
                int  topK     = static_cast<int>( ijkRange.first.z() );
                int  baseK    = static_cast<int>( ijkRange.second.z() );

                double minimumDistanceFromTip = 100.0;
                int    maxFractureCount       = 100;
                multipleFractionsUi->setValues( firstSourceCase, minimumDistanceFromTip, maxFractureCount );

                // Options
                auto newItem = new RicCreateMultipleFracturesOptionItemUi;

                RimFractureTemplate* firstFractureTemplate = nullptr;
                if ( !proj->allFractureTemplates().empty() )
                {
                    firstFractureTemplate = proj->allFractureTemplates().front();
                }

                double minimumSpacing = 300.0;
                newItem->setValues( topK + 1, baseK + 1, firstFractureTemplate, minimumSpacing );

                multipleFractionsUi->insertOptionItem( nullptr, newItem );
            }
        }

        // Selected well paths
        std::vector<RimWellPath*> selWells = caf::selectedObjectsByTypeStrict<RimWellPath*>();
        multipleFractionsUi->clearWellPaths();
        for ( auto wellPath : selWells )
            multipleFractionsUi->addWellPath( wellPath );

        caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                                     multipleFractionsUi,
                                                     "Create Multiple Fractures",
                                                     "" );

        multipleFractionsUi->setParentDialog( &propertyDialog );

        propertyDialog.resize( QSize( 700, 450 ) );

        QDialogButtonBox* dialogButtonBox = propertyDialog.dialogButtonBox();

        dialogButtonBox->clear();

        {
            QPushButton* pushButton = dialogButtonBox->addButton( RiuCreateMultipleFractionsUi::REPLACE_FRACTURES_BUTTON_TEXT,
                                                                  QDialogButtonBox::ActionRole );
            connect( pushButton, SIGNAL( clicked() ), this, SLOT( slotDeleteAndAppendFractures() ) );
            pushButton->setDefault( false );
            pushButton->setAutoDefault( false );
            pushButton->setToolTip( "Delete all existing fractures before adding new fractures" );
        }

        {
            QPushButton* pushButton = dialogButtonBox->addButton( RiuCreateMultipleFractionsUi::ADD_FRACTURES_BUTTON_TEXT,
                                                                  QDialogButtonBox::ActionRole );
            connect( pushButton, SIGNAL( clicked() ), this, SLOT( slotAppendFractures() ) );
            pushButton->setDefault( false );
            pushButton->setAutoDefault( false );
            pushButton->setToolTip( "Add new fractures" );
        }

        {
            QPushButton* pushButton = dialogButtonBox->addButton( "Close", QDialogButtonBox::ActionRole );
            connect( pushButton, SIGNAL( clicked() ), &propertyDialog, SLOT( close() ) );
            pushButton->setDefault( false );
            pushButton->setAutoDefault( false );
        }

        propertyDialog.exec();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FractureTemplate16x16.png" ) );
    actionToSetup->setText( "Create Multiple Fractures" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateMultipleFracturesFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> selWells = caf::selectedObjectsByTypeStrict<RimWellPath*>();
    return !selWells.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCreateMultipleFractionsUi* RicCreateMultipleFracturesFeature::multipleFractionsUi() const
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    return proj->dialogData()->multipleFractionsData();
}
