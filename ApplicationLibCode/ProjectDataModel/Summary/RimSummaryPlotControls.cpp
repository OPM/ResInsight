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

#include "RimSummaryPlotControls.h"

#include "RimSummaryPlotSourceStepping.h"

#include <QKeyEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotControls::handleKeyEvents( RimSummaryPlotSourceStepping* srcStepping, QKeyEvent* keyEvent )
{
    if ( !keyEvent ) return false;
    if ( !srcStepping ) return false;

    if ( !( keyEvent->modifiers() & Qt::ControlModifier ) ) return false;

    bool bHandled = false;

    if ( keyEvent->key() == Qt::Key_Up )
    {
        srcStepping->applyPrevStep();
        keyEvent->accept();
        bHandled = true;
    }
    else if ( keyEvent->key() == Qt::Key_Down )
    {
        srcStepping->applyNextStep();
        keyEvent->accept();
        bHandled = true;
    }

    return bHandled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotControls::nextStepKeyText()
{
    return QString( "Ctrl-Down" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotControls::prevStepKeyText()
{
    return QString( "Ctrl-Up" );
}
