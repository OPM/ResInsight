/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Statoil ASA
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
#include "RicRemoveComparison3dViewFeature.h"

#include "RiaApplication.h"

#include "Rim3dOverlayInfoConfig.h"
#include "Rim3dView.h"
#include "RimGridView.h"

#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicRemoveComparison3dViewFeature, "RicRemoveComparison3dViewFeature" );

class RemoveComparison3dViewImpl
{
public:
    bool makeReady()
    {
        m_activeView = RiaApplication::instance()->activeReservoirView();
        if ( m_activeView && m_activeView->viewer() && m_activeView->viewer()->viewerCommands() &&
             m_activeView->viewer()->viewerCommands()->isCurrentPickInComparisonView() )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void execute()
    {
        m_activeView->setComparisonView( nullptr );
        m_activeView->scheduleCreateDisplayModelAndRedraw();

        auto gridView = dynamic_cast<RimGridView*>( m_activeView );
        if ( gridView ) gridView->overlayInfoConfig()->updateConnectedEditors();
    }

private:
    Rim3dView* m_activeView = nullptr;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicRemoveComparison3dViewFeature::isCommandEnabled()
{
    RemoveComparison3dViewImpl cmdImpl;

    return cmdImpl.makeReady();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRemoveComparison3dViewFeature::onActionTriggered( bool isChecked )
{
    RemoveComparison3dViewImpl cmdImpl;

    if ( cmdImpl.makeReady() )
    {
        cmdImpl.execute();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicRemoveComparison3dViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Stop Comparison" );
    actionToSetup->setIcon( QIcon( ":/RemoveComparisonView16x16.png" ) );
}
