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

#include "cafPdmObjectGroup.h"
#include "cafPdmUiTreeView.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimMimeData.h"
#include "RiuMainWindow.h"

#include "OperationsUsingObjReferences/RicPasteEclipseCasesFeature.h"
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
            dynamic_cast<RimCaseCollection*>(uiItem))
        {
            return Qt::ItemIsDropEnabled;
        }
        else if (dynamic_cast<RimEclipseCase*>(uiItem))
        {
            // TODO: Remember to handle reservoir holding the main grid
            return Qt::ItemIsDragEnabled;
        }
    }

    Qt::ItemFlags itemflags;
    return itemflags;
/*
    Qt::ItemFlags defaultFlags = caf::UiTreeModelPdm::flags(index);
    if (index.isValid())
    {
        caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(index);
        CVF_ASSERT(currentItem);

        if (dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p()) ||
            dynamic_cast<RimCaseCollection*>(currentItem->dataObject().p()))
        {
            return Qt::ItemIsDropEnabled | defaultFlags;
        }
        else if (dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p()))
        {
            // TODO: Remember to handle reservoir holding the main grid
            return Qt::ItemIsDragEnabled | defaultFlags;
        }
    }

    return itemFlags;
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuDragDrop::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    caf::PdmUiTreeView* uiTreeView = RiuMainWindow::instance()->projectTreeView();
    RimIdenticalGridCaseGroup* gridCaseGroup = NULL;
    
    {
        caf::PdmUiItem* dropTarget = uiTreeView->uiItemFromModelIndex(parent);
        caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(dropTarget);
        if (objHandle)
        {
            objHandle->firstAnchestorOrThisOfType(gridCaseGroup);
        }
    }

    if (!gridCaseGroup) return false;

    const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>(data);
    if (myMimeData && parent.isValid())
    {
        caf::PdmObjectGroup pog;

        for (int i = 0; i < myMimeData->indexes().size(); i++)
        {
            QModelIndex mi = myMimeData->indexes().at(i);
            caf::PdmUiItem* currentItem = uiTreeView->uiItemFromModelIndex(mi);
            caf::PdmObjectHandle* pdmObj = dynamic_cast<caf::PdmObjectHandle*>(currentItem);

            if (pdmObj)
            {
                pog.objects.push_back(pdmObj);
            }
        }

        if (action == Qt::CopyAction)
        {
            caf::RicPasteEclipseCasesFeature::addCasesToGridCaseGroup(pog, gridCaseGroup);

            //addObjects(parent, pog);
        }
        else if (action == Qt::MoveAction)
        {
            assert(false);
            //moveObjects(parent, pog);
        }

        return true;
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
