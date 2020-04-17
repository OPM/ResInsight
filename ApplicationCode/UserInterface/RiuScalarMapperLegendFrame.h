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

#include "RimRegularLegendConfig.h"
#include "RiuAbstractLegendFrame.h"

#include "cvfObject.h"
#include "cvfRect.h"
#include "cvfScalarMapper.h"

#include <QLabel>
#include <QSize>

class QFont;
class QPaintEvent;
class QPainter;
class QRect;

class RiuScalarMapperLegendFrame : public RiuAbstractLegendFrame
{
    Q_OBJECT

public:
    using NumberFormat = RimRegularLegendConfig::NumberFormatType;

public:
    RiuScalarMapperLegendFrame( QWidget* parent, const QString& title, cvf::ScalarMapper* scalarMapper );
    ~RiuScalarMapperLegendFrame();

    void setTickPrecision( int precision );
    void setTickFormat( NumberFormat format );

private:
    void    layoutInfo( LayoutInfo* layout ) const override;
    QString label( int index ) const override;
    int     labelCount() const override;
    int     rectCount() const override;
    void    renderRect( QPainter* painter, const LayoutInfo& layout, int rectIndex ) const override;
    QRect   labelRect( const LayoutInfo& layout, int index ) const override;

private:
    cvf::cref<cvf::ScalarMapper> m_scalarMapper;
    std::vector<double>          m_tickValues;
    int                          m_tickNumberPrecision;
    NumberFormat                 m_numberFormat;
};
