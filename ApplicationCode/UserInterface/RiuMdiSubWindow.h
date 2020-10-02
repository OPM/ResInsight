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

#include "RimViewWindow.h"
#include <QMdiSubWindow>

class RiuMdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    RiuMdiSubWindow( QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr );

    ~RiuMdiSubWindow() override;

    RimMdiWindowGeometry windowGeometry() const;

    void blockTilingChanges( bool block );

protected:
    void closeEvent( QCloseEvent* event ) override;
    void resizeEvent( QResizeEvent* resizeEvent ) override;
    void moveEvent( QMoveEvent* moveEvent ) override;
    void showEvent( QShowEvent* event ) override;

private:
    QRect m_normalWindowGeometry;
    bool  m_blockTilingChanges;
};
