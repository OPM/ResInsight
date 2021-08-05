/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RicCreateMultipleWellPathLaterals.h"

#include "RicNewWellPathLateralAtDepthFeature.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"
#include "RimWellPathTieIn.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>

#include <memory>

CAF_CMD_SOURCE_INIT( RicCreateMultipleWellPathLaterals, "RicCreateMultipleWellPathLaterals" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateMultipleWellPathLaterals::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLaterals::onActionTriggered( bool isChecked )
{
    m_ui = std::make_unique<RicCreateMultipleWellPathLateralsUi>();

    if ( m_ui.get() == nullptr ) m_ui = std::make_unique<RicCreateMultipleWellPathLateralsUi>();

    auto selected = dynamic_cast<RimModeledWellPath*>( caf::SelectionManager::instance()->selectedItem() );

    if ( selected )
    {
        m_ui->setTopLevelWellPath( selected->topLevelWellPath() );

        double startMD = 0.0;
        double endMD   = 0.0;

        auto sourceLateral = m_ui->sourceLateral();
        if ( sourceLateral )
        {
            if ( auto tieIn = sourceLateral->wellPathTieIn() )
            {
                startMD = sourceLateral->wellPathTieIn()->tieInMeasuredDepth() + 50.0;
                endMD   = startMD + 50.0;

                if ( auto parentWell = sourceLateral->wellPathTieIn()->parentWell() )
                {
                    if ( !parentWell->wellPathGeometry()->measuredDepths().empty() )
                    {
                        double candidate = parentWell->wellPathGeometry()->measuredDepths().back() - 50.0;

                        if ( candidate > startMD ) endMD = candidate;
                    }
                }
            }
        }

        m_ui->setDefaultValues( startMD, endMD );
    }

    {
        caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                                     m_ui.get(),
                                                     "Create Multiple Well Path Laterals",
                                                     "" );

        propertyDialog.resize( QSize( 700, 450 ) );

        QDialogButtonBox* dialogButtonBox = propertyDialog.dialogButtonBox();

        dialogButtonBox->clear();

        {
            QPushButton* pushButton = dialogButtonBox->addButton( "Create Laterals", QDialogButtonBox::ActionRole );
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
void RicCreateMultipleWellPathLaterals::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Multiple Well Path Laterals" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLaterals::slotAppendFractures()
{
    RimModeledWellPath* sourceLateral = m_ui->sourceLateral();

    if ( !sourceLateral ) return;

    auto parentWellPath = sourceLateral->wellPathTieIn()->parentWell();
    if ( !parentWellPath ) return;

    auto sourceLocationOfFirstWellTarget = sourceLateral->geometryDefinition()->firstActiveTarget()->targetPointXYZ();
    auto sourceTieInMeasuredDepth        = sourceLateral->wellPathTieIn()->tieInMeasuredDepth();

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
    if ( wellPathCollection )
    {
        int index = 0;
        for ( auto measuredDepth : m_ui->locationConfig()->locations() )
        {
            RimModeledWellPath* newModeledWellPath = dynamic_cast<RimModeledWellPath*>(
                sourceLateral->xmlCapability()->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

            auto nameOfNewWell =
                RicNewWellPathLateralAtDepthFeature::updateNameOfParentAndFindNameOfSideStep( parentWellPath );
            newModeledWellPath->setName( nameOfNewWell );

            newModeledWellPath->wellPathTieIn()->setTieInMeasuredDepth( measuredDepth );

            wellPathCollection->addWellPath( newModeledWellPath, false );
            newModeledWellPath->resolveReferencesRecursively();

            newModeledWellPath->updateReferencePoint();

            updateLocationOfTargets( newModeledWellPath, sourceLocationOfFirstWellTarget );
            updateLocationOfCompletions( newModeledWellPath, sourceTieInMeasuredDepth );

            newModeledWellPath->updateWellPathVisualization();
        }

        wellPathCollection->rebuildWellPathNodes();
        wellPathCollection->uiCapability()->updateConnectedEditors();

        Riu3DMainWindowTools::selectAsCurrentItem( sourceLateral );

        RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();

        m_ui->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLaterals::updateLocationOfTargets( RimModeledWellPath* newModeledWellPath,
                                                                 const cvf::Vec3d&   sourceLocationOfFirstWellTarget )
{
    newModeledWellPath->updateTieInLocationFromParentWell();
    newModeledWellPath->wellPathTieIn()->updateFirstTargetFromParentWell();

    auto firstTarget               = newModeledWellPath->geometryDefinition()->firstActiveTarget();
    auto locationOfFirstWellTarget = firstTarget->targetPointXYZ();
    auto offsetFirstTarget         = locationOfFirstWellTarget - sourceLocationOfFirstWellTarget;

    auto targets = newModeledWellPath->geometryDefinition()->activeWellTargets();
    for ( auto wellTarget : targets )
    {
        // Skip first target, as this is already updated by wellPathTieIn()->updateFirstTargetFromParentWell()
        if ( wellTarget == firstTarget ) continue;

        auto newTargetLocationXYZ = wellTarget->targetPointXYZ() + offsetFirstTarget;
        wellTarget->setPointXYZ( newTargetLocationXYZ );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleWellPathLaterals::updateLocationOfCompletions( RimModeledWellPath* newModeledWellPath,
                                                                     const double        sourceTieInMeasuredDepth )
{
    auto tieInMeasuredDepth = newModeledWellPath->wellPathTieIn()->tieInMeasuredDepth();
    auto diffMD             = tieInMeasuredDepth - sourceTieInMeasuredDepth;

    for ( auto completion : newModeledWellPath->completions()->allCompletionsNoConst() )
    {
        completion->applyOffset( diffMD );
    }
}
