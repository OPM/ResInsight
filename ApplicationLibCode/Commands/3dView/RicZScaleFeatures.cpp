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
#include "RiaDefines.h"

#include "ContourMap/RimEclipseContourMapView.h"
#include "Rim3dView.h"

#include <QAction>

#include <algorithm>

CAF_CMD_SOURCE_INIT( RicIncreaseZScaleFeature, "RicIncreaseZScaleFeature" );
CAF_CMD_SOURCE_INIT( RicDecreaseZScaleFeature, "RicDecreaseZScaleFeature" );

namespace
{
Rim3dView* activeViewWithEditableZScale()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view || dynamic_cast<RimEclipseContourMapView*>( view ) != nullptr ) return nullptr;
    if ( !view->isScaleZEditable() ) return nullptr;

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
        auto scaleOptions = RiaDefines::viewScaleOptions();
        auto it           = std::upper_bound( scaleOptions.begin(), scaleOptions.end(), view->scaleZ() );
        if ( it != scaleOptions.end() )
        {
            view->setScaleZAndUpdate( *it );
        }
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
        auto scaleOptions = RiaDefines::viewScaleOptions();
        auto it           = std::lower_bound( scaleOptions.begin(), scaleOptions.end(), view->scaleZ() );
        if ( it != scaleOptions.begin() )
        {
            view->setScaleZAndUpdate( *( it - 1 ) );
        }
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
