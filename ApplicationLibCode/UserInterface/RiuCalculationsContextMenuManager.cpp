/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuCalculationsContextMenuManager.h"

#include "RimSummaryCalculationCollection.h"

#include "RicUserDefinedCalculatorUi.h"

#include <QMenu>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCalculationsContextMenuManager::attachWidget( QWidget* widget, RicUserDefinedCalculatorUi* curveCalc )
{
    if ( m_widget != widget )
    {
        widget->setContextMenuPolicy( Qt::CustomContextMenu );
        QObject::connect( widget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( slotMenuItems( QPoint ) ) );

        m_widget    = widget;
        m_curveCalc = curveCalc;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCalculationsContextMenuManager::slotMenuItems( QPoint point )
{
    QMenu menu;

    QAction action( "Create copy", this );
    connect( &action, SIGNAL( triggered() ), SLOT( slotCreateCalculationCopy() ) );
    action.setEnabled( m_curveCalc->currentCalculation() != nullptr );

    menu.addAction( &action );

    QPoint globalPoint = point;
    if ( m_widget )
    {
        globalPoint = m_widget->mapToGlobal( point );
    }

    menu.exec( globalPoint );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCalculationsContextMenuManager::slotCreateCalculationCopy()
{
    if ( m_widget != nullptr && m_curveCalc != nullptr )
    {
        RimUserDefinedCalculation* currCalculation = m_curveCalc->currentCalculation();
        if ( currCalculation != nullptr )
        {
            RimUserDefinedCalculationCollection* coll     = m_curveCalc->calculationCollection();
            RimUserDefinedCalculation*           calcCopy = coll->addCalculationCopy( currCalculation );
            m_curveCalc->setCurrentCalculation( calcCopy );
            m_curveCalc->updateConnectedEditors();
        }
    }
}
