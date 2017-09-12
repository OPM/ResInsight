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
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellLogTrack.h"
#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"
#include "cafPdmUiItem.h"

#include <QAbstractItemModel>
#include <QModelIndex>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
class RiuTypedPdmObjects
{
public:
    explicit RiuTypedPdmObjects(const caf::PdmObjectGroup& objectGroup)
    {
        objectGroup.objectsByType(&m_typedObjects);
    }

    explicit RiuTypedPdmObjects(const std::vector<caf::PdmPointer<caf::PdmObjectHandle> >& objectHandles)
    {
        for (size_t i = 0; i < objectHandles.size(); i++)
        {
            T* obj = dynamic_cast<T*>(objectHandles[i].p());
            if (obj) m_typedObjects.push_back(obj);
        }
    }

    static std::vector<T*> typedObjectsFromGroup(const caf::PdmObjectGroup& objectGroup)
    {
        RiuTypedPdmObjects<T> typedObjectsGetter(objectGroup);
        return typedObjectsGetter.typedObjects();
    }

    static bool containsTypedObjects(const caf::PdmObjectGroup& objectGroup)
    {
        RiuTypedPdmObjects<T> typedObjectsGetter(objectGroup);
        return typedObjectsGetter.typedObjects().size() > 0;
    }

    static bool containsTypedObjects(const std::vector<caf::PdmPointer<caf::PdmObjectHandle> >& objectHandles)
    {
        RiuTypedPdmObjects<T> typedObjectsGetter(objectHandles);
        return typedObjectsGetter.typedObjects().size() > 0;
    }

private:
    std::vector<T*> typedObjects()
    {
        std::vector<T*> typedObjectsVec;
        for (size_t i = 0; i < m_typedObjects.size(); i++)
        {
            typedObjectsVec.push_back(m_typedObjects[i].p());
        }

        return typedObjectsVec;
    }

private:
    std::vector<caf::PdmPointer<T> > m_typedObjects;
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

    if (RiuTypedPdmObjects<RimEclipseCase>::containsTypedObjects(m_dragItems))
    {
        return Qt::CopyAction | Qt::MoveAction;
    }
    else if (RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects(m_dragItems))
    {
        return Qt::CopyAction;
    }

