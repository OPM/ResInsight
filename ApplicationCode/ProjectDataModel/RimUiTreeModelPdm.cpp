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

#include "RiaStdInclude.h"

#include "RimUiTreeModelPdm.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"

#include "cafPdmObject.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"

#include "RimReservoirView.h"
#include "RiuViewer.h"
#include "RimCalcScript.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RimInputProperty.h"
#include "RimInputPropertyCollection.h"
#include "cafPdmField.h"
#include "RimInputCase.h"
#include "RimStatisticsCase.h"
#include "RimResultCase.h"
#include "RigGridManager.h"
#include "RimCase.h"
#include "RigCaseData.h"

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
/// TO BE DELETED
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::insertRows_special(int position, int rows, const QModelIndex &parent /*= QModelIndex()*/)
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
    removeRows_special(itemIndex.row(), 1, itemIndex.parent());

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

    clearClipboard();

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
    removeRows_special(itemIndex.row(), 1, itemIndex.parent());

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

    clearClipboard();

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
    removeRows_special(itemIndex.row(), 1, itemIndex.parent());

    reservoirView->eclipseCase()->removeReservoirView(reservoirView);
    delete reservoirView;

    clearClipboard();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteReservoir(RimCase* reservoir)
{
    if (reservoir->parentCaseCollection())
    {
    RimCaseCollection* caseCollection = reservoir->parentCaseCollection();
    QModelIndex caseCollectionModelIndex = getModelIndexFromPdmObject(caseCollection);
    if (!caseCollectionModelIndex.isValid()) return;

    QModelIndex mi = getModelIndexFromPdmObjectRecursive(caseCollectionModelIndex, reservoir);
    if (mi.isValid())
    {
        caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(mi);
        CVF_ASSERT(uiItem);

        // Remove Ui items pointing at the pdm object to delete
        removeRows_special(mi.row(), 1, mi.parent());
    }

    if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollection))
    {
        RimIdenticalGridCaseGroup* caseGroup = caseCollection->parentCaseGroup();
        CVF_ASSERT(caseGroup);

        caseGroup->statisticsCaseCollection()->reservoirs.removeChildObject(reservoir);
    }
    else
    {
        RimProject* proj = RiaApplication::instance()->project();
        proj->removeCaseFromAllGroups(reservoir);
    }
    }
    else
    {
        RimProject* proj = RiaApplication::instance()->project();
        QModelIndex mi = getModelIndexFromPdmObject(reservoir);
        if (mi.isValid())
        {
            caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(mi);
            CVF_ASSERT(uiItem);

            // Remove Ui items pointing at the pdm object to delete
            removeRows_special(mi.row(), 1, mi.parent());
        }

        proj->removeCaseFromAllGroups(reservoir);
    }

    delete reservoir;

    clearClipboard();
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

    bool itemIndexIsCollection = false;
    QModelIndex collectionIndex;
    if (dynamic_cast<RimReservoirView*>(currentItem->dataObject().p()))
    {
        collectionItem = currentItem->parent();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimCase*>(currentItem->dataObject().p()))
    {
        collectionItem = currentItem;
        collectionIndex = itemIndex;
    }

    if (collectionItem)
    {
        RimCase* rimReservoir = dynamic_cast<RimCase*>(collectionItem->dataObject().p());

        // If the case is one of the source cases in a CaseGroup, but not the main case, we need to 
        // trigger a complete load of the case, if the new view is the first on the case.

        if (rimReservoir && rimReservoir->parentGridCaseGroup() 
            && rimReservoir->parentGridCaseGroup()->contains(rimReservoir) 
            && !(rimReservoir ==  rimReservoir->parentGridCaseGroup()->mainCase())
            && rimReservoir->reservoirViews().size() == 0)
        {
            if (rimReservoir->reservoirData())
            {
                CVF_ASSERT(rimReservoir->reservoirData()->refCount() == 1);
            }

            rimReservoir->removeReservoirData();
        }

        RimReservoirView* insertedView = rimReservoir->createAndAddReservoirView();

        // Must be run before buildViewItems, as wells are created in this function
        insertedView->loadDataAndUpdate(); 

        int viewCount = rowCount(collectionIndex);
        beginInsertRows(collectionIndex, viewCount, viewCount);

        // NOTE: -1 as second argument indicates append
        caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(collectionItem, -1, insertedView);
        
        endInsertRows();

        return insertedView;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::updateScriptPaths()
{
    RimProject* proj = RiaApplication::instance()->project();

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
    RimProject* proj = RiaApplication::instance()->project();

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
    
    std::vector<RimInputCase*> parentObjects;
    inputPropertyCollection->parentObjectsOfType(parentObjects);
    CVF_ASSERT(parentObjects.size() == 1);

    RimInputCase* inputReservoir = parentObjects[0];
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
    removeRows_special(itemIndex.row(), 1, itemIndex.parent());

    std::vector<RimInputPropertyCollection*> parentObjects;
    object->parentObjectsOfType(parentObjects);
    CVF_ASSERT(parentObjects.size() == 1);

    RimInputPropertyCollection* inputPropertyCollection = parentObjects[0];
    if (!inputPropertyCollection) return;

    std::vector<RimInputCase*> parentObjects2;
    inputPropertyCollection->parentObjectsOfType(parentObjects2);
    CVF_ASSERT(parentObjects2.size() == 1);

    RimInputCase* inputReservoir = parentObjects2[0];
    if (!inputReservoir) return;

    inputReservoir->removeProperty(inputProperty);

    delete inputProperty;

    clearClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCase* RimUiTreeModelPdm::addStatisticalCalculation(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimIdenticalGridCaseGroup* caseGroup = NULL;
    caf::PdmUiTreeItem* parentCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimStatisticsCase*>(currentItem->dataObject().p()))
    {
        RimStatisticsCase* currentObject = dynamic_cast<RimStatisticsCase*>(currentItem->dataObject().p());
        caseGroup = currentObject->parentStatisticsCaseCollection()->parentCaseGroup();
        parentCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimCaseCollection*>(currentItem->dataObject().p()))
    {
        RimCaseCollection* statColl = dynamic_cast<RimCaseCollection*>(currentItem->dataObject().p());
        caseGroup = statColl->parentCaseGroup();
        parentCollectionItem = currentItem;
        position = parentCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    if (parentCollectionItem && caseGroup)
    {
        beginInsertRows(collectionIndex, position, position);

        RimStatisticsCase* createdObject = caseGroup->createAndAppendStatisticsCase();
        caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(parentCollectionItem, position, createdObject);

        endInsertRows();

        insertedModelIndex = index(position, 0, collectionIndex);

        return createdObject;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimUiTreeModelPdm::addCaseGroup(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);
    if (currentItem)
    {
        if (dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p()) ||
            dynamic_cast<RimCase*>(currentItem->dataObject().p()))
        {
            QModelIndex rootIndex = itemIndex.parent();
            caf::PdmUiTreeItem* rootTreeItem = currentItem->parent();

            // New case group is inserted before the last item, the script item
            int position = rootTreeItem->childCount() - 1;

            beginInsertRows(rootIndex, position, position);

            RimIdenticalGridCaseGroup* createdObject = new RimIdenticalGridCaseGroup;
            proj->caseGroups().push_back(createdObject);

            caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(rootTreeItem, position, createdObject);
            endInsertRows();

            insertedModelIndex = index(position, 0, rootIndex);

            return createdObject;
        }
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::addObjects(const QModelIndex& itemIndex, caf::PdmObjectGroup& pdmObjects)
{
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimIdenticalGridCaseGroup* gridCaseGroup = gridCaseGroupFromItemIndex(itemIndex);
    if (gridCaseGroup)
    {
        std::vector<caf::PdmPointer<RimResultCase> > typedObjects;
        pdmObjects.createCopyByType(&typedObjects);

        if (typedObjects.size() == 0)
    {
        return;
    }

        RimResultCase* mainResultCase = NULL;
        std::vector< std::vector<int> > mainCaseGridDimensions;

        // Read out main grid and main grid dimensions if present in case group
        if (gridCaseGroup->mainCase())
    {
            mainResultCase = dynamic_cast<RimResultCase*>(gridCaseGroup->mainCase());
            CVF_ASSERT(mainResultCase);

            mainResultCase->readGridDimensions(mainCaseGridDimensions);
        }

        // Add cases to case group
        for (size_t i = 0; i < typedObjects.size(); i++)
        {
            RimResultCase* rimResultReservoir = typedObjects[i];

            if (gridCaseGroup->contains(rimResultReservoir))
            {
                continue;
            }

            if (!mainResultCase)
            {
                rimResultReservoir->openEclipseGridFile();
                rimResultReservoir->readGridDimensions(mainCaseGridDimensions);

                mainResultCase = rimResultReservoir;
            }
            else
            {
                std::vector< std::vector<int> > caseGridDimensions;
                rimResultReservoir->readGridDimensions(caseGridDimensions);
                
                bool identicalGrid = RigGridManager::isGridDimensionsEqual(mainCaseGridDimensions, caseGridDimensions);
                if (!identicalGrid)
                {
                    continue;
                }

                if (!rimResultReservoir->openAndReadActiveCellData(mainResultCase->reservoirData()))
                {
                    CVF_ASSERT(false);
                }
            }

            proj->insertCaseInCaseGroup(gridCaseGroup, rimResultReservoir);

            caf::PdmObjectGroup::initAfterReadTraversal(rimResultReservoir);

            {
                QModelIndex rootIndex = getModelIndexFromPdmObject(gridCaseGroup->caseCollection());
                caf::PdmUiTreeItem* caseCollectionUiItem = getTreeItemFromIndex(rootIndex);

                int position = rowCount(rootIndex);
                beginInsertRows(rootIndex, position, position);
                caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(caseCollectionUiItem, -1, rimResultReservoir);
                endInsertRows();
            }

            for (size_t i = 0; i < rimResultReservoir->reservoirViews.size(); i++)
            {
                RimReservoirView* riv = rimResultReservoir->reservoirViews()[i];
                riv->loadDataAndUpdate();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::moveObjects(const QModelIndex& itemIndex, caf::PdmObjectGroup& pdmObjects)
{
    addObjects(itemIndex, pdmObjects);

    // Delete objects from original container
    std::vector<caf::PdmPointer<RimResultCase> > typedObjects;
    pdmObjects.objectsByType(&typedObjects);

    for (size_t i = 0; i < typedObjects.size(); i++)
    {
        RimCase* rimReservoir = typedObjects[i];
        deleteReservoir(rimReservoir);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::deleteObjectFromPdmPointersField(const QModelIndex& itemIndex)
{
    if (!itemIndex.isValid())
    {
        return false;
    }

    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(currentItem);

    caf::PdmObject* currentPdmObject = currentItem->dataObject().p();
    CVF_ASSERT(currentPdmObject);

    std::vector<caf::PdmFieldHandle*> parentFields;
    currentPdmObject->parentFields(parentFields);

    if (parentFields.size() == 1)
    {
        beginRemoveRows(itemIndex.parent(), itemIndex.row(), itemIndex.row());
        if (currentItem->parent())
        {
            currentItem->parent()->removeChildren(itemIndex.row(), 1);
        }
        endRemoveRows();

        caf::PdmPointersField<RimIdenticalGridCaseGroup*>* caseGroup = dynamic_cast<caf::PdmPointersField<RimIdenticalGridCaseGroup*> *>(parentFields[0]);
        if (caseGroup)
        {
            caseGroup->removeChildObject(currentPdmObject);

            delete currentPdmObject;
        }
    }

    clearClipboard();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::clearClipboard()
{
    // We use QModelIndex to identify a selection on the clipboard
    // When we delete or move an entity, the clipboard data might be invalid

    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard)
    {
        if (dynamic_cast<const MimeDataWithIndexes*>(clipboard->mimeData()))
        {
            clipboard->clear();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::DropActions RimUiTreeModelPdm::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags RimUiTreeModelPdm::flags(const QModelIndex &index) const
{
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
        else if (dynamic_cast<RimCase*>(currentItem->dataObject().p()))
        {
            // TODO: Remember to handle reservoir holding the main grid
            return Qt::ItemIsDragEnabled | defaultFlags;
        }
    }

    return defaultFlags;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeModelPdm::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>(data);
    if (myMimeData && parent.isValid())
    {
        caf::PdmObjectGroup pog;

        for (int i = 0; i < myMimeData->indexes().size(); i++)
        {
            QModelIndex mi = myMimeData->indexes().at(i);
            caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(mi);
            caf::PdmObject* pdmObj = currentItem->dataObject().p();

            pog.objects().push_back(pdmObj);
        }

        if (action == Qt::CopyAction)
        {
        addObjects(parent, pog);
        }
        else if (action == Qt::MoveAction)
        {
            moveObjects(parent, pog);
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMimeData* RimUiTreeModelPdm::mimeData(const QModelIndexList &indexes) const
{
    MimeDataWithIndexes* myObj = new MimeDataWithIndexes();
    myObj->setIndexes(indexes);
    return myObj;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimUiTreeModelPdm::mimeTypes() const
{
    QStringList types;
    types << MimeDataWithIndexes::formatName();
    return types;
}

//--------------------------------------------------------------------------------------------------
/// Return grid case group when QModelIndex points to grid case group, case collection or case in a grid case group
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimUiTreeModelPdm::gridCaseGroupFromItemIndex(const QModelIndex& itemIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    RimIdenticalGridCaseGroup* gridCaseGroup = NULL;

    if (dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p()))
    {
        gridCaseGroup = dynamic_cast<RimIdenticalGridCaseGroup*>(currentItem->dataObject().p());
    }
    else if (dynamic_cast<RimCaseCollection*>(currentItem->dataObject().p()))
    {
        RimCaseCollection* caseCollection = dynamic_cast<RimCaseCollection*>(currentItem->dataObject().p());
        CVF_ASSERT(caseCollection);

        gridCaseGroup = caseCollection->parentCaseGroup();
    }
    else if (dynamic_cast<RimCase*>(currentItem->dataObject().p()))
    {
        RimCase* rimReservoir = dynamic_cast<RimCase*>(currentItem->dataObject().p());
        CVF_ASSERT(rimReservoir);

        RimCaseCollection* caseCollection = rimReservoir->parentCaseCollection();
        if (caseCollection)
        {
            gridCaseGroup = caseCollection->parentCaseGroup();
        }
    }

    return gridCaseGroup;
}

