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
#include "RiuDraggableOverlayFrame.h"
#include "RiuWidgetDragger.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDraggableOverlayFrame::RiuDraggableOverlayFrame(QWidget* parent, QWidget* widgetToSnapTo, const QColor& backgroundColor)
    : QFrame(parent)
{
    RiuWidgetDragger* dragger = new RiuWidgetDragger(this, widgetToSnapTo);

    QPalette pal = this->palette();
    pal.setColor(QPalette::Background, backgroundColor);
    setAutoFillBackground(true);
    setPalette(pal);
    setFrameShape(QFrame::Box);
    QGraphicsDropShadowEffect* dropShadowEffect = new QGraphicsDropShadowEffect(this);
    dropShadowEffect->setOffset(1.0, 1.0);
    dropShadowEffect->setBlurRadius(3.0);
    dropShadowEffect->setColor(QColor(100, 100, 100, 100));
    setGraphicsEffect(dropShadowEffect);

    auto hblayout = new QVBoxLayout(this);
    this->setLayout(hblayout);

    m_overlayItemLabel = new QLabel(this);
    hblayout->addWidget(m_overlayItemLabel);
    m_overlayItemLabel->setObjectName("OverlayFrameLabel");
    m_overlayItemLabel->setGraphicsEffect(nullptr);
    m_overlayItemLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    dragger->addWidget(m_overlayItemLabel);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* RiuDraggableOverlayFrame::label()
{
    return m_overlayItemLabel;
}
