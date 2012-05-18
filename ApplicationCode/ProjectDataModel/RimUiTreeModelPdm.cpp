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
bool RimUiTreeModelPdm::removePropertyFilter(const QModelIndex& itemIndex)
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
bool RimUiTreeModelPdm::removeRangeFilter(const QModelIndex& itemIndex)
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
bool RimUiTreeModelPdm::removeReservoirView(const QModelIndex& itemIndex)
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

    RimReservoirView* reservoirView = dynamic_cast<RimReservoirView*>(currentItem->dataObject().p());
    if (!reservoirView) return NULL;

    RimReservoirView* insertedView = reservoirView->eclipseCase()->createAndAddReservoirView();
    caf::PdmUiTreeItem* collectionItem = currentItem->parent();

    size_t viewCount = rowCount(itemIndex.parent());
    beginInsertRows(itemIndex.parent(), viewCount, viewCount);

    caf::PdmUiTreeItem* childItem = new caf::PdmUiTreeItem(collectionItem, viewCount, insertedView);

    endInsertRows();

    insertedView->loadDataAndUpdate();

    rebuildUiSubTree(insertedView);
    
    return insertedView;
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
RimTreeView::RimTreeView(QWidget *parent /*= 0*/)
    : QTreeView(parent)
{
    setHeaderHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
        if (uiItem && uiItem->dataObject())
        {
            // Range filters
            if (dynamic_cast<RimReservoirView*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Show 3D Window"), this, SLOT(slotShowWindow()));
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteView()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCellRangeFilterCollection*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("New Range Filter"), this, SLOT(slotAddRangeFilter()));
                menu.addAction(QString("Slice I Filter"), this, SLOT(slotAddSliceFilterI()));
                menu.addAction(QString("Slice J Filter"), this, SLOT(slotAddSliceFilterJ()));
                menu.addAction(QString("Slice K Filter"), this, SLOT(slotAddSliceFilterK()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCellRangeFilter*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Insert Range Filter"), this, SLOT(slotAddRangeFilter()));
                menu.addAction(QString("Slice I Filter"), this, SLOT(slotAddSliceFilterI()));
                menu.addAction(QString("Slice J Filter"), this, SLOT(slotAddSliceFilterJ()));
                menu.addAction(QString("Slice K Filter"), this, SLOT(slotAddSliceFilterK()));
                menu.addSeparator();
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteRangeFilter()));

                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCellPropertyFilterCollection*>(uiItem->dataObject().p()))
            {

                QMenu menu;
                menu.addAction(QString("New Property Filter"), this, SLOT(slotAddPropertyFilter()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCellPropertyFilter*>(uiItem->dataObject().p()))
            {

                QMenu menu;
                menu.addAction(QString("Insert Property Filter"), this, SLOT(slotAddPropertyFilter()));
                menu.addSeparator();
                menu.addAction(QString("Delete"), this, SLOT(slotDeletePropertyFilter()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCalcScript*>(uiItem->dataObject().p()))
            {
                RIApplication* app = RIApplication::instance();

                QMenu menu;
                {
                    QAction* action = menu.addAction(QString("Edit"), this, SLOT(slotEditScript()));
                    if (app->scriptEditorPath().isEmpty())
                    {
                        action->setEnabled(false);
                    }
                }
                menu.addAction(QString("New"), this, SLOT(slotNewScript()));
                //menu.addAction(QString("ReadFromFile"), this, SLOT(slotReadScriptContentFromFile()));
                menu.addSeparator();

                {
                    QAction* action = menu.addAction(QString("Execute"), this, SLOT(slotExecuteScript()));
                    if (app->octavePath().isEmpty())
                    {
                        action->setEnabled(false);
                    }
                }

                menu.exec(event->globalPos());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddChildItem()
{

    QModelIndex index = currentIndex();
    QAbstractItemModel* myModel = model();

    // Insert a single row at the end of the collection of items
    int itemCount = myModel->rowCount(index);
    if (!myModel->insertRow(itemCount, index))
        return;

    selectionModel()->setCurrentIndex(myModel->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotDeleteItem()
{
    QModelIndex index = currentIndex();
    QAbstractItemModel* myModel = model();

    if (!myModel->removeRow(index.row(), index.parent()))
        return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotShowWindow()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    RimReservoirView * riv = NULL;
    if (riv = dynamic_cast<RimReservoirView*>(uiItem->dataObject().p()))
    {
        riv->showWindow = true;
        bool generateDisplayModel = (riv->viewer() == NULL);
        riv->updateViewerWidget();
        if (generateDisplayModel)
        {
            riv->createDisplayModelAndRedraw();
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotDeletePropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->removePropertyFilter(currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotDeleteRangeFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->removeRangeFilter(currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddPropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimCellPropertyFilter* propFilter = myModel->addPropertyFilter(currentIndex(), insertedIndex);
        setCurrentIndex(insertedIndex);
        if (propFilter)
        {
            propFilter->parentContainer()->reservoirView()->createDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddRangeFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimCellRangeFilter* newFilter = myModel->addRangeFilter(currentIndex(), insertedIndex);
        setCurrentIndex(insertedIndex);

        if (newFilter && newFilter->parentContainer())
        {
            newFilter->parentContainer()->reservoirView()->createDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddSliceFilterI()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(currentIndex(), insertedIndex);

        RimCellRangeFilterCollection* rangeFilterCollection = rangeFilter->parentContainer();
        rangeFilter->name = QString("Slice I (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountI = 1;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddSliceFilterJ()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(currentIndex(), insertedIndex);

        RimCellRangeFilterCollection* rangeFilterCollection = rangeFilter->parentContainer();
        rangeFilter->name = QString("Slice J (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountJ = 1;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddSliceFilterK()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimCellRangeFilter* rangeFilter = myModel->addRangeFilter(currentIndex(), insertedIndex);

        RimCellRangeFilterCollection* rangeFilterCollection = rangeFilter->parentContainer();
        rangeFilter->name = QString("Slice K (%1)").arg(rangeFilterCollection->rangeFilters().size());
        rangeFilter->cellCountK = 1;

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RivReservoirViewPartMgr::RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotReadScriptContentFromFile()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());
        if (calcScript)
        {
            calcScript->readContentFromFile();
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotEditScript()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());

        RIApplication* app = RIApplication::instance();
        QString scriptEditor = app->scriptEditorPath();
        if (!scriptEditor.isEmpty())
        {
            QStringList arguments;
            arguments << calcScript->absolutePath;

            QProcess* myProcess = new QProcess(this);
            myProcess->start(scriptEditor, arguments);
            
            if (!myProcess->waitForStarted(1000))
            {
                QMessageBox::warning(RIMainWindow::instance(), "Script editor", "Failed to start script editor executable\n" + scriptEditor);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotNewScript()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    RimCalcScript* calcScript = NULL;
    RimScriptCollection * scriptColl = NULL;

    calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());
    scriptColl = dynamic_cast<RimScriptCollection*>(uiItem->dataObject().p());
    QString fullPathNewScript;

    if (calcScript )
    {
        QFileInfo existingScriptFileInfo(calcScript->absolutePath());
        fullPathNewScript = existingScriptFileInfo.absolutePath();
    }
    else if (scriptColl)
    {
        fullPathNewScript = scriptColl->directory();
    }
    else
    {
        return;
    }

    QString fullPathFilenameNewScript;

    fullPathFilenameNewScript = fullPathNewScript + "/untitled.m";
    int num= 1;
    while (QFileInfo(fullPathFilenameNewScript).exists())
    {
        fullPathFilenameNewScript = fullPathNewScript + "/untitled" + QString::number(num) + ".m";
        num++;
    }

    RIApplication* app = RIApplication::instance();
    QString scriptEditor = app->scriptEditorPath();
    if (!scriptEditor.isEmpty())
    {
        QStringList arguments;
        arguments << fullPathFilenameNewScript;

        QProcess* myProcess = new QProcess(this);
        myProcess->start(scriptEditor, arguments);

        if (!myProcess->waitForStarted(1000))
        {
            QMessageBox::warning(RIMainWindow::instance(), "Script editor", "Failed to start script editor executable\n" + scriptEditor);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotExecuteScript()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());

        RIApplication* app = RIApplication::instance();
        QString octavePath = app->octavePath();
        if (!octavePath.isEmpty())
        {
            QStringList arguments;

            arguments << calcScript->absolutePath();

            if (!RIApplication::instance()->launchProcess(octavePath, arguments))
            {
                QMessageBox::warning(RIMainWindow::instance(), "Script execution", "Failed to start script executable located at\n" + octavePath);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotAddView()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    
    RimReservoirView* rimView = dynamic_cast<RimReservoirView*>(uiItem->dataObject().p());
    if (rimView)
    {
        QModelIndex insertedIndex;
        myModel->addReservoirView(index, insertedIndex);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimTreeView::slotDeleteView()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->removeReservoirView(currentIndex());

        RIApplication* app = RIApplication::instance();
        app->setActiveReservoirView(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimUiPropertyCreatorPdm::RimUiPropertyCreatorPdm(QObject* parent)
    : caf::UiPropertyCreatorPdm(parent)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiPropertyCreatorPdm::uiFields(const caf::PdmObject* object, std::vector<caf::PdmFieldHandle*>& fields) const
{
    const RimCellPropertyFilter* propertyFilter = dynamic_cast<const RimCellPropertyFilter*>(object);
    if (propertyFilter)
    {
        caf::UiPropertyCreatorPdm::uiFields(object, fields);

        CVF_ASSERT(propertyFilter->resultDefinition);

        // Append fields from result definition object
        std::vector<caf::PdmFieldHandle*> childFields;
        propertyFilter->resultDefinition->fields(childFields);

        size_t i;
        for (i = 0; i < childFields.size(); i++)
        {
            fields.push_back(childFields[i]);
        }
    }
    else
    {
        caf::UiPropertyCreatorPdm::uiFields(object, fields);
    }


}

