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
#include <QString>

class RiuAbstractLegendFrame : public RiuAbstractOverlayContentFrame
{
    Q_OBJECT

public:
    RiuAbstractLegendFrame( QWidget* parent, const QString& title );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void renderTo( QPainter* painter, const QRect& targetRect ) override;

protected:
    struct LayoutInfo
    {
        LayoutInfo( const QSize& size )
        {
            charHeight  = 0;
            charAscent  = 0;
            lineSpacing = 0;
            margins     = QMargins( 0, 0, 0, 0 );
            tickEndX    = 0;
            tickStartX  = 0;
            tickMidX    = 0;

            tickTextLeadSpace = 5;
            overallLegendSize = size;
        }

        int      charHeight;
        int      charAscent;
        int      lineSpacing;
        QMargins margins;
        int      tickStartX, tickMidX, tickEndX;
        int      tickTextLeadSpace;

        QRect colorBarRect;

        std::vector<int> tickYPixelPos;

        QSize overallLegendSize;
    };

    void                                   paintEvent( QPaintEvent* e ) override;
    std::vector<std::pair<QRect, QString>> visibleLabels( const LayoutInfo& layout ) const;

private:
    virtual void    layoutInfo( LayoutInfo* layout ) const                 = 0;
    virtual QString label( int index ) const                               = 0;
    virtual QRect   labelRect( const LayoutInfo& layout, int index ) const = 0;
    virtual int     labelCount() const                                     = 0;
    virtual int     rectCount() const                                      = 0;

    virtual void renderRect( QPainter* painter, const LayoutInfo& layout, int rectIndex ) const = 0;

protected:
    QString m_title;
};
