/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicZScaleFeatures.h"

#include "RiaApplication.h"
#include "RiaZScaleTools.h"

#include "Rim3dView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicIncreaseZScaleFeature, "RicIncreaseZScaleFeature" );
CAF_CMD_SOURCE_INIT( RicDecreaseZScaleFeature, "RicDecreaseZScaleFeature" );

namespace
{
Rim3dView* activeViewWithEditableZScale()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view || !view->isScaleZEditable() ) return nullptr;

    return view;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicIncreaseZScaleFeature::isCommandEnabled() const
{
    return activeViewWithEditableZScale() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIncreaseZScaleFeature::onActionTriggered( bool isChecked )
{
    if ( Rim3dView* view = activeViewWithEditableZScale() )
    {
        double nextScale = RiaZScaleTools::nextScaleFactor( view->scaleZ(), RiaZScaleTools::scaleFactorOptions() );
        view->setScaleZAndUpdate( nextScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIncreaseZScaleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Increase Z Scale" );
    actionToSetup->setToolTip( "Increase Z Scale (Ctrl+Shift+Up)" );
    actionToSetup->setIcon( QIcon( ":/ZScaleIncrease.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Shift+Up" ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDecreaseZScaleFeature::isCommandEnabled() const
{
    return activeViewWithEditableZScale() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDecreaseZScaleFeature::onActionTriggered( bool isChecked )
{
    if ( Rim3dView* view = activeViewWithEditableZScale() )
    {
        double previousScale = RiaZScaleTools::previousScaleFactor( view->scaleZ(), RiaZScaleTools::scaleFactorOptions() );
        view->setScaleZAndUpdate( previousScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDecreaseZScaleFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Decrease Z Scale" );
    actionToSetup->setToolTip( "Decrease Z Scale (Ctrl+Shift+Down)" );
    actionToSetup->setIcon( QIcon( ":/ZScaleDecrease.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Shift+Down" ) ) );
}
