/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include <QWidget>
#include <QList>

class RimWellLogPlot;
class RiuWellLogTracePlot;
class QHBoxLayout;

//==================================================================================================
//
// RiuWellLogPlot
//
//==================================================================================================
class RiuWellLogPlot : public QWidget
{
    Q_OBJECT

public:
    RiuWellLogPlot(QWidget* parent = NULL);
    virtual ~RiuWellLogPlot();

    RiuWellLogTracePlot* createTracePlot();

private:
    QHBoxLayout* m_layout;
    QList<RiuWellLogTracePlot*> m_tracePlots;
};

