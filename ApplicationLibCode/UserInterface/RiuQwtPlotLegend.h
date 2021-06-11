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

#include "qwt_legend.h"

class RiuQwtPlotLegend : public QwtLegend
{
    Q_OBJECT
public:
    RiuQwtPlotLegend( QWidget* parent = nullptr );
    void  resizeEvent( QResizeEvent* event ) override;
    QSize sizeHint() const override;
public slots:
    void updateLegend( const QVariant&, const QList<QwtLegendData>& ) override;

signals:
    void legendUpdated();

private:
    int m_columnCount;
};
