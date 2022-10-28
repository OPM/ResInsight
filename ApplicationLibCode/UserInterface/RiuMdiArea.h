/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include <QMdiArea>

#include <list>

class QMdiSubWindow;

class RiuMdiArea : public QMdiArea
{
    Q_OBJECT

public:
    enum class TileMode
    {
        NO_TILING,
        DEFAULT_TILING,
        TILE_VERTICALLY,
        TILE_HORIZONTALLY
    };

public:
    RiuMdiArea( QWidget* parent = nullptr );
    ~RiuMdiArea() override;

    void     setTileMode( TileMode tileMode );
    TileMode tileMode() const;
    void     updateTiling();

    std::list<QMdiSubWindow*> subWindowListSortedByPosition();

protected:
    void resizeEvent( QResizeEvent* resizeEvent ) override;
    void moveEvent( QMoveEvent* event ) override;

    bool subWindowsAreTiled() const;
    void tileWindowsHorizontally();
    void tileWindowsVertically();
    void tileWindowsDefault();

private:
    TileMode m_tileMode;
};
