/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimUiTreeModelPdm.h"

#include "RiaApplication.h"
#include "RigGridManager.h"
#include "RimAnalysisModels.h"
#include "RimEclipseCase.h"
#include "RimCaseCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimMimeData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimEclipseView.h"
#include "RimResultCase.h"
#include "RimScriptCollection.h"
#include "RimStatisticsCase.h"
#include "RimUiTreeView.h"
#include "RimWellCollection.h"
#include "RimWellPathCollection.h"
#include "RimGeoMechView.h"

#include "cvfAssert.h"

#include <QClipboard>
#include <QFileSystemWatcher>
#include "RimGeoMechCase.h"



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
bool RimUiTreeModelPdm::deletePropertyFilter(const QModelIndex& itemIndex)
{
    CVF_ASSERT(itemIndex.isValid());

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    CVF_ASSERT(uiItem);

    RimEclipsePropertyFilter* propertyFilter = dynamic_cast<RimEclipsePropertyFilter*>(uiItem->dataObject().p());
    CVF_ASSERT(propertyFilter);

    RimEclipsePropertyFilterCollection* propertyFilterCollection = propertyFilter->parentContainer();
    CVF_ASSERT(propertyFilterCollection);

    bool wasFilterActive = propertyFilter->isActive();
    bool wasSomeFilterActive = propertyFilterCollection->hasActiveFilters();

    // Remove Ui items pointing at the pdm object to delete
    removeRows_special(itemIndex.row(), 1, itemIndex.parent()); // To be deleted

    propertyFilterCollection->remove(propertyFilter);
    delete propertyFilter;

    // updateUiSubTree(propertyFilterCollection); // To be enabled

    if (wasFilterActive)
    {
        propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
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

    bool wasFilterActive = rangeFilter->isActive();
    bool wasSomeFilterActive = rangeFilterCollection->hasActiveFilters();

    // Remove Ui items pointing at the pdm object to delete
    removeRows_special(itemIndex.row(), 1, itemIndex.parent()); // To be deleted

    rangeFilterCollection->remove(rangeFilter);
    delete rangeFilter;

    // updateUiSubTree(rangeFilterCollection); // To be enabled

    if (wasFilterActive)
    {
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
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
void RimUiTreeModelPdm::deleteReservoirViews(const std::vector<caf::PdmUiItem*>& treeSelection)
{
    std::set<RimCase*> ownerCases;

    for (size_t sIdx = 0; sIdx < treeSelection.size(); ++sIdx)
    {
        RimView* reservoirView = dynamic_cast<RimView*>(treeSelection[sIdx]);
        ownerCases.insert(reservoirView->ownerCase());

        reservoirView->removeFromParentFields();
        delete reservoirView;
        
    }

    for (std::set<RimCase*>::iterator it = ownerCases.begin(); it != ownerCases.end(); ++it)
    {
        updateUiSubTree(*it); 
    }

    clearClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteGeoMechCases(const std::vector<caf::PdmUiItem*>& treeSelection)
{
    std::set<caf::PdmObject*> allParents;
    
    for (size_t sIdx = 0; sIdx < treeSelection.size(); ++sIdx)
    {
        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(treeSelection[sIdx]);
        if (!geomCase) continue;

        std::vector<caf::PdmObject*> parents;
        geomCase->parentObjects(parents);
        for (size_t pIdx = 0; pIdx < treeSelection.size(); ++pIdx)
        {
            allParents.insert(parents[pIdx]);
        }

        geomCase->removeFromParentFields();
        delete geomCase;
    }

    for (std::set<caf::PdmObject*>::iterator it = allParents.begin(); it != allParents.end(); ++it)
    {
        updateUiSubTree(*it); 
    }

    clearClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteReservoir(RimEclipseCase* reservoir)
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
            RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
            RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
            if (analysisModels) analysisModels->removeCaseFromAllGroups(reservoir);
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

        RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
        RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
        if (analysisModels) analysisModels->removeCaseFromAllGroups(reservoir);
    }

    delete reservoir;

    clearClipboard();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipsePropertyFilter* RimUiTreeModelPdm::addPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimEclipsePropertyFilterCollection* propertyFilterCollection = NULL;
    caf::PdmUiTreeItem* propertyFilterCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimEclipsePropertyFilter*>(currentItem->dataObject().p()))
    {
        RimEclipsePropertyFilter* propertyFilter = dynamic_cast<RimEclipsePropertyFilter*>(currentItem->dataObject().p());
        propertyFilterCollection = propertyFilter->parentContainer();
        propertyFilterCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(currentItem->dataObject().p()))
    {
        propertyFilterCollection = dynamic_cast<RimEclipsePropertyFilterCollection*>(currentItem->dataObject().p());
        propertyFilterCollectionItem = currentItem;
        position = propertyFilterCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    beginInsertRows(collectionIndex, position, position);
    
    RimEclipsePropertyFilter* propertyFilter = propertyFilterCollection->createAndAppendPropertyFilter();
    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(propertyFilterCollectionItem, position, propertyFilter);

    endInsertRows();

    insertedModelIndex = index(position, 0, collectionIndex);

    if (propertyFilterCollection)
    {
        propertyFilterCollection->reservoirView()->scheduleGeometryRegen(PROPERTY_FILTERED);
    }

    return propertyFilter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechPropertyFilter* RimUiTreeModelPdm::addGeoMechPropertyFilter(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimGeoMechPropertyFilterCollection* propertyFilterCollection = NULL;
    caf::PdmUiTreeItem* propertyFilterCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimGeoMechPropertyFilter*>(currentItem->dataObject().p()))
    {
        RimGeoMechPropertyFilter* propertyFilter = dynamic_cast<RimGeoMechPropertyFilter*>(currentItem->dataObject().p());
        propertyFilterCollection = propertyFilter->parentContainer();
        propertyFilterCollectionItem = currentItem->parent();
        position = itemIndex.row();
        collectionIndex = itemIndex.parent();
    }
    else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(currentItem->dataObject().p()))
    {
        propertyFilterCollection = dynamic_cast<RimGeoMechPropertyFilterCollection*>(currentItem->dataObject().p());
        propertyFilterCollectionItem = currentItem;
        position = propertyFilterCollectionItem->childCount();
        collectionIndex = itemIndex;
    }

    beginInsertRows(collectionIndex, position, position);

    RimGeoMechPropertyFilter* propertyFilter = propertyFilterCollection->createAndAppendPropertyFilter();
    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(propertyFilterCollectionItem, position, propertyFilter);

    endInsertRows();

    insertedModelIndex = index(position, 0, collectionIndex);

    if (propertyFilterCollection)
    {
        static_cast<RimView*>(propertyFilterCollection->reservoirView())->scheduleGeometryRegen(PROPERTY_FILTERED);
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
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

    }
    return rangeFilter;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RimUiTreeModelPdm::addReservoirView(const std::vector<caf::PdmUiItem*>& treeSelection)
{
    if (!treeSelection.size() || treeSelection[0] == NULL) return NULL;

    caf::PdmUiItem* currentItem = treeSelection[0];

    // Establish type of selected object
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(currentItem);
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(currentItem);
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(currentItem);
    RimEclipseView* reservoirView = dynamic_cast<RimEclipseView*>(currentItem);

    // Find case to insert into

    if (geoMechView) geomCase = geoMechView->geoMechCase();   
    if (reservoirView) eclipseCase = reservoirView->eclipseCase();

    RimView* insertedView = NULL;

    if (eclipseCase)
    {
        insertedView = eclipseCase->createAndAddReservoirView();
    }
    else if (geomCase)
    {
        insertedView = geomCase->createAndAddReservoirView();
    }

    // Must be run before buildViewItems, as wells are created in this function
    
    insertedView->loadDataAndUpdate();

    if (eclipseCase ) this->updateUiSubTree(eclipseCase);
    if (geomCase )    this->updateUiSubTree(geomCase);
   
    return insertedView;
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
        this->updateUiSubTree(changedSColl);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::addInputProperty(const QModelIndex& itemIndex, const QStringList& fileNames)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    RimEclipseInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimEclipseInputPropertyCollection*>(currentItem->dataObject().p());
    CVF_ASSERT(inputPropertyCollection);
    
    std::vector<RimEclipseInputCase*> parentObjects;
    inputPropertyCollection->parentObjectsOfType(parentObjects);
    CVF_ASSERT(parentObjects.size() == 1);

    RimEclipseInputCase* inputReservoir = parentObjects[0];
    CVF_ASSERT(inputReservoir);
    if (inputReservoir)
    {
        inputReservoir->openDataFileSet(fileNames);
    }

    this->updateUiSubTree(inputPropertyCollection);
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
    RimEclipseInputProperty* inputProperty = dynamic_cast<RimEclipseInputProperty*>(object);
    if (!inputProperty) return;

    // Remove item from UI tree model before delete of project data structure
    removeRows_special(itemIndex.row(), 1, itemIndex.parent());

    std::vector<RimEclipseInputPropertyCollection*> parentObjects;
    object->parentObjectsOfType(parentObjects);
    CVF_ASSERT(parentObjects.size() == 1);

    RimEclipseInputPropertyCollection* inputPropertyCollection = parentObjects[0];
    if (!inputPropertyCollection) return;

    std::vector<RimEclipseInputCase*> parentObjects2;
    inputPropertyCollection->parentObjectsOfType(parentObjects2);
    CVF_ASSERT(parentObjects2.size() == 1);

    RimEclipseInputCase* inputReservoir = parentObjects2[0];
    if (!inputReservoir) return;

    inputReservoir->removeProperty(inputProperty);

    delete inputProperty;

    clearClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimUiTreeModelPdm::addStatisticalCalculation(const QModelIndex& itemIndex, QModelIndex& insertedModelIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    QModelIndex collectionIndex;
    RimIdenticalGridCaseGroup* caseGroup = NULL;
    caf::PdmUiTreeItem* parentCollectionItem = NULL;
    int position = 0;

    if (dynamic_cast<RimEclipseStatisticsCase*>(currentItem->dataObject().p()))
    {
        RimEclipseStatisticsCase* currentObject = dynamic_cast<RimEclipseStatisticsCase*>(currentItem->dataObject().p());
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

        RimProject* proj = RiaApplication::instance()->project();
        RimEclipseStatisticsCase* createdObject = caseGroup->createAndAppendStatisticsCase();
        proj->assignCaseIdToCase(createdObject);

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
RimIdenticalGridCaseGroup* RimUiTreeModelPdm::addCaseGroup(QModelIndex& insertedModelIndex)
{
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimEclipseCaseCollection* analysisModels = proj->activeOilField() ? proj->activeOilField()->analysisModels() : NULL;

    if (analysisModels)
    {
        RimIdenticalGridCaseGroup* createdObject = new RimIdenticalGridCaseGroup;
        proj->assignIdToCaseGroup(createdObject);

        RimEclipseCase* createdReservoir = createdObject->createAndAppendStatisticsCase();
        proj->assignCaseIdToCase(createdReservoir);
        createdObject->name = QString("Grid Case Group %1").arg(analysisModels->caseGroups().size() + 1);

        analysisModels->caseGroups().push_back(createdObject);

        this->updateUiSubTree(analysisModels);
        insertedModelIndex = getModelIndexFromPdmObject(createdObject);

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
void RimUiTreeModelPdm::addObjects(const QModelIndex& itemIndex, const caf::PdmObjectGroup& pdmObjects)
{
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimIdenticalGridCaseGroup* gridCaseGroup = gridCaseGroupFromItemIndex(itemIndex);
    if (gridCaseGroup)
    {
        std::vector<caf::PdmPointer<RimEclipseResultCase> > typedObjects;
        pdmObjects.createCopyByType(&typedObjects);

        if (typedObjects.size() == 0)
        {
            return;
        }

        RimEclipseResultCase* mainResultCase = NULL;
        std::vector< std::vector<int> > mainCaseGridDimensions;

        // Read out main grid and main grid dimensions if present in case group
        if (gridCaseGroup->mainCase())
        {
            mainResultCase = dynamic_cast<RimEclipseResultCase*>(gridCaseGroup->mainCase());
            CVF_ASSERT(mainResultCase);

            mainResultCase->readGridDimensions(mainCaseGridDimensions);
        }

        std::vector<RimEclipseResultCase*> insertedCases;

        // Add cases to case group
        for (size_t i = 0; i < typedObjects.size(); i++)
        {
            RimEclipseResultCase* rimResultReservoir = typedObjects[i];

            proj->assignCaseIdToCase(rimResultReservoir);

            if (gridCaseGroup->contains(rimResultReservoir))
            {
                continue;
            }

            insertedCases.push_back(rimResultReservoir);
        }

        // Initialize the new objects
        for (size_t i = 0; i < insertedCases.size(); i++)
        {
            RimEclipseResultCase* rimResultReservoir = insertedCases[i];
            caf::PdmDocument::initAfterReadTraversal(rimResultReservoir);
        }

        // Load stuff 
        for (size_t i = 0; i < insertedCases.size(); i++)
        {
            RimEclipseResultCase* rimResultReservoir = insertedCases[i];

 
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

            RimOilField* activeOilField = proj ? proj->activeOilField() : NULL;
            RimEclipseCaseCollection* analysisModels = (activeOilField) ? activeOilField->analysisModels() : NULL;
            if (analysisModels) analysisModels->insertCaseInCaseGroup(gridCaseGroup, rimResultReservoir);

            caf::PdmDocument::updateUiIconStateRecursively(rimResultReservoir);

            {
                QModelIndex rootIndex = getModelIndexFromPdmObject(gridCaseGroup->caseCollection());
                caf::PdmUiTreeItem* caseCollectionUiItem = getTreeItemFromIndex(rootIndex);

                int position = rowCount(rootIndex);
                beginInsertRows(rootIndex, position, position);
                caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(caseCollectionUiItem, -1, rimResultReservoir);
                endInsertRows();
            }

            for (size_t rvIdx = 0; rvIdx < rimResultReservoir->reservoirViews.size(); rvIdx++)
            {
                RimEclipseView* riv = rimResultReservoir->reservoirViews()[rvIdx];
                riv->loadDataAndUpdate();
            }
        }
    }
    else if (caseFromItemIndex(itemIndex))
    {
        std::vector<caf::PdmPointer<RimEclipseView> > typedObjects;
        pdmObjects.createCopyByType(&typedObjects);

        if (typedObjects.size() == 0)
        {
            return;
        }

        RimEclipseCase* rimCase = caseFromItemIndex(itemIndex);
        QModelIndex collectionIndex = getModelIndexFromPdmObject(rimCase);
        caf::PdmUiTreeItem* collectionItem = getTreeItemFromIndex(collectionIndex);

        // Add cases to case group
        for (size_t i = 0; i < typedObjects.size(); i++)
        {
            RimEclipseView* rimReservoirView = typedObjects[i];
            QString nameOfCopy = QString("Copy of ") + rimReservoirView->name;
            rimReservoirView->name = nameOfCopy;
            rimCase->reservoirViews().push_back(rimReservoirView);
 
            // Delete all wells to be able to copy/paste between cases, as the wells differ between cases
            rimReservoirView->wellCollection()->wells().deleteAllChildObjects();

            caf::PdmDocument::initAfterReadTraversal(rimReservoirView);
            rimReservoirView->setEclipseCase(rimCase);

            caf::PdmDocument::updateUiIconStateRecursively(rimReservoirView);

            rimReservoirView->loadDataAndUpdate(); 

            int position = static_cast<int>(rimCase->reservoirViews().size());
            beginInsertRows(collectionIndex, position, position);

            caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(collectionItem, position, rimReservoirView);

            endInsertRows();
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
    std::vector<caf::PdmPointer<RimEclipseResultCase> > typedObjects;
    pdmObjects.objectsByType(&typedObjects);

    for (size_t i = 0; i < typedObjects.size(); i++)
    {
        RimEclipseCase* rimReservoir = typedObjects[i];
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
        else if (dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p()))
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
    else if (dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p()))
    {
        RimEclipseCase* rimReservoir = dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p());
        CVF_ASSERT(rimReservoir);

        RimCaseCollection* caseCollection = rimReservoir->parentCaseCollection();
        if (caseCollection)
        {
            gridCaseGroup = caseCollection->parentCaseGroup();
        }
    }

    return gridCaseGroup;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::addToParentAndBuildUiItems(caf::PdmUiTreeItem* parentTreeItem, int position, caf::PdmObject* pdmObject)
{
    QModelIndex parentModelIndex;
    
    if (parentTreeItem && parentTreeItem->dataObject())
    {
        parentModelIndex = getModelIndexFromPdmObject(parentTreeItem->dataObject());
    }

    beginInsertRows(parentModelIndex, position, position);

    caf::PdmUiTreeItem* childItem = caf::UiTreeItemBuilderPdm::buildViewItems(parentTreeItem, position, pdmObject);

    endInsertRows();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimUiTreeModelPdm::caseFromItemIndex(const QModelIndex& itemIndex)
{
    caf::PdmUiTreeItem* currentItem = getTreeItemFromIndex(itemIndex);

    RimEclipseCase* rimCase = NULL;

    if (dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p()))
    {
        rimCase = dynamic_cast<RimEclipseCase*>(currentItem->dataObject().p());
    }
    else if (dynamic_cast<RimEclipseView*>(currentItem->dataObject().p()))
    {
        RimEclipseView* reservoirView = dynamic_cast<RimEclipseView*>(currentItem->dataObject().p());
        CVF_ASSERT(reservoirView);

        rimCase = reservoirView->eclipseCase();
    }

    return rimCase;
}

//--------------------------------------------------------------------------------------------------
/// Set toggle state for list of model indices. 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::setObjectToggleStateForSelection(QModelIndexList selectedIndexes, int state)
{
    bool toggleOn = (state == Qt::Checked);

    std::set<RimEclipseView*> resViewsToUpdate;

    foreach (QModelIndex index, selectedIndexes)
    {
        if (!index.isValid())
        {
            continue;
        }

        caf::PdmUiTreeItem* treeItem = UiTreeModelPdm::getTreeItemFromIndex(index);
        assert(treeItem);

        caf::PdmObject* obj = treeItem->dataObject();
        assert(obj);

        if (selectedIndexes.size() != 1)
        {
            if (obj && obj->objectToggleField())
            {
                caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>* >(obj->objectToggleField());
                if (field)
                {
                    if (state == RimUiTreeView::TOGGLE_ON)  field->setValueFromUi(true);
                    if (state == RimUiTreeView::TOGGLE_OFF) field->setValueFromUi(false);
                    if (state == RimUiTreeView::TOGGLE)     field->setValueFromUi(!(field->v()));
                }
            }
        }
        else 
        {
            // If only one item is selected, loop over its children, and toggle them instead of the 
            // selected item directly

            for (int cIdx = 0; cIdx < treeItem->childCount(); ++ cIdx)
            {
                caf::PdmUiTreeItem*  child = treeItem->child(cIdx);
                if (!child) continue;

                caf::PdmObject* childObj = child->dataObject();

                if (childObj && childObj->objectToggleField())
                {
                    caf::PdmField<bool>* field = dynamic_cast<caf::PdmField<bool>* >(childObj->objectToggleField());
                    if (field)
                    {
                        if (state == RimUiTreeView::TOGGLE_ON)  field->setValueFromUi(true);
                        if (state == RimUiTreeView::TOGGLE_OFF) field->setValueFromUi(false);
                        if (state == RimUiTreeView::TOGGLE)     field->setValueFromUi(!(field->v()));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::deleteAllWellPaths(const QModelIndex& itemIndex)
{
    if (!itemIndex.isValid()) return;

    caf::PdmUiTreeItem* uiItem = getTreeItemFromIndex(itemIndex);
    if (!uiItem) return;

    caf::PdmObject* object = uiItem->dataObject().p();
    RimWellPathCollection* wellPathCollection = dynamic_cast<RimWellPathCollection*>(object);
    if (!wellPathCollection) return;

    // Remove item from UI tree model before delete of project data structure
    removeRows_special(0, uiItem->childCount(), itemIndex);

    wellPathCollection->wellPaths.deleteAllChildObjects();

    clearClipboard();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeModelPdm::populateObjectGroupFromModelIndexList(const QModelIndexList& modelIndexList, caf::PdmObjectGroup* objectGroup)
{
    CVF_ASSERT(objectGroup);

    for (int i = 0; i < modelIndexList.size(); i++)
    {
        caf::PdmUiTreeItem* uiItem = UiTreeModelPdm::getTreeItemFromIndex(modelIndexList.at(i));

        if (uiItem && uiItem->dataObject() && uiItem->dataObject().p())
        {
            objectGroup->addObject(uiItem->dataObject().p());
        }
    }
}

