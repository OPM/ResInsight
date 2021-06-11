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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include <QAction>
#include <QPointer>
#include <QWidget>
#include <map>
#include <memory>

class RimSummaryCalculationVariable;
class RicSummaryCurveCalculatorUi;

//==================================================================================================
///
///
//==================================================================================================
class RiuCalculationsContextMenuManager : public QObject
{
    Q_OBJECT

    static const std::map<QString, std::set<QString>> MENU_MAP;

public:
    RiuCalculationsContextMenuManager()
        : m_curveCalc( nullptr )
        , m_textPosition( 0 )
    {
    }

    void attachWidget( QWidget* widget, RicSummaryCurveCalculatorUi* curveCalc );

public slots:
    void slotMenuItems( QPoint point );

private slots:
    void slotCreateCalculationCopy();

private:
    QPointer<QWidget>                           m_widget;
    RicSummaryCurveCalculatorUi*                m_curveCalc;
    int                                         m_textPosition;
    std::map<QString, std::unique_ptr<QAction>> m_actionCache;
};