    return Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags RiuDragDrop::flags(const QModelIndex &index) const
{
    Qt::ItemFlags itemflags = 0;

    if (index.isValid())
    {
        caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();
        caf::PdmUiItem* uiItem = uiTreeView->uiItemFromModelIndex(index);

        caf::PdmObject* pdmObj = dynamic_cast<caf::PdmObject*>(uiItem);
        if (pdmObj)
        {
            RimWellAllocationPlot* wellAllocationPlot = nullptr;
            pdmObj->firstAncestorOrThisOfType(wellAllocationPlot);
            if (wellAllocationPlot) return itemflags;
        }

        if (dynamic_cast<RimEclipseCase*>(uiItem) ||
            dynamic_cast<RimWellLogCurve*>(uiItem) ||
            dynamic_cast<RimWellLogFileChannel*>(uiItem) || 
            dynamic_cast<RimWellLogTrack*>(uiItem) ||
            dynamic_cast<RimSummaryCase*>(uiItem))
        {
            // TODO: Remember to handle reservoir holding the main grid
            itemflags |= Qt::ItemIsDragEnabled;
        }

        if (dynamic_cast<RimEclipseCase*>(uiItem) || dynamic_cast<RimCaseCollection*>(uiItem))
        {
            if (RiuTypedPdmObjects<RimEclipseCase>::containsTypedObjects(m_dragItems))
            {
                itemflags |= Qt::ItemIsDropEnabled;
            }
        }
        else if (m_proposedDropAction == Qt::MoveAction)
        {
            if (dynamic_cast<RimWellLogPlot*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimWellLogTrack>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimWellLogTrack*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimWellLogCurve>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
                else if (RiuTypedPdmObjects<RimWellLogTrack>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimWellLogCurve*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimWellLogCurve>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimSummaryCase*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimSummaryCaseCollection*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimSummaryCaseMainCollection*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimSummaryCase>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
        }
        else if (m_proposedDropAction == Qt::CopyAction)
        {
            if (dynamic_cast<RimWellLogTrack*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects(m_dragItems))
                {
                    itemflags |= Qt::ItemIsDropEnabled;
                }
            }
            else if (dynamic_cast<RimWellLogCurve*>(uiItem))
            {
                if (RiuTypedPdmObjects<RimWellLogFileChannel>::containsTypedObjects(m_dragItems))
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
bool RiuDragDrop::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();
    caf::PdmUiItem* dropTargetUiItem = uiTreeView->uiItemFromModelIndex(parent);
    caf::PdmObjectHandle* dropTarget = dynamic_cast<caf::PdmObjectHandle*>(dropTargetUiItem);
    if (dropTarget)
    {
        caf::PdmObjectGroup draggedObjects;
        const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>(data);
        if (myMimeData && parent.isValid())
        {
            objectGroupFromModelIndexes(&draggedObjects, myMimeData->indexes());
        }
        else
        {
            return false;
        }

        RimIdenticalGridCaseGroup* gridCaseGroup;
        dropTarget->firstAncestorOrThisOfType(gridCaseGroup);
        if (gridCaseGroup)
        {
            return handleGridCaseGroupDrop(action, draggedObjects, gridCaseGroup);
        }

        RimWellLogCurve* wellLogPlotCurve;
        dropTarget->firstAncestorOrThisOfType(wellLogPlotCurve);
        if (wellLogPlotCurve)
        {
            return handleWellLogPlotCurveDrop(action, draggedObjects, wellLogPlotCurve);
        }

        RimWellLogTrack* wellLogPlotTrack;
        dropTarget->firstAncestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            return handleWellLogPlotTrackDrop(action, draggedObjects, wellLogPlotTrack);
        }

        RimWellLogPlot* wellLogPlot;
        dropTarget->firstAncestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            return handleWellLogPlotDrop(action, draggedObjects, wellLogPlot);
        }

        RimSummaryCaseCollection* summaryCaseCollection;
        dropTarget->firstAncestorOrThisOfType(summaryCaseCollection);
        if (summaryCaseCollection)
        {
            return handleSummaryCaseCollectionDrop(action, draggedObjects, summaryCaseCollection);
        }

        RimSummaryCaseMainCollection* summaryCaseMainCollection;
        dropTarget->firstAncestorOrThisOfType(summaryCaseMainCollection);
        if (summaryCaseMainCollection)
        {
            return handleSummaryCaseMainCollectionDrop(action, draggedObjects, summaryCaseMainCollection);
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMimeData* RiuDragDrop::mimeData(const QModelIndexList &indexes) const
{
    MimeDataWithIndexes* myObj = new MimeDataWithIndexes();
    myObj->setIndexes(indexes);
        
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
void RiuDragDrop::moveCasesToGridGroup(caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup)
{
    std::vector<RimEclipseCase*> casesToBeDeleted = RiuTypedPdmObjects<RimEclipseCase>::typedObjectsFromGroup(objectGroup);

    if (RicCloseCaseFeature::userConfirmedGridCaseGroupChange(casesToBeDeleted))
    {
        RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(objectGroup, gridCaseGroup);
    
        for (size_t i = 0; i < casesToBeDeleted.size(); i++)
        {
            RicCloseCaseFeature::deleteEclipseCase(casesToBeDeleted[i]);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleGridCaseGroupDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup)
{
    if (action == Qt::CopyAction)
    {
        RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(objectGroup, gridCaseGroup);
    }
    else if (action == Qt::MoveAction)
    {
        moveCasesToGridGroup(objectGroup, gridCaseGroup);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotTrackDrop(Qt::DropAction action, caf::PdmObjectGroup& draggedObjects, RimWellLogTrack* trackTarget)
{
    std::vector<RimWellLogFileChannel*> wellLogFileChannels = RiuTypedPdmObjects<RimWellLogFileChannel>::typedObjectsFromGroup(draggedObjects);
    if (wellLogFileChannels.size() > 0)
    {
        if (action == Qt::CopyAction)
        {
            RicNewWellLogFileCurveFeature::addWellLogChannelsToPlotTrack(trackTarget, wellLogFileChannels);
            return true;
        }
    }

    std::vector<RimWellLogCurve*> wellLogPlotCurves = RiuTypedPdmObjects<RimWellLogCurve>::typedObjectsFromGroup(draggedObjects);
    if (wellLogPlotCurves.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(trackTarget, wellLogPlotCurves, NULL);
            return true;
        }
    }

    std::vector<RimWellLogTrack*> wellLogPlotTracks = RiuTypedPdmObjects<RimWellLogTrack>::typedObjectsFromGroup(draggedObjects);
    if (wellLogPlotTracks.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RimWellLogPlot* wellLogPlot;
            trackTarget->firstAncestorOrThisOfType(wellLogPlot);
            RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot(wellLogPlot, wellLogPlotTracks, trackTarget); 
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotDrop(Qt::DropAction action, caf::PdmObjectGroup& draggedObjects, RimWellLogPlot* wellLogPlotTarget)
{
    std::vector<RimWellLogTrack*> wellLogPlotTracks = RiuTypedPdmObjects<RimWellLogTrack>::typedObjectsFromGroup(draggedObjects);
    if (wellLogPlotTracks.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot(wellLogPlotTarget, wellLogPlotTracks, NULL);
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotCurveDrop(Qt::DropAction action, caf::PdmObjectGroup& draggedObjects, RimWellLogCurve* curveDropTarget)
{
    std::vector<RimWellLogCurve*> wellLogPlotCurves = RiuTypedPdmObjects<RimWellLogCurve>::typedObjectsFromGroup(draggedObjects);
    if (wellLogPlotCurves.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RimWellLogTrack* wellLogPlotTrack;
            curveDropTarget->firstAncestorOrThisOfType(wellLogPlotTrack);

            RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(wellLogPlotTrack, wellLogPlotCurves, curveDropTarget);
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleSummaryCaseCollectionDrop(Qt::DropAction action, caf::PdmObjectGroup& draggedObjects, RimSummaryCaseCollection* summaryCaseDropTarget)
{
    std::vector<RimSummaryCase*> summaryCases = RiuTypedPdmObjects<RimSummaryCase>::typedObjectsFromGroup(draggedObjects);

    if (action != Qt::MoveAction || summaryCases.size() == 0) return false;

    for (RimSummaryCase* summaryCase : summaryCases)
    {
        RimSummaryCaseCollection* summaryCaseCollection;
        summaryCase->firstAncestorOrThisOfType(summaryCaseCollection);
        
        if (summaryCaseCollection)
        {
            summaryCaseCollection->removeCase(summaryCase);
            summaryCaseDropTarget->addCase(summaryCase);
            summaryCaseCollection->updateConnectedEditors();
            continue;
        }

        RimSummaryCaseMainCollection* summaryCaseMainCollection;
        summaryCase->firstAncestorOrThisOfType(summaryCaseMainCollection);
        if (summaryCaseMainCollection)
        {
            summaryCaseMainCollection->removeCase(summaryCase);
            summaryCaseDropTarget->addCase(summaryCase);
            summaryCaseMainCollection->updateConnectedEditors();
        }
    }
    summaryCaseDropTarget->updateConnectedEditors();
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleSummaryCaseMainCollectionDrop(Qt::DropAction action, caf::PdmObjectGroup& draggedObjects, RimSummaryCaseMainCollection* summaryCaseDropTarget)
{
    std::vector<RimSummaryCase*> summaryCases = RiuTypedPdmObjects<RimSummaryCase>::typedObjectsFromGroup(draggedObjects);
    
    if (action != Qt::MoveAction || summaryCases.size() == 0 ) return false;

    for (RimSummaryCase* summaryCase : summaryCases)
    {
        RimSummaryCaseCollection* summaryCaseCollection;
        summaryCase->firstAncestorOrThisOfType(summaryCaseCollection);

        if (summaryCaseCollection)
        {
            summaryCaseCollection->removeCase(summaryCase);
            summaryCaseDropTarget->addCase(summaryCase);
            summaryCaseCollection->updateConnectedEditors();
        }
    }

    summaryCaseDropTarget->updateConnectedEditors();
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::objectGroupFromModelIndexes(caf::PdmObjectGroup* objectGroup, const QModelIndexList& indexes)
{
    CVF_ASSERT(objectGroup);

    objectGroup->objects.clear();
    caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();

    for (int i = 0; i < indexes.size(); i++)
    {
        caf::PdmUiItem* uiItem = uiTreeView->uiItemFromModelIndex(indexes[i]);
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(uiItem);
        if (objHandle)
        {
            objectGroup->objects.push_back(objHandle);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmPointer<caf::PdmObjectHandle> > RiuDragDrop::objectHandlesFromSelection()
{
    std::vector<caf::PdmObjectHandle*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    std::vector<caf::PdmPointer<caf::PdmObjectHandle> > objectHandles;

    for (size_t sIdx = 0; sIdx < selection.size(); sIdx++)
    {
        objectHandles.push_back(selection[sIdx]);
    }

    return objectHandles;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuDragDrop::onProposedDropActionUpdated(Qt::DropAction action)
{
    m_proposedDropAction = action;
}
