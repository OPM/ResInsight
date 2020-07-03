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

#include "RiuDragDrop.h"

#include "OperationsUsingObjReferences/RicPasteEclipseCasesFeature.h"
#include "RicCloseCaseFeature.h"
#include "WellLogCommands/RicNewWellLogFileCurveFeature.h"
#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMimeData.h"
#include "RimMultiPlot.h"
#include "RimPlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RiuMainWindow.h"

#include "RicWellLogTools.h"

#include "cafPdmUiItem.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAbstractItemModel>
#include <QModelIndex>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
class RiuTypedPdmObjects
{
public:
    explicit RiuTypedPdmObjects( const caf::PdmObjectGroup& objectGroup ) { objectGroup.objectsByType( &m_objects ); }

    explicit RiuTypedPdmObjects( const std::vector<caf::PdmPointer<caf::PdmObjectHandle>>& objectHandles )
    {
        for ( size_t i = 0; i < objectHandles.size(); i++ )
        {
            caf::PdmObject* obj = dynamic_cast<caf::PdmObject*>( objectHandles[i].p() );
            if ( obj ) m_objects.push_back( obj );
        }
    }

    static std::vector<T*> typedObjectsFromGroup( const caf::PdmObjectGroup& objectGroup )
    {
        RiuTypedPdmObjects<T> typedObjectsGetter( objectGroup );
        return typedObjectsGetter.typedObjects();
    }

    static bool containsTypedObjects( const caf::PdmObjectGroup& objectGroup )
    {
        RiuTypedPdmObjects<T> typedObjectsGetter( objectGroup );
        return typedObjectsGetter.typedObjects().size() > 0;
    }

