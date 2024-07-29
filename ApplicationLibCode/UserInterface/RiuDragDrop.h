/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "cafPdmObjectGroup.h"
#include "cafPdmPointer.h"
#include "cafPdmUiDragDropInterface.h"

#include <vector>

namespace caf
{
class PdmObjectHandle;
class PdmUiTreeView;
} // namespace caf

class RimMultiPlot;
class RimIdenticalGridCaseGroup;
class RimSummaryEnsemble;
class RimSummaryCaseMainCollection;
class RimSummaryCurve;
class RimSummaryPlot;
class RimSurfaceCollection;
class RimWellLogPlot;
class RimWellLogTrack;
class RimWellLogCurve;
class QEvent;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiuDragDrop : public caf::PdmUiDragDropInterface
{
public:
    RiuDragDrop( caf::PdmUiTreeView* treeView );
    ~RiuDragDrop() override;

    static std::vector<caf::PdmObjectHandle*> draggedObjectsFromTreeView( caf::PdmUiTreeView* dragSource, const QMimeData* data );

    static bool handleGenericDropEvent( QEvent* event, std::vector<caf::PdmObjectHandle*>& droppedObjects );

    static std::vector<caf::PdmObjectHandle*> convertToObjects( const QMimeData* mimeData );

protected:
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags   flags( const QModelIndex& index ) const override;
    bool            dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent ) override;
    QMimeData*      mimeData( const QModelIndexList& indexes ) const override;
    QStringList     mimeTypes() const override;

    void onDragCanceled() override;
    void onProposedDropActionUpdated( Qt::DropAction action ) override;

private:
    void moveCasesToGridGroup( caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup );
    bool handleGridCaseGroupDrop( Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup );

    bool handleWellLogPlotTrackDrop( Qt::DropAction       action,
                                     caf::PdmObjectGroup& objectGroup,
                                     RimWellLogTrack*     wellLogPlotTrack,
                                     int                  insertAtPosition );

    bool handleWellLogPlotDrop( Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimWellLogPlot* wellLogPlot, int insertAtPosition );

    bool handleMultiPlotDrop( Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimMultiPlot* multiPlot, int insertAtPosition );
    bool handleSummaryCaseCollectionDrop( Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimSummaryEnsemble* summaryCaseCollection );
    bool handleSummaryCaseMainCollectionDrop( Qt::DropAction                action,
                                              caf::PdmObjectGroup&          objectGroup,
                                              RimSummaryCaseMainCollection* summaryCaseMainCollection );

    bool handleSurfaceCollectionDrop( Qt::DropAction action, int row, caf::PdmObjectGroup& objectGroup, RimSurfaceCollection* surfaceCollection );

    static void objectGroupFromModelIndexes( caf::PdmUiTreeView* uiTreeView, caf::PdmObjectGroup* objectGroup, const QModelIndexList& indexes );
    static std::vector<caf::PdmPointer<caf::PdmObjectHandle>> objectHandlesFromSelection();

private:
    mutable std::vector<caf::PdmPointer<caf::PdmObjectHandle>> m_dragItems;

    Qt::DropAction      m_proposedDropAction;
    caf::PdmUiTreeView* m_projectTreeView;
};
