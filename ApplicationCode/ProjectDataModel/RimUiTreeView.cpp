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

#include "RimUiTreeView.h"
#include "RimUiTreeModelPdm.h"
#include "RimReservoirView.h"
#include "RimCalcScript.h"
#include "RIApplication.h"
#include "RIMainWindow.h"
#include "RimInputPropertyCollection.h"
#include "RimExportInputPropertySettings.h"
#include "RIPreferencesDialog.h"
#include "RifEclipseInputFileTools.h"
#include "RimInputReservoir.h"
#include "RimBinaryExportSettings.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimUiTreeView::RimUiTreeView(QWidget *parent /*= 0*/)
    : QTreeView(parent)
{
    setHeaderHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::contextMenuEvent(QContextMenuEvent* event)
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
            else if (dynamic_cast<RimInputPropertyCollection*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Add Input Property"), this, SLOT(slotAddInputProperty()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimInputProperty*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteObjectFromContainer()));
                menu.addAction(QString("Write"), this, SLOT(slotWriteInputProperty()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimResultSlot*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Write"), this, SLOT(slotWriteBinaryResultAsInputProperty()));
                menu.exec(event->globalPos());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotAddChildItem()
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
void RimUiTreeView::slotDeleteItem()
{
    QModelIndex index = currentIndex();
    QAbstractItemModel* myModel = model();

    if (!myModel->removeRow(index.row(), index.parent()))
        return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotShowWindow()
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
void RimUiTreeView::slotDeletePropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deletePropertyFilter(currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotDeleteRangeFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteRangeFilter(currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotAddPropertyFilter()
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
void RimUiTreeView::slotAddRangeFilter()
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
void RimUiTreeView::slotAddSliceFilterI()
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
void RimUiTreeView::slotAddSliceFilterJ()
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
void RimUiTreeView::slotAddSliceFilterK()
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
void RimUiTreeView::slotReadScriptContentFromFile()
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
void RimUiTreeView::slotEditScript()
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
void RimUiTreeView::slotNewScript()
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
void RimUiTreeView::slotExecuteScript()
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
            arguments.append("-q");
            arguments << calcScript->absolutePath();

            RIApplication::instance()->launchProcess(octavePath, arguments);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotAddView()
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
void RimUiTreeView::slotDeleteView()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteReservoirView(currentIndex());

        RIApplication* app = RIApplication::instance();
        app->setActiveReservoirView(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    caf::PdmObject* pdmObject = NULL;

    if (selected.indexes().size() == 1)
    {
        QModelIndex mi = selected.indexes()[0];
        if (mi.isValid())
        {
            RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
            if (myModel)
            {
                caf::PdmUiTreeItem* treeItem = myModel->getTreeItemFromIndex(mi);
                if (treeItem && treeItem->dataObject())
                {
                    pdmObject = treeItem->dataObject();
                }
            }
        }
    }

    emit selectedObjectChanged(pdmObject);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);

    if (selectionModel())
    {
        connect(selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotSelectionChanged( const QItemSelection & , const QItemSelection & )));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotAddInputProperty()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Eclipse Input Property Files", NULL, "All Files (*.* *)");
    if (fileNames.isEmpty()) return;

    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimInputPropertyCollection*>(uiItem->dataObject().p());
    if (inputPropertyCollection)
    {
        myModel->addInputProperty(index, fileNames);

        setCurrentIndex(index);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotDeleteObjectFromContainer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel) myModel->deleteInputProperty(index);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotWriteInputProperty()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimInputProperty* inputProperty = dynamic_cast<RimInputProperty*>(uiItem->dataObject().p());
    if (!inputProperty) return;

    {
        bool isResolved = false;
        if (inputProperty->resolvedState == RimInputProperty::RESOLVED || inputProperty->resolvedState == RimInputProperty::RESOLVED_NOT_SAVED)
        {
            isResolved = true;
        }

        if (!isResolved)
        {
            QMessageBox::warning(RIMainWindow::instance(), "Export failure", "Property is not resolved, and then it is not possible to export the property.");

            return;
        }
    }

    RimExportInputSettings exportSettings;
    exportSettings.eclipseKeyword = inputProperty->eclipseKeyword;

    // Find input reservoir for this property
    RimInputReservoir* inputReservoir = NULL;
    {
        std::vector<caf::PdmObject*> parentObjects;
        inputProperty->parentObjects(parentObjects);
        CVF_ASSERT(parentObjects.size() == 1);

        RimInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimInputPropertyCollection*>(parentObjects[0]);
        if (!inputPropertyCollection) return;

        std::vector<caf::PdmObject*> parentObjects2;
        inputPropertyCollection->parentObjects(parentObjects2);
        CVF_ASSERT(parentObjects2.size() == 1);

        inputReservoir = dynamic_cast<RimInputReservoir*>(parentObjects2[0]);
    }

    if (!inputReservoir) return;

    {
        QString projectFolder;

        RIApplication* app = RIApplication::instance();
        QString projectFileName = app->currentProjectFileName();
        if (!projectFileName.isEmpty())
        {   
            QFileInfo fi(projectFileName);
            projectFolder = fi.absolutePath();
        }
        else
        {
            projectFolder = inputReservoir->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + inputProperty->eclipseKeyword;

        exportSettings.fileName = outputFileName;
    }

    RIPreferencesDialog preferencesDialog(this, &exportSettings, "Export Eclipse Property to Text File");
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        bool isOk = RifEclipseInputFileTools::writePropertyToTextFile(exportSettings.fileName, inputReservoir->reservoirData(), 0, inputProperty->resultName, exportSettings.eclipseKeyword);
        if (isOk)
        {
            inputProperty->fileName = exportSettings.fileName;
            inputProperty->eclipseKeyword = exportSettings.eclipseKeyword;
            inputProperty->resolvedState = RimInputProperty::RESOLVED;

            inputProperty->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotWriteBinaryResultAsInputProperty()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimResultSlot* resultSlot = dynamic_cast<RimResultSlot*>(uiItem->dataObject().p());
    if (!resultSlot) return;
    if (!resultSlot->reservoirView()) return;
    if (!resultSlot->reservoirView()->eclipseCase()) return;
    if (!resultSlot->reservoirView()->eclipseCase()->reservoirData()) return;

    RimBinaryExportSettings exportSettings;
    exportSettings.eclipseKeyword = resultSlot->resultVariable;

    {
        QString projectFolder;

        RIApplication* app = RIApplication::instance();
        QString projectFileName = app->currentProjectFileName();
        if (!projectFileName.isEmpty())
        {   
            QFileInfo fi(projectFileName);
            projectFolder = fi.absolutePath();
        }
        else
        {
            projectFolder = resultSlot->reservoirView()->eclipseCase()->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + resultSlot->resultVariable;

        exportSettings.fileName = outputFileName;
    }

    RIPreferencesDialog preferencesDialog(this, &exportSettings, "Export Binary Eclipse Data to Text File");
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        size_t timeStep = resultSlot->reservoirView()->currentTimeStep();
        bool isOk = RifEclipseInputFileTools::writeBinaryResultToTextFile(exportSettings.fileName, resultSlot->reservoirView()->eclipseCase()->reservoirData(), timeStep, resultSlot->resultVariable, exportSettings.eclipseKeyword, exportSettings.undefinedValue);
        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }
}

