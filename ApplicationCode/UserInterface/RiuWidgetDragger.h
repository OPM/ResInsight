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
#pragma once

#include <QObject>
#include <QPoint>
#include <QPointer>

class QWidget;
class QEvent;

class RiuWidgetDragger : public QObject
{
    Q_OBJECT
public:
    RiuWidgetDragger( QWidget* widgetToMove, int snapMargins = 5 );

    void addWidget( QWidget* widget );
    bool eventFilter( QObject* watched, QEvent* event ) override;

private:
    QPointer<QWidget> m_widgetToMove;
    int               m_snapMargins;
    QPoint            m_startPos;
};
