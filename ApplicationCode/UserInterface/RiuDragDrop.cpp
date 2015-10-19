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

#include "RimWellLogPlotTrack.h"
#include "RiuMainWindow.h"

#include "cafPdmObjectGroup.h"
#include "cafPdmUiTreeView.h"

#include <QAbstractItemModel>
#include <QModelIndex>

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
    if (index.isValid())
    {
        caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();
        caf::PdmUiItem* uiItem = uiTreeView->uiItemFromModelIndex(index);

        if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem) ||
            dynamic_cast<RimCaseCollection*>(uiItem) ||
            dynamic_cast<RimWellLogPlotTrack*>(uiItem))
        {
            return Qt::ItemIsDropEnabled;
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem) ||
            dynamic_cast<RimWellLogPlotCurve*>(uiItem) ||
            dynamic_cast<RimWellLogFileChannel*>(uiItem))
        {
            // TODO: Remember to handle reservoir holding the main grid
            return Qt::ItemIsDragEnabled;
        }
    }

    Qt::ItemFlags itemflags;
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
    std::vector<caf::PdmPointer<RimEclipseResultCase> > typedObjects;
    objectGroup.objectsByType(&typedObjects);

    std::vector<RimEclipseCase*> casesToBeDeleted;
    for (size_t i = 0; i < typedObjects.size(); i++)
    {
        RimEclipseCase* eclipseCase = typedObjects[i];
        casesToBeDeleted.push_back(eclipseCase);
    }

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
    {
        std::vector<caf::PdmPointer<RimWellLogFileChannel> > typedObjects;
        objectGroup.objectsByType(&typedObjects);
        if (typedObjects.size() > 0)
        {
            std::vector<RimWellLogFileChannel*> wellLogFileChannels;
            for (size_t cIdx = 0; cIdx < typedObjects.size(); cIdx++)
            {
                wellLogFileChannels.push_back(typedObjects[cIdx]);
            }

            if (wellLogFileChannels.size() > 0)
            {
                if (action == Qt::CopyAction)
                {
                    RicNewWellLogFileCurveFeature::addWellLogChannelsToPlotTrack(wellLogPlotTrack, wellLogFileChannels);
                    return true;
                }
            }
        }
    }

    {
        std::vector<caf::PdmPointer<RimWellLogPlotCurve> > typedObjects;
        objectGroup.objectsByType(&typedObjects);
        if (typedObjects.size() > 0)
        {
            std::vector<RimWellLogPlotCurve*> wellLogPlotCurves;
            for (size_t cIdx = 0; cIdx < typedObjects.size(); cIdx++)
            {
                wellLogPlotCurves.push_back(typedObjects[cIdx]);
            }

            if (wellLogPlotCurves.size() > 0)
            {
                if (action == Qt::CopyAction)
                {
                    RicWellLogPlotTrackFeatureImpl::moveCurvesToWellLogPlotTrack(wellLogPlotTrack, wellLogPlotCurves);
                    return true;
                }
            }
        }

    }

    return false;
}

