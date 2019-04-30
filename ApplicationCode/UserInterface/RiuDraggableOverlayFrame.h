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
#pragma once

#include <QFrame>
#include <QPointer>

class QColor;
class QLabel;

class RiuDraggableOverlayFrame : public QFrame
{
    Q_OBJECT
public:
    RiuDraggableOverlayFrame(QWidget* parent, QWidget* widgetToSnapTo = nullptr, const QColor& backgroundColor = QColor(255, 255, 255, 100));
    QLabel* label();

protected:
    QPointer<QLabel> m_overlayItemLabel;
};
