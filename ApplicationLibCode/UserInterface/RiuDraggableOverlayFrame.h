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

#include "RiuAbstractOverlayContentFrame.h"

#include <QFrame>
#include <QLabel>
#include <QPointer>

class QColor;
class QVBoxLayout;
class RiuWidgetDragger;

class RiuDraggableOverlayFrame : public QFrame
{
    Q_OBJECT
public:
    enum class AnchorCorner
    {
        TopLeft,
        TopRight,
    };

public:
    RiuDraggableOverlayFrame( QWidget*      parent,
                              const int     snapMargins,
                              const QColor& backgroundColor = QColor( 255, 255, 255, 100 ) );

    RiuAbstractOverlayContentFrame* contentFrame();
    void                            setContentFrame( RiuAbstractOverlayContentFrame* contentFrame );
    void                            renderTo( QPainter* painter, const QRect& targetRect );

    void         setAnchorCorner( AnchorCorner corner );
    AnchorCorner anchorCorner() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QPointer<RiuWidgetDragger>               m_widgetDragger;
    QPointer<QVBoxLayout>                    m_layout;
    QPointer<RiuAbstractOverlayContentFrame> m_contentFrame;
    AnchorCorner                             m_anchorCorner;
};
