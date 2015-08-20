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

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMimeData.h"

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
        }
        else if (action == Qt::MoveAction)
        {
            moveCasesToGridGroup(pog, gridCaseGroup);
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

