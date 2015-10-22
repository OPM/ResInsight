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

#include "cafPdmUiDragDropHandle.h"
#include "cafPdmPointer.h"
#include "cafPdmObjectGroup.h"

#include <vector>

namespace caf
{
    class PdmObjectHandle;
}

class RimIdenticalGridCaseGroup;
class RimWellLogPlotTrack;
class RimWellLogPlot;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuDragDrop : public caf::PdmUiDragDropHandle
{
public:
    RiuDragDrop();
    virtual ~RiuDragDrop();

    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual void endDrag();
    virtual void setProposedAction(Qt::DropAction action);

private:
    void moveCasesToGridGroup(caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup);
    bool handleGridCaseGroupDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup);
    bool handleWellLogPlotTrackDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimWellLogPlotTrack* wellLogPlotTrack);
    bool handleWellLogPlotDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimWellLogPlot* wellLogPlot);

    static void objectGroupFromModelIndexes(caf::PdmObjectGroup* objectGroup, const QModelIndexList &indexes);
    static std::vector<caf::PdmPointer<caf::PdmObjectHandle> > objectHandlesFromSelection();

private:
    mutable std::vector<caf::PdmPointer<caf::PdmObjectHandle> > m_dragItems;
    Qt::DropAction m_proposedAction;
};

