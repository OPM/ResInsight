/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cvfBase.h"
#include "cvfObject.h"

#include <QPointer>

#include <QMouseEvent>
#include <QPainter>

namespace caf
{
class Viewer;
}

class RiuComparisonViewMover : public QObject
{
public:
    RiuComparisonViewMover( caf::Viewer* viewer );

    virtual bool eventFilter( QObject* watched, QEvent* event ) override;

    void paintMoverHandles( QPainter* painter );

private:
    enum DragState
    {
        NONE,
        COMPLETE_BOX,
        LEFT_EDGE,
        RIGHT_EDGE,
        TOP_EDGE,
        BOTTOM_EDGE,
    };

    DragState findHandleUnderMouse( const QPoint& mousePos );

    QPointer<caf::Viewer> m_viewer;
    DragState             m_dragState;
    DragState             m_highlightHandle;
};
