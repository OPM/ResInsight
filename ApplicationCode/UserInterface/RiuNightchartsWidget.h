/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


#include "nightcharts/nightcharts.h"

#include "cafPdmPointer.h"

#include <QWidget>
#include <QPaintEvent>


class RiuNightchartsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RiuNightchartsWidget(QWidget* parent = nullptr);

    void addItem(const QString& name, const QColor& color, float value);
    void setType(Nightcharts::type type);
    void showLegend(bool doShow);
    void showPie(bool doShow);

    void clear();


    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent* e);

private:
    void updateSizePolicy();

    Nightcharts m_chart;
    int         m_marginLeft;
    int         m_marginTop;
    bool        m_showLegend;
    bool        m_showPie;
    int         m_maxNameWidth;
};

