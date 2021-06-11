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

#include "RiuAbstractLegendFrame.h"

#include "cvfObject.h"

#include "cafCategoryMapper.h"

#include <QLabel>
#include <QSize>

class QFont;
class QPaintEvent;
class QPainter;
class QRect;

class RiuCategoryLegendFrame : public RiuAbstractLegendFrame
{
    Q_OBJECT

public:
    RiuCategoryLegendFrame( QWidget* parent, const QString& title, caf::CategoryMapper* categoryMapper );
    ~RiuCategoryLegendFrame() override;

private:
    void    layoutInfo( LayoutInfo* layout ) const override;
    QString label( int index ) const override;
    int     labelCount() const override;
    int     rectCount() const override;
    void    renderRect( QPainter* painter, const LayoutInfo& layout, int rectIndex ) const override;
    QRect   labelRect( const LayoutInfo& layout, int index ) const override;

private:
    cvf::cref<caf::CategoryMapper> m_categoryMapper;
};
