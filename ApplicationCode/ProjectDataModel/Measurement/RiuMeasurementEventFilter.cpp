/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiuMeasurementEventFilter.h"

#include "RiaGuiApplication.h"

#include "RimMeasurement.h"

#include "RiuMainWindow.h"
#include <QKeyEvent>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMeasurementEventFilter::RiuMeasurementEventFilter( RimMeasurement* parent )
    : QObject( nullptr )
    , m_parent( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMeasurementEventFilter::registerFilter()
{
    RiaGuiApplication::instance()->installEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMeasurementEventFilter::unregisterFilter()
{
    RiaGuiApplication::instance()->removeEventFilter( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMeasurementEventFilter::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

        if ( keyEvent->key() == Qt::Key_Escape )
        {
            keyEvent->setAccepted( true );

            unregisterFilter();

            if ( m_parent )
            {
                m_parent->setMeasurementMode( RimMeasurement::MEASURE_DISABLED );
            }

            return true;
        }
    }

    return QObject::eventFilter( obj, event );
}