    static bool containsTypedObjects( const std::vector<caf::PdmPointer<caf::PdmObjectHandle>>& objectHandles )
    {
        RiuTypedPdmObjects<T> typedObjectsGetter( objectHandles );
        return typedObjectsGetter.typedObjects().size() > 0;
    }

private:
    std::vector<T*> typedObjects()
    {
        std::vector<T*> typedObjectsVec;
        for ( size_t i = 0; i < m_objects.size(); i++ )
        {
            T* typedObject = dynamic_cast<T*>( m_objects[i].p() );
            if ( typedObject )
            {
                typedObjectsVec.push_back( typedObject );
            }
        }

        return typedObjectsVec;
    }

private:
    std::vector<caf::PdmPointer<caf::PdmObject>> m_objects;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDragDrop::RiuDragDrop()
{
    m_proposedDropAction = Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuDragDrop::~RiuDragDrop()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::DropActions RiuDragDrop::supportedDropActions() const
{
    // Keep drag items so that we can determine allowed actions while dragging
    m_dragItems = objectHandlesFromSelection();

    if ( RiuTypedPdmObjects<RimEclipseCase>::containsTypedObjects( m_dragItems ) )
    {
        return Qt::CopyAction | Qt::MoveAction;
    }
    else if ( RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects( m_dragItems ) )
    {
        return Qt::CopyAction;
    }

    return Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags RiuDragDrop::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags itemflags = nullptr;

    if ( index.isValid() )
    {
        caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();
        caf::PdmUiItem*     uiItem     = uiTreeView->uiItemFromModelIndex( index );

        caf::PdmObject* pdmObj = dynamic_cast<caf::PdmObject*>( uiItem );
        if ( pdmObj )
        {
            RimWellAllocationPlot* wellAllocationPlot = nullptr;
            pdmObj->firstAncestorOrThisOfType( wellAllocationPlot );
            if ( wellAllocationPlot ) return itemflags;
        }

        if ( dynamic_cast<RimEclipseCase*>( uiItem ) || dynamic_cast<RimWellLogCurve*>( uiItem ) ||
             dynamic_cast<RimWellLogFileChannel*>( uiItem ) || dynamic_cast<RimPlot*>( uiItem ) ||
             dynamic_cast<RimSummaryCase*>( uiItem ) )
        {
            // TODO: Remember to handle reservoir holding the main grid
            itemflags |= Qt::ItemIsDragEnabled;
        }

        if ( m_dragItems.empty() ) return itemflags;

        if ( dynamic_cast<RimEclipseCase*>( uiItem ) || dynamic_cast<RimCaseCollection*>( uiItem ) )
        {
            if ( RiuTypedPdmObjects<RimEclipseCase>::containsTypedObjects( m_dragItems ) )
            {
                itemflags |= Qt::ItemIsDropEnabled;
            }
        }
        else if ( m_proposedDropAction == Qt::MoveAction )
        {
            if ( dynamic_cast<RimWellLogPlot*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimWellLogTrack>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimMultiPlot*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimPlot>::containsTypedObjects( m_dragItems ) &&
                     !RiuTypedPdmObjects<RimWellLogTrack>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimWellLogTrack*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimWellLogCurve>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
                else if ( RiuTypedPdmObjects<RimWellLogTrack>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimWellLogCurve*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimWellLogCurve>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimPlot*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimPlot>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimSummaryCase*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimSummaryCaseCollection*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimSummaryCaseMainCollection*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
        }
        else if ( m_proposedDropAction == Qt::CopyAction )
        {
            if ( dynamic_cast<RimWellLogTrack*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if ( dynamic_cast<RimWellLogCurve*>( uiItem ) )
            {
                if ( RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects( m_dragItems ) )
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
        }
    }

    return itemflags;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::isSwapOperation( const QModelIndexList& dragIndices, const QModelIndex& dropTargetIndex )
{
    if ( dragIndices.size() == 1u )
    {
        QModelIndex dragIndex         = dragIndices.front();
        bool        sharesParent      = dropTargetIndex.parent() == dragIndex.parent();
        bool        targetIsJustAbove = dropTargetIndex.row() >= 0 && dragIndex.row() >= 0 &&
                                 ( dragIndex.row() - dropTargetIndex.row() ) == 1;
        return sharesParent && targetIsJustAbove;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    caf::PdmUiTreeView*   uiTreeView       = RiuMainWindow::instance()->projectTreeView();
    caf::PdmUiItem*       dropTargetUiItem = uiTreeView->uiItemFromModelIndex( parent );
    caf::PdmObjectHandle* dropTarget       = dynamic_cast<caf::PdmObjectHandle*>( dropTargetUiItem );
    if ( dropTarget )
    {
        caf::PdmObjectGroup        draggedObjects;
        const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>( data );

        if ( myMimeData && parent.isValid() )
        {
            QModelIndexList indices = myMimeData->indexes();
            objectGroupFromModelIndexes( &draggedObjects, indices );
        }
        else
        {
            return false;
        }

        RimIdenticalGridCaseGroup* gridCaseGroup;
        dropTarget->firstAncestorOrThisOfType( gridCaseGroup );
        if ( gridCaseGroup )
        {
            return handleGridCaseGroupDrop( action, draggedObjects, gridCaseGroup );
        }

        RimWellLogCurve* wellLogPlotCurve;
        dropTarget->firstAncestorOrThisOfType( wellLogPlotCurve );
        if ( wellLogPlotCurve )
        {
            return handleWellLogPlotCurveDrop( action,
                                               draggedObjects,
                                               wellLogPlotCurve,
                                               isSwapOperation( myMimeData->indexes(), parent ) );
        }

        RimWellLogTrack* wellLogPlotTrack;
        dropTarget->firstAncestorOrThisOfType( wellLogPlotTrack );
        if ( wellLogPlotTrack )
        {
            return handleWellLogPlotTrackDrop( action,
                                               draggedObjects,
                                               wellLogPlotTrack,
                                               row,
                                               isSwapOperation( myMimeData->indexes(), parent ) );
        }

        RimWellLogPlot* wellLogPlot;
        dropTarget->firstAncestorOrThisOfType( wellLogPlot );
        if ( wellLogPlot )
        {
            return handleWellLogPlotDrop( action,
                                          draggedObjects,
                                          wellLogPlot,
                                          row,
                                          isSwapOperation( myMimeData->indexes(), parent ) );
        }

        RimMultiPlot* multiPlot;
        dropTarget->firstAncestorOrThisOfType( multiPlot );
        if ( multiPlot )
        {
            return handleMultiPlotDrop( action, draggedObjects, multiPlot, row );
        }

        RimSummaryCaseCollection* summaryCaseCollection;
        dropTarget->firstAncestorOrThisOfType( summaryCaseCollection );
        if ( summaryCaseCollection )
        {
            return handleSummaryCaseCollectionDrop( action, draggedObjects, summaryCaseCollection );
        }

        RimSummaryCaseMainCollection* summaryCaseMainCollection;
        dropTarget->firstAncestorOrThisOfType( summaryCaseMainCollection );
        if ( summaryCaseMainCollection )
        {
            return handleSummaryCaseMainCollectionDrop( action, draggedObjects, summaryCaseMainCollection );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMimeData* RiuDragDrop::mimeData( const QModelIndexList& indexes ) const
{
    MimeDataWithIndexes* myObj = new MimeDataWithIndexes();
    myObj->setIndexes( indexes );

    return myObj;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiuDragDrop::mimeTypes() const
{
    QStringList types;
    types << MimeDataWithIndexes::formatName();
    return types;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::onDragCanceled()
{
    m_dragItems.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::moveCasesToGridGroup( caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup )
{
    std::vector<RimEclipseCase*> casesToBeDeleted =
        RiuTypedPdmObjects<RimEclipseCase>::typedObjectsFromGroup( objectGroup );

    if ( RicCloseCaseFeature::userConfirmedGridCaseGroupChange( casesToBeDeleted ) )
    {
        RicPasteEclipseCasesFeature::addCasesToGridCaseGroup( objectGroup, gridCaseGroup );

        for ( size_t i = 0; i < casesToBeDeleted.size(); i++ )
        {
            RicCloseCaseFeature::deleteEclipseCase( casesToBeDeleted[i] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleGridCaseGroupDrop( Qt::DropAction             action,
                                           caf::PdmObjectGroup&       objectGroup,
                                           RimIdenticalGridCaseGroup* gridCaseGroup )
{
    if ( action == Qt::CopyAction )
    {
        RicPasteEclipseCasesFeature::addCasesToGridCaseGroup( objectGroup, gridCaseGroup );
    }
    else if ( action == Qt::MoveAction )
    {
        moveCasesToGridGroup( objectGroup, gridCaseGroup );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleMultiPlotDrop( Qt::DropAction       action,
                                       caf::PdmObjectGroup& draggedObjects,
                                       RimMultiPlot*        multiPlot,
                                       int                  insertAtPosition )
{
    std::vector<RimPlot*> plots = RiuTypedPdmObjects<RimPlot>::typedObjectsFromGroup( draggedObjects );
    if ( plots.size() > 0 )
    {
        if ( action == Qt::MoveAction )
        {
            RimPlot* insertAfter = nullptr;
            if ( insertAtPosition > 0 )
            {
                auto visibleTracks = multiPlot->visiblePlots();
                if ( !visibleTracks.empty() )
                {
                    int insertAfterPosition = std::min( insertAtPosition - 1, (int)visibleTracks.size() - 1 );
                    insertAfter             = dynamic_cast<RimPlot*>( visibleTracks[insertAfterPosition] );
                }
            }
            multiPlot->movePlotsToThis( plots, insertAfter );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotCurveDrop( Qt::DropAction       action,
                                              caf::PdmObjectGroup& draggedObjects,
                                              RimWellLogCurve*     curveDropTarget,
                                              bool                 isSwapOperation )
{
    std::vector<RimWellLogCurve*> wellLogPlotCurves =
        RiuTypedPdmObjects<RimWellLogCurve>::typedObjectsFromGroup( draggedObjects );
    if ( wellLogPlotCurves.size() > 0 )
    {
        if ( action == Qt::MoveAction )
        {
            RimWellLogTrack* wellLogPlotTrack;
            curveDropTarget->firstAncestorOrThisOfType( wellLogPlotTrack );

            RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack( wellLogPlotTrack,
                                                                          wellLogPlotCurves,
                                                                          curveDropTarget,
                                                                          isSwapOperation );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotTrackDrop( Qt::DropAction       action,
                                              caf::PdmObjectGroup& draggedObjects,
                                              RimWellLogTrack*     trackTarget,
                                              int                  insertAtPosition,
                                              bool                 isSwapOperation )
{
    std::vector<RimWellLogFileChannel*> wellLogFileChannels =
        RiuTypedPdmObjects<RimWellLogFileChannel>::typedObjectsFromGroup( draggedObjects );
    if ( wellLogFileChannels.size() > 0 )
    {
        if ( action == Qt::CopyAction )
        {
            RicWellLogTools::addWellLogChannelsToPlotTrack( trackTarget, wellLogFileChannels );
            return true;
        }
    }

    std::vector<RimWellLogCurve*> wellLogPlotCurves =
        RiuTypedPdmObjects<RimWellLogCurve>::typedObjectsFromGroup( draggedObjects );
    if ( wellLogPlotCurves.size() > 0 )
    {
        if ( action == Qt::MoveAction )
        {
            RimWellLogCurve* insertAfter = nullptr;
            if ( insertAtPosition > 0 )
            {
                auto visibleCurves = trackTarget->visibleCurves();
                if ( !visibleCurves.empty() )
                {
                    int insertAfterPosition = std::min( insertAtPosition - 1, (int)visibleCurves.size() - 1 );
                    insertAfter             = visibleCurves[insertAfterPosition];
                }
            }
            RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack( trackTarget,
                                                                          wellLogPlotCurves,
                                                                          insertAfter,
                                                                          isSwapOperation );
            return true;
        }
    }

    std::vector<RimWellLogTrack*> wellLogPlotTracks =
        RiuTypedPdmObjects<RimWellLogTrack>::typedObjectsFromGroup( draggedObjects );
    if ( wellLogPlotTracks.size() > 0 )
    {
        if ( action == Qt::MoveAction )
        {
            RimWellLogPlot* wellLogPlot;
            trackTarget->firstAncestorOrThisOfType( wellLogPlot );
            return handleWellLogPlotDrop( action, draggedObjects, wellLogPlot, insertAtPosition );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotDrop( Qt::DropAction       action,
                                         caf::PdmObjectGroup& draggedObjects,
                                         RimWellLogPlot*      wellLogPlotTarget,
                                         int                  insertAtPosition,
                                         bool                 isSwapOperation )
{
    std::vector<RimWellLogTrack*> wellLogPlotTracks =
        RiuTypedPdmObjects<RimWellLogTrack>::typedObjectsFromGroup( draggedObjects );
    if ( wellLogPlotTracks.size() > 0 )
    {
        if ( action == Qt::MoveAction )
        {
            RimWellLogTrack* insertAfter = nullptr;
            if ( insertAtPosition > 0 )
            {
                auto visibleTracks = wellLogPlotTarget->visiblePlots();
                if ( !visibleTracks.empty() )
                {
                    int insertAfterPosition = std::min( insertAtPosition - 1, (int)visibleTracks.size() - 1 );
                    insertAfter             = dynamic_cast<RimWellLogTrack*>( visibleTracks[insertAfterPosition] );
                }
            }
            RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot( wellLogPlotTarget,
                                                                     wellLogPlotTracks,
                                                                     insertAfter,
                                                                     isSwapOperation );
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleSummaryCaseCollectionDrop( Qt::DropAction            action,
                                                   caf::PdmObjectGroup&      draggedObjects,
                                                   RimSummaryCaseCollection* summaryCaseDropTarget )
{
    std::vector<RimSummaryCase*> summaryCases = RiuTypedPdmObjects<RimSummaryCase>::typedObjectsFromGroup( draggedObjects );

    if ( action != Qt::MoveAction || summaryCases.size() == 0 ) return false;

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        RimSummaryCaseCollection* summaryCaseCollection;
        summaryCase->firstAncestorOrThisOfType( summaryCaseCollection );

        if ( summaryCaseCollection )
        {
            summaryCaseCollection->removeCase( summaryCase );
            summaryCaseDropTarget->addCase( summaryCase );
            summaryCaseCollection->updateConnectedEditors();
            continue;
        }

        RimSummaryCaseMainCollection* summaryCaseMainCollection;
        summaryCase->firstAncestorOrThisOfType( summaryCaseMainCollection );
        if ( summaryCaseMainCollection )
        {
            summaryCaseMainCollection->removeCase( summaryCase );
            summaryCaseDropTarget->addCase( summaryCase );
            summaryCaseMainCollection->updateConnectedEditors();
        }
    }
    summaryCaseDropTarget->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleSummaryCaseMainCollectionDrop( Qt::DropAction                action,
                                                       caf::PdmObjectGroup&          draggedObjects,
                                                       RimSummaryCaseMainCollection* summaryCaseDropTarget )
{
    std::vector<RimSummaryCase*> summaryCases = RiuTypedPdmObjects<RimSummaryCase>::typedObjectsFromGroup( draggedObjects );

    if ( action != Qt::MoveAction || summaryCases.size() == 0 ) return false;

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        RimSummaryCaseCollection* summaryCaseCollection;
        summaryCase->firstAncestorOrThisOfType( summaryCaseCollection );

        if ( summaryCaseCollection )
        {
            summaryCaseCollection->removeCase( summaryCase );
            summaryCaseDropTarget->addCase( summaryCase );
            summaryCaseCollection->updateConnectedEditors();
        }
    }

    summaryCaseDropTarget->updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::objectGroupFromModelIndexes( caf::PdmObjectGroup* objectGroup, const QModelIndexList& indexes )
{
    CVF_ASSERT( objectGroup );

    objectGroup->objects.clear();
    caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();

    for ( int i = 0; i < indexes.size(); i++ )
    {
        caf::PdmUiItem*       uiItem    = uiTreeView->uiItemFromModelIndex( indexes[i] );
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( uiItem );
        if ( objHandle )
        {
            objectGroup->objects.push_back( objHandle );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<caf::PdmObjectHandle>> RiuDragDrop::objectHandlesFromSelection()
{
    std::vector<caf::PdmObjectHandle*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );

    std::vector<caf::PdmPointer<caf::PdmObjectHandle>> objectHandles;

    for ( size_t sIdx = 0; sIdx < selection.size(); sIdx++ )
    {
        objectHandles.push_back( selection[sIdx] );
    }

    return objectHandles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::onProposedDropActionUpdated( Qt::DropAction action )
{
    m_proposedDropAction = action;
}
