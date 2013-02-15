/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RIStdInclude.h"

#include "RimUiTreeModelPdm.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafPdmObject.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"

#include "RimReservoirView.h"
#include "RIViewer.h"
#include "RimCalcScript.h"
#include "RIApplication.h"
#include "RIMainWindow.h"
#include "RimInputProperty.h"
#include "RimInputPropertyCollection.h"
#include "cafPdmField.h"
#include "RimInputReservoir.h"
#include "RimStatisticalCalculation.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimUiTreeModelPdm::RimUiTreeModelPdm(QObject* parent)
    : caf::UiTreeModelPdm(parent)
{
    m_scriptChangeDetector = new QFileSystemWatcher(this);
    this->updateScriptPaths();
    connect(m_scriptChangeDetector, SIGNAL(directoryChanged(QString)), this, SLOT(slotRefreshScriptTree(QString)));
    connect(m_scriptChangeDetector, SIGNAL(fileChanged(QString)), this, SLOT(slotRefreshScriptTree(QString)));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::insertRows(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
{
    caf::PdmUiTreeItem* parentItem = getTreeItemFromIndex(parent);
    
    bool canCreateChildren = false;
    QModelIndex parentIndex = parent;

    if (dynamic_cast<RimCellRangeFilterCollection*>(parentItem->dataObject().p()) ||
        dynamic_cast<RimCellPropertyFilterCollection*>(parentItem->dataObject().p()))
    {
        canCreateChildren = true;
    }
    else if (dynamic_cast<RimCellFilter*>(parentItem->dataObject().p()))
    {
        parentItem = parentItem->parent();
        parentIndex = parent.parent();

        canCreateChildren = true;
    }

    if (canCreateChildren)
    {
        beginInsertRows(parent, position, position + rows - 1);
        
        int i;
        for (i = 0; i < rows; i++)
        {
            if (dynamic_cast<RimCellRangeFilterCollection*>(parentItem->dataObject().p()))
            {
                RimCellRangeFilterCollection* rangeFilterCollection = dynamic_cast<RimCellRangeFilterCollection*>(parentItem->dataObject().p());
                
                RimCellRangeFilter* rangeFilter = rangeFilterCollection->createAndAppendRangeFilter();

                caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(parentItem, position + i, rangeFilter);
            }
            else if (dynamic_cast<RimCellPropertyFilterCollection*>(parentItem->dataObject().p()))
            {
                RimCellPropertyFilterCollection* propertyFilterCollection = dynamic_cast<RimCellPropertyFilterCollection*>(parentItem->dataObject().p());

                RimCellPropertyFilter* propertyFilter = propertyFilterCollection->createAndAppendPropertyFilter();

                caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(parentItem, position + i, propertyFilter);
            }

        }
        endInsertRows();
    }

    return canCreateChildren;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::deletePropertyFilter(const QModelIndex& itemIndex)
{
    CVF_ASSERT(itemIndex.isValid());

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(uiItem);

    RimCellPropertyFilter* propertyFilter = dynamic_cast<RimCellPropertyFilter*>(uiItem->dataObject().p());
    CVF_ASSERT(propertyFilter);

    RimCellPropertyFilterCollection* propertyFilterCollection = propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    bool wasFilterActive = propertyFilter->active();
    bool wasSomeFilterActive = propertyFilterCollection->hasActiveFilters();

    // Remove Ui items pointing at the pdm object to delete
    removeRow(itemIndex.row(), itemIndex.parent());

    propertyFilterCollection->remove(propertyFilter);
    delete propertyFilter;

    if (wasFilterActive)
    {
        propertyFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::PROPERTY_FILTERED);
    }

    if (wasSomeFilterActive)
    {
        propertyFilterCollection->reservoirView()->createDisplayModelAndRedraw();
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::deleteRangeFilter(const QModelIndex& itemIndex)
{
    CVF_ASSERT(itemIndex.isValid());

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(uiItem);

    RimCellRangeFilter* rangeFilter = dynamic_cast<RimCellRangeFilter*>(uiItem->dataObject().p());
    CVF_ASSERT(rangeFilter);

    RimCellRangeFilterCollection* rangeFilterCollection = rangeFilter->parentContainer();
    CVF_ASSERT(rangeFilterCollection);

    bool wasFilterActive = rangeFilter->active();
    bool wasSomeFilterActive = rangeFilterCollection->hasActiveFilters();

    // Remove Ui items pointing at the pdm object to delete
    removeRow(itemIndex.row(), itemIndex.parent());

    rangeFilterCollection->remove(rangeFilter);
    delete rangeFilter;

    if (wasFilterActive)
    {
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::PROPERTY_FILTERED);
    }

    if (wasSomeFilterActive)
    {
        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();
    }    

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::deleteReservoirView(const QModelIndex& itemIndex)
{
    CVF_ASSERT(itemIndex.isValid());

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(uiItem);

    RimReservoirView* reservoirView = dynamic_cast<RimReservoirView*>(uiItem->dataObject().p());
    CVF_ASSERT(reservoirView);

    // Remove Ui items pointing at the pdm object to delete
    removeRow(itemIndex.row(), itemIndex.parent());

    reservoirView->eclipseCase()->removeReservoirView(reservoirView);
    delete reservoirView;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteReservoir(const QModelIndex& itemIndex)
{
    CVF_ASSERT(itemIndex.isValid());

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(uiItem);

    RimReservoir* reservoir = dynamic_cast<RimReservoir*>(uiItem->dataObject().p());
    CVF_ASSERT(reservoir);

    // Remove Ui items pointing at the pdm object to delete
    removeRow(itemIndex.row(), itemIndex.parent());

    RimProject* proj = RIApplication::instance()->project();
    proj->reservoirs().removeChildObject(reservoir);

    delete reservoir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellPropertyFilter* RimUiTreeModelPdm::addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimCellPropertyFilterCollection* propertyFilterCollection = NULL;
    caf::PdmUiTreeItem* propertyFilterCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimCellPropertyFilter*>(currentItem->dataObject().p()))
    {
        RimCellPropertyFilter* propertyFilter = dynamic_cast<RimCellPropertyFilter*>(currentItem->dataObject().p());
        propertyFilterCollection = propertyFilter->parentContainer();
        propertyFilterCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimCellPropertyFilterCollection*>(currentItem->dataObject().p()))
    {
        propertyFilterCollection = dynamic_cast<RimCellPropertyFilterCollection*>(currentItem->dataObject().p());
        propertyFilterCollectionItem = currentItem;
        position = propertyFilterCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    beginInsertRows(collectionIndex, position, position);
    
    RimCellPropertyFilter* propertyFilter = propertyFilterCollection->createAndAppendPropertyFilter();
    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(propertyFilterCollectionItem, position, propertyFilter);

    endInsertRows();

    insertedModelIndex = index(position, 0, collectionIndex);

    if (propertyFilterCollection)
    {
        propertyFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::PROPERTY_FILTERED);
    }

    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilter* RimUiTreeModelPdm::addRangeFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimCellRangeFilterCollection* rangeFilterCollection = NULL;
    caf::PdmUiTreeItem* rangeFilterCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimCellRangeFilter*>(currentItem->dataObject().p()))
    {
        RimCellRangeFilter* rangeFilter = dynamic_cast<RimCellRangeFilter*>(currentItem->dataObject().p());
        rangeFilterCollection = rangeFilter->parentContainer();
        rangeFilterCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimCellRangeFilterCollection*>(currentItem->dataObject().p()))
    {
        rangeFilterCollection = dynamic_cast<RimCellRangeFilterCollection*>(currentItem->dataObject().p());
        rangeFilterCollectionItem = currentItem;
        position = rangeFilterCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    beginInsertRows(collectionIndex, position, position);

    RimCellRangeFilter* rangeFilter = rangeFilterCollection->createAndAppendRangeFilter();
    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(rangeFilterCollectionItem, position, rangeFilter);

    endInsertRows();

    insertedModelIndex = index(position, 0, collectionIndex);
    if (rangeFilterCollection)
    {
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

    }
    return rangeFilter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimUiTreeModelPdm::addReservoirView(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);
    if (!currentItem) return NULL;

    caf::PdmUiTreeItem* collectionItem = NULL;

    if (dynamic_cast<RimReservoirView*>(currentItem->dataObject().p()))
    {
        collectionItem = currentItem->parent();
    }
    else if (dynamic_cast<RimReservoir*>(currentItem->dataObject().p()))
    {
        collectionItem = currentItem;
    }

    if (collectionItem)
    {
        RimReservoir* rimReservoir = dynamic_cast<RimReservoir*>(collectionItem->dataObject().p());
        RimReservoirView* insertedView = rimReservoir->createAndAddReservoirView();

        int viewCount = rowCount(itemIndex);
        beginInsertRows(itemIndex, viewCount, viewCount);

        caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(collectionItem, viewCount, insertedView);

        endInsertRows();

        insertedView->loadDataAndUpdate();

        rebuildUiSubTree(insertedView);

        return insertedView;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::updateScriptPaths()
{
    RimProject* proj = RIApplication::instance()->project();

    if (!proj || !proj->scriptCollection()) return;

    QStringList paths;

    proj->scriptCollection()->pathsAndSubPaths(paths);

    if (m_scriptChangeDetector->directories().size()) m_scriptChangeDetector->removePaths( m_scriptChangeDetector->directories());
    if (m_scriptChangeDetector->files().size()) m_scriptChangeDetector->removePaths( m_scriptChangeDetector->files());

    if (paths.size()) m_scriptChangeDetector->addPaths(paths);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::slotRefreshScriptTree(QString path)
{
    RimProject* proj = RIApplication::instance()->project();

    if (!proj || !proj->scriptCollection()) return;

    RimScriptCollection* changedSColl = proj->scriptCollection()->findScriptCollection(path);
    if (changedSColl)
    {
        changedSColl->readContentFromDisc();
        this->rebuildUiSubTree(changedSColl);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::addInputProperty(const QModelIndex& itemIndex, const QStringList& fileNames)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    RimInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimInputPropertyCollection*>(currentItem->dataObject().p());
    CVF_ASSERT(inputPropertyCollection);
    
    std::vector<caf::PdmObject*> parentObjects;
    inputPropertyCollection->parentObjects(parentObjects);


    CVF_ASSERT(parentObjects.size() == 1);

    RimInputReservoir* inputReservoir = dynamic_cast<RimInputReservoir*>(parentObjects[0]);
    CVF_ASSERT(inputReservoir);
    if (inputReservoir)
    {
        inputReservoir->openDataFileSet(fileNames);
    }

    this->rebuildUiSubTree(inputPropertyCollection);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteInputProperty(const QModelIndex& itemIndex)
{
    if (!itemIndex.isValid()) return;

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    if (!uiItem) return;

    caf::PdmObject* object = uiItem->dataObject().p();
    RimInputProperty* inputProperty = dynamic_cast<RimInputProperty*>(object);
    if (!inputProperty) return;

    // Remove item from UI tree model before delete of project data structure
    removeRow(itemIndex.row(), itemIndex.parent());

    std::vector<caf::PdmObject*> parentObjects;
    object->parentObjects(parentObjects);
    CVF_ASSERT(parentObjects.size() == 1);

    RimInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimInputPropertyCollection*>(parentObjects[0]);
    if (!inputPropertyCollection) return;

    std::vector<caf::PdmObject*> parentObjects2;
    inputPropertyCollection->parentObjects(parentObjects2);
    CVF_ASSERT(parentObjects2.size() == 1);

    RimInputReservoir* inputReservoir = dynamic_cast<RimInputReservoir*>(parentObjects2[0]);
    if (!inputReservoir) return;

    inputReservoir->removeProperty(inputProperty);

    delete inputProperty;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticalCalculation* RimUiTreeModelPdm::addStatisticalCalculation(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimIdenticalGridCaseGroup* caseGroup = NULL;
    caf::PdmUiTreeItem* parentCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimStatisticalCalculation*>(currentItem->dataObject().p()))
    {
        RimStatisticalCalculation* currentObject = dynamic_cast<RimStatisticalCalculation*>(currentItem->dataObject().p());
        caseGroup = currentObject->parent();
        parentCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p()))
    {
        caseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p());
        parentCollectionItem = currentItem;
        position = parentCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    beginInsertRows(collectionIndex, position, position);

    RimStatisticalCalculation* createdObject = caseGroup->createAndAppendStatisticalCalculation();
    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(parentCollectionItem, position, createdObject);

    endInsertRows();

    insertedModelIndex = index(position, 0, collectionIndex);

    return createdObject;
}

