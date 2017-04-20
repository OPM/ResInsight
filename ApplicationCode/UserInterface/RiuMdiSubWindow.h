/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include <QMdiSubWindow>
#include "RimViewWindow.h"


class RiuMdiSubWindow : public QMdiSubWindow
{
public:
    RiuMdiSubWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    ~RiuMdiSubWindow();

    static RimMdiWindowGeometry windowGeometryForWidget(QWidget* widget);

protected:
    virtual void closeEvent(QCloseEvent* event);
};

