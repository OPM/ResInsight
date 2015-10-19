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
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellLogPlotCurve.h"
#include "RimWellLogPlot.h"

#include "RimWellLogPlotTrack.h"
#include "RiuMainWindow.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmUiTreeView.h"

#include <QAbstractItemModel>
#include <QModelIndex>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
class RiuTypedObjectsFromObjectGroupGetter
{
public:
    RiuTypedObjectsFromObjectGroupGetter(const caf::PdmObjectGroup& objectGroup)
    {
        objectGroup.objectsByType(&m_typedObjects);
    }

    static std::vector<T*> typedObjectsFromGroup(const caf::PdmObjectGroup& objectGroup)
    {
        RiuTypedObjectsFromObjectGroupGetter<T> typedObjectsGetter(objectGroup);
        return typedObjectsGetter.typedObjects();
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
    return Qt::CopyAction | Qt::MoveAction;
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

        if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem) ||
            dynamic_cast<RimCaseCollection*>(uiItem) ||
            dynamic_cast<RimWellLogPlot*>(uiItem) ||
            dynamic_cast<RimWellLogPlotTrack*>(uiItem))
        {
            itemflags |= Qt::ItemIsDropEnabled;
        }

        if (dynamic_cast<RimEclipseCase*>(uiItem) ||
            dynamic_cast<RimWellLogPlotCurve*>(uiItem) ||
            dynamic_cast<RimWellLogPlotTrack*>(uiItem) ||
            dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            // TODO: Remember to handle reservoir holding the main grid
            itemflags |= Qt::ItemIsDragEnabled;
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
    caf::PdmUiItem* dropTarget = uiTreeView->uiItemFromModelIndex(parent);
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(dropTarget);
    if (objHandle)
    {
        caf::PdmObjectGroup objectGroup;
        const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>(data);
        if (myMimeData && parent.isValid())
        {
            for (int i = 0; i < myMimeData->indexes().size(); i++)
            {
                QModelIndex mi = myMimeData->indexes().at(i);
                caf::PdmUiItem* currentItem = uiTreeView->uiItemFromModelIndex(mi);
                caf::PdmObjectHandle* pdmObj = dynamic_cast<caf::PdmObjectHandle*>(currentItem);

                if (pdmObj)
                {
                    objectGroup.objects.push_back(pdmObj);
                }
            }
        }
        else
        {
            return false;
        }

        RimIdenticalGridCaseGroup* gridCaseGroup;
        objHandle->firstAnchestorOrThisOfType(gridCaseGroup);
        if (gridCaseGroup)
        {
            return handleGridCaseGroupDrop(action, objectGroup, gridCaseGroup);
        }

        RimWellLogPlotTrack* wellLogPlotTrack;
        objHandle->firstAnchestorOrThisOfType(wellLogPlotTrack);
        if (wellLogPlotTrack)
        {
            return handleWellLogPlotTrackDrop(action, objectGroup, wellLogPlotTrack);
        }

        RimWellLogPlot* wellLogPlot;
        objHandle->firstAnchestorOrThisOfType(wellLogPlot);
        if (wellLogPlot)
        {
            return handleWellLogPlotDrop(action, objectGroup, wellLogPlot);
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
void RiuDragDrop::moveCasesToGridGroup(caf::PdmObjectGroup& objectGroup, RimIdenticalGridCaseGroup* gridCaseGroup)
{
    std::vector<RimEclipseCase*> casesToBeDeleted = RiuTypedObjectsFromObjectGroupGetter<RimEclipseCase>::typedObjectsFromGroup(objectGroup);

    if (RicCloseCaseFeature::userConfirmedGridCaseGroupChange(casesToBeDeleted))
    {
        caf::RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(objectGroup, gridCaseGroup);
    
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
        caf::RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(objectGroup, gridCaseGroup);
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
bool RiuDragDrop::handleWellLogPlotTrackDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimWellLogPlotTrack* wellLogPlotTrack)
{
    std::vector<RimWellLogFileChannel*> wellLogFileChannels = RiuTypedObjectsFromObjectGroupGetter<RimWellLogFileChannel>::typedObjectsFromGroup(objectGroup);
    if (wellLogFileChannels.size() > 0)
    {
        if (action == Qt::CopyAction)
        {
            RicNewWellLogFileCurveFeature::addWellLogChannelsToPlotTrack(wellLogPlotTrack, wellLogFileChannels);
            return true;
        }
    }

    std::vector<RimWellLogPlotCurve*> wellLogPlotCurves = RiuTypedObjectsFromObjectGroupGetter<RimWellLogPlotCurve>::typedObjectsFromGroup(objectGroup);
    if (wellLogPlotCurves.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(wellLogPlotTrack, wellLogPlotCurves);
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::handleWellLogPlotDrop(Qt::DropAction action, caf::PdmObjectGroup& objectGroup, RimWellLogPlot* wellLogPlot)
{
    std::vector<RimWellLogPlotTrack*> wellLogPlotTracks = RiuTypedObjectsFromObjectGroupGetter<RimWellLogPlotTrack>::typedObjectsFromGroup(objectGroup);
    if (wellLogPlotTracks.size() > 0)
    {
        if (action == Qt::MoveAction)
        {
            RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot(wellLogPlot, wellLogPlotTracks);
            return true;
        }
    }

    return false;
}

