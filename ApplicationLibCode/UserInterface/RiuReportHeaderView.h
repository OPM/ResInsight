/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <QColor>
#include <QHeaderView>

//==================================================================================================
/// Horizontal header showing the column title in bold, and the unit as a smaller muted second line
//==================================================================================================
class RiuReportHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    struct Colors
    {
        QColor background;
        QColor text;
        QColor mutedText;
        QColor border;
    };

    RiuReportHeaderView( const Colors& colors, QWidget* parent = nullptr );

protected:
    void  paintSection( QPainter* painter, const QRect& rect, int logicalIndex ) const override;
    QSize sectionSizeFromContents( int logicalIndex ) const override;

private:
    QFont titleFont() const;
    QFont unitFont() const;
    bool  anyColumnHasUnit() const;

private:
    Colors m_colors;
};
