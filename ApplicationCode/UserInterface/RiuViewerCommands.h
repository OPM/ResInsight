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

#include "cvfStructGrid.h"
#include "cafPdmPointer.h"

#include <QObject>
#include <QPointer>

class RicViewerEventInterface;
class RimEclipseView;
class RimGeoMechView;
class RimIntersection;
class Rim3dView;
class RiuViewer;
class RivIntersectionBoxSourceInfo;
class RivIntersectionSourceInfo;

class QMouseEvent;

namespace caf {
    class PdmObject;
}

namespace cvf {
    class HitItemCollection;
    class Part;
}

class RiuViewerCommands: public QObject
{
    Q_OBJECT

public:
    explicit RiuViewerCommands(RiuViewer* ownerViewer);
    ~RiuViewerCommands();

    void            setOwnerView(Rim3dView * owner);

    void            displayContextMenu(QMouseEvent* event);
    void            handlePickAction(int winPosX, int winPosY, Qt::KeyboardModifiers keyboardModifiers);
    cvf::Vec3d      lastPickPositionInDomainCoords() const;

private:
    void            findCellAndGridIndex(const RivIntersectionSourceInfo* crossSectionSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex);
    void            findCellAndGridIndex(const RivIntersectionBoxSourceInfo* intersectionBoxSourceInfo, cvf::uint firstPartTriangleIndex, size_t* cellIndex, size_t* gridIndex);

    void            ijkFromCellIndex(size_t gridIdx, size_t cellIndex, size_t* i, size_t* j, size_t* k);
    void            extractIntersectionData(const cvf::HitItemCollection& hitItems, cvf::Vec3d* localIntersectionPoint, cvf::Vec3d* globalIntersectionPoint, std::vector<std::pair<const cvf::Part*, cvf::uint>>* partAndTriangleIndexPairs, const cvf::Part** nncPart, uint* nncPartFaceHit);

    bool            handleOverlayItemPicking(int winPosX, int winPosY);

    size_t m_currentGridIdx;
    size_t m_currentCellIndex;
    cvf::StructGridInterface::FaceType m_currentFaceIndex;
    cvf::Vec3d  m_currentPickPositionInDomainCoords;

    caf::PdmPointer<Rim3dView> m_reservoirView;

    QPointer<RiuViewer> m_viewer;

    std::vector<RicViewerEventInterface*> m_viewerEventHandlers;
};


