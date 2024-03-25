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

class RiuTextContentFrame : public RiuAbstractOverlayContentFrame
{
    Q_OBJECT

public:
    RiuTextContentFrame( QWidget* parent, const QString& title, const QString& text, int fontPixeSize );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void renderTo( QPainter* painter, const QRect& targetRect ) override;

private:
    struct LayoutInfo
    {
        LayoutInfo( const QSize& size )
        {
            charHeight  = 0;
            charAscent  = 0;
            lineSpacing = 0;
            margins     = QMargins( 0, 0, 0, 0 );

            overallLegendSize = size;
        }

        int      charHeight;
        int      charAscent;
        int      lineSpacing;
        QMargins margins;
        int      tickTextLeadSpace;
        QSize    overallLegendSize;
    };

    void paintEvent( QPaintEvent* e ) override;

private:
    virtual void layoutInfo( LayoutInfo* layout ) const;

    void updateFontSize();

private:
    QString m_title;
    QString m_text;
    int     m_fontPixelSize;
};
