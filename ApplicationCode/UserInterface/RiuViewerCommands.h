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
#include <QObject>

class RiuViewer;
class RimView;
class RimEclipseView;
class RimGeoMechView;

class QMouseEvent;

#include "cvfStructGrid.h"
#include "cafPdmPointer.h"
#include <QPointer>

namespace cvf {
    class HitItemCollection;
    class Part;
}

class RiuViewerCommands: public QObject
{
    Q_OBJECT

public:
    RiuViewerCommands(RiuViewer* ownerViewer);
    ~RiuViewerCommands();

    void setOwnerView(RimView * owner);

    void            displayContextMenu(QMouseEvent* event);
    void            handlePickAction(int winPosX, int winPosY, Qt::KeyboardModifiers keyboardModifiers);


private slots:
    void            slotRangeFilterI();
    void            slotRangeFilterJ();
    void            slotRangeFilterK();
    void            slotHideFault();
    void            slotAddEclipsePropertyFilter();
    void            slotAddGeoMechPropertyFilter();

private:
    void            ijkFromCellIndex(size_t gridIdx, size_t cellIndex, size_t* i, size_t* j, size_t* k);
    void            createSliceRangeFilter(int ijOrk);
    void            extractIntersectionData(const cvf::HitItemCollection& hitItems, cvf::Vec3d* localIntersectionPoint, cvf::Part** firstPart, uint* firstPartFaceHit, cvf::Part** nncPart, uint* nncPartFaceHit);
    void            updateSelectionFromPickedPart(cvf::Part* part);

    size_t m_currentGridIdx;
    size_t m_currentCellIndex;
    cvf::StructGridInterface::FaceType m_currentFaceIndex;

    caf::PdmPointer<RimView> m_reservoirView;

    QPointer<RiuViewer> m_viewer;
};


