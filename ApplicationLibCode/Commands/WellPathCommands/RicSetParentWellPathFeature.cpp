/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicSetParentWellPathFeature.h"

#include "RigWellPath.h"

#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathTieIn.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_PDM_SOURCE_INIT( RicSelectWellPathUi, "RicSelectWellPathUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSelectWellPathUi::RicSelectWellPathUi()
{
    CAF_PDM_InitObject( "RicSelectWellPathUi" );

    CAF_PDM_InitFieldNoDefault( &m_selectedWellPath, "SelectedWellPath", "Well Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectWellPathUi::setWellPaths( const std::vector<RimWellPath*>& wellPaths )
{
    m_wellPaths = wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectWellPathUi::setSelectedWell( RimWellPath* selectedWell )
{
    m_selectedWellPath = selectedWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicSelectWellPathUi::wellPath() const
{
    return m_selectedWellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectWellPathUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

    RimTools::optionItemsForSpecifiedWellPaths( m_wellPaths, &options );
    return options;
}

CAF_CMD_SOURCE_INIT( RicSetParentWellPathFeature, "RicSetParentWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetParentWellPathFeature::onActionTriggered( bool isChecked )
{
    auto selectedWellPath = caf::SelectionManager::instance()->selectedItemOfType<RimWellPath>();
    if ( !selectedWellPath ) return;

    auto wpc = RimTools::wellPathCollection();
    if ( !wpc ) return;

    std::vector<RimWellPath*> wellPathCandidates;
    for ( auto w : wpc->allWellPaths() )
    {
        if ( w != selectedWellPath ) wellPathCandidates.push_back( w );
    }

    RicSelectWellPathUi ui;
    ui.setWellPaths( wellPathCandidates );

    RimWellPath* parentWell = nullptr;
    if ( selectedWellPath->wellPathTieIn() ) parentWell = selectedWellPath->wellPathTieIn()->parentWell();
    ui.setSelectedWell( parentWell );

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, &ui, "Select Parent Well", "" );
    propertyDialog.resize( QSize( 400, 200 ) );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        auto parentWellPath = ui.wellPath();

        double tieInMeasuredDepth = 0.0;
        if ( parentWellPath )
        {
            if ( !parentWellPath->wellPathGeometry() || parentWellPath->wellPathGeometry()->measuredDepths().size() < 2 ) return;
            if ( selectedWellPath->wellPathGeometry()->wellPathPoints().empty() ) return;

            auto headOfLateral = selectedWellPath->wellPathGeometry()->wellPathPoints().front();

            cvf::Vec3d p1, p2;
            parentWellPath->wellPathGeometry()->twoClosestPoints( headOfLateral, &p1, &p2 );

            tieInMeasuredDepth = parentWellPath->wellPathGeometry()->closestMeasuredDepth( p1 );
        }

        selectedWellPath->connectWellPaths( parentWellPath, tieInMeasuredDepth );

        wpc->rebuildWellPathNodes();
        wpc->scheduleRedrawAffectedViews();
        wpc->updateAllRequiredEditors();

        RiuMainWindow::instance()->setExpanded( selectedWellPath );
        RiuMainWindow::instance()->selectAsCurrentItem( selectedWellPath );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSetParentWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Set Parent Well Path" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
