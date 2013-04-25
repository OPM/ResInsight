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

#include "cafPdmDocument.h"

#include "RimUiTreeView.h"
#include "RimUiTreeModelPdm.h"
#include "RimReservoirView.h"
#include "RimCalcScript.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"
#include "RimInputPropertyCollection.h"
#include "RimExportInputPropertySettings.h"
#include "RiuPreferencesDialog.h"
#include "RifEclipseInputFileTools.h"
#include "RimInputCase.h"
#include "RimBinaryExportSettings.h"
#include "RigCaseCellResultsData.h"
#include "RimStatisticsCase.h"
#include "RimResultCase.h"
#include "RimMimeData.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimUiTreeView::RimUiTreeView(QWidget *parent /*= 0*/)
    : QTreeView(parent)
{
    setHeaderHidden(true);

    m_pasteAction = new QAction(QString("Paste"), this);
    connect(m_pasteAction, SIGNAL(triggered()), SLOT(slotPastePdmObjects()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimUiTreeView::~RimUiTreeView()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    m_pasteAction->setEnabled(hasClipboardValidData());

    if (selectionModel() && selectionModel()->selection().size() == 0)
    {
        // Clicking in blank space in tree view
        QMenu menu;
        menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
        menu.exec(event->globalPos());

        return;
    }

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
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Copy View"), this, SLOT(slotCopyPdmObjectToClipboard()));
                menu.addAction(m_pasteAction);
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
                RiaApplication* app = RiaApplication::instance();

                QMenu menu;
                {
                    QAction* action = menu.addAction(QString("Edit"), this, SLOT(slotEditScript()));
                    if (app->scriptEditorPath().isEmpty())
                    {
                        action->setEnabled(false);
                    }
                }
                menu.addAction(QString("New"), this, SLOT(slotNewScript()));
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
                menu.addAction(QString("Save Property To File"), this, SLOT(slotWriteInputProperty()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimResultSlot*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Save Property To File"), this, SLOT(slotWriteBinaryResultAsInputProperty()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimStatisticsCaseCollection*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("New Statistcs Case"), this, SLOT(slotNewStatisticsCase()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimStatisticsCase*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Compute"), this, SLOT(slotComputeStatistics()));
                menu.addAction(QString("Close"), this, SLOT(slotCloseCase()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCase*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("Copy"), this, SLOT(slotCopyPdmObjectToClipboard()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Close"), this, SLOT(slotCloseCase()));
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Close"), this, SLOT(slotDeleteObjectFromPdmPointersField()));
                menu.exec(event->globalPos());
            }
            else if (dynamic_cast<RimCaseCollection*>(uiItem->dataObject().p()))
            {
                QMenu menu;
                menu.addAction(m_pasteAction);

                // Check if parent field is a StatisticsCaseCollection
                RimCaseCollection* rimCaseCollection = dynamic_cast<RimCaseCollection*>(uiItem->dataObject().p());
                if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(rimCaseCollection))
                {
                    menu.addAction(QString("New Statistics Case"), this, SLOT(slotNewStatisticsCase()));
                }

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
void RimUiTreeView::slotEditScript()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());

        RiaApplication* app = RiaApplication::instance();
        QString scriptEditor = app->scriptEditorPath();
        if (!scriptEditor.isEmpty())
        {
            QStringList arguments;
            arguments << calcScript->absolutePath;

            QProcess* myProcess = new QProcess(this);
            myProcess->start(scriptEditor, arguments);
            
            if (!myProcess->waitForStarted(1000))
            {
                QMessageBox::warning(RiuMainWindow::instance(), "Script editor", "Failed to start script editor executable\n" + scriptEditor);
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

    RiaApplication* app = RiaApplication::instance();
    QString scriptEditor = app->scriptEditorPath();
    if (!scriptEditor.isEmpty())
    {
        QStringList arguments;
        arguments << fullPathFilenameNewScript;

        QProcess* myProcess = new QProcess(this);
        myProcess->start(scriptEditor, arguments);

        if (!myProcess->waitForStarted(1000))
        {
            QMessageBox::warning(RiuMainWindow::instance(), "Script editor", "Failed to start script editor executable\n" + scriptEditor);
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

        RiaApplication* app = RiaApplication::instance();
        QString octavePath = app->octavePath();
        if (!octavePath.isEmpty())
        {
            // http://www.gnu.org/software/octave/doc/interpreter/Command-Line-Options.html#Command-Line-Options

            // -p path
            // Add path to the head of the search path for function files. The value of path specified on the command line
            // will override any value of OCTAVE_PATH found in the environment, but not any commands in the system or
            // user startup files that set the internal load path through one of the path functions.

            // -q
            // Don't print the usual greeting and version message at startup.


            // TODO: Must rename RimCalcScript::absolutePath to absoluteFileName, as the code below is confusing
            // absolutePath() is a function in QFileInfo
            QFileInfo fi(calcScript->absolutePath());
            QString octaveFunctionSearchPath = fi.absolutePath();

            QStringList arguments;
            arguments.append("--path");
            arguments << octaveFunctionSearchPath;

            arguments.append("-q");
            arguments << calcScript->absolutePath();

            RiaApplication::instance()->launchProcess(octavePath, arguments);
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
    
    QModelIndex insertedIndex;
    myModel->addReservoirView(index, insertedIndex);
    
    // Expand parent collection and inserted view item
    setExpandedUpToRoot(insertedIndex);
    
    setCurrentIndex(insertedIndex);
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

        RiaApplication* app = RiaApplication::instance();
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
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->defaultFileDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Eclipse Input Property Files", defaultDir, "All Files (*.* *)");

    if (fileNames.isEmpty()) return;

    // Remember the directory to next time
    defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setDefaultFileDialogDirectory("INPUT_FILES", defaultDir);


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
            QMessageBox::warning(RiuMainWindow::instance(), "Export failure", "Property is not resolved, and then it is not possible to export the property.");

            return;
        }
    }

    RimExportInputSettings exportSettings;
    exportSettings.eclipseKeyword = inputProperty->eclipseKeyword;

    // Find input reservoir for this property
    RimInputCase* inputReservoir = NULL;
    {
        std::vector<RimInputPropertyCollection*> parentObjects;
        inputProperty->parentObjectsOfType(parentObjects);
        CVF_ASSERT(parentObjects.size() == 1);

        RimInputPropertyCollection* inputPropertyCollection = parentObjects[0];
        if (!inputPropertyCollection) return;

        std::vector<RimInputCase*> parentObjects2;
        inputPropertyCollection->parentObjectsOfType(parentObjects2);
        CVF_ASSERT(parentObjects2.size() == 1);

        inputReservoir = parentObjects2[0];
    }

    if (!inputReservoir) return;

    {
        QString projectFolder;

        RiaApplication* app = RiaApplication::instance();
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

    RiuPreferencesDialog preferencesDialog(this, &exportSettings, "Export Eclipse Property to Text File");
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

        RiaApplication* app = RiaApplication::instance();
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

    RiuPreferencesDialog preferencesDialog(this, &exportSettings, "Export Binary Eclipse Data to Text File");
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        size_t timeStep = resultSlot->reservoirView()->currentTimeStep();
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultSlot->porosityModel());

        bool isOk = RifEclipseInputFileTools::writeBinaryResultToTextFile(exportSettings.fileName, resultSlot->reservoirView()->eclipseCase()->reservoirData(), porosityModel, timeStep, resultSlot->resultVariable, exportSettings.eclipseKeyword, exportSettings.undefinedValue);
        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotCloseCase()
{
    QModelIndexList miList;
    miList << currentIndex();

    if (userConfirmedGridCaseGroupChange(miList))
    {
        RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
        if (myModel)
        {
            QItemSelectionModel* m = selectionModel();
            CVF_ASSERT(m);

            caf::PdmObjectGroup group;

            QModelIndexList mil = m->selectedRows();
            for (int i = 0; i < mil.size(); i++)
            {
                caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(mil.at(i));
                group.addObject(uiItem->dataObject().p());
            }

            std::vector<caf::PdmPointer<RimCase> > typedObjects;
            group.objectsByType(&typedObjects);

            for (size_t i = 0; i < typedObjects.size(); i++)
            {
                RimCase* rimReservoir = typedObjects[i];
                myModel->deleteReservoir(rimReservoir);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotNewStatisticsCase()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimStatisticsCase* newObject = myModel->addStatisticalCalculation(currentIndex(), insertedIndex);
        setCurrentIndex(insertedIndex);

        setExpanded(insertedIndex, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotComputeStatistics()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimStatisticsCase* statisticsCase = dynamic_cast<RimStatisticsCase*>(uiItem->dataObject().p());
    if (!statisticsCase) return;

    statisticsCase->computeStatistics();

    if (statisticsCase->reservoirViews.size() == 0)
    {
        slotAddView();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotAddCaseGroup()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        myModel->addCaseGroup(insertedIndex);
        setCurrentIndex(insertedIndex);

        setExpanded(insertedIndex, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotDeleteObjectFromPdmPointersField()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteObjectFromPdmPointersField(currentIndex());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotCopyPdmObjectToClipboard()
{
    QItemSelectionModel* m = selectionModel();

    QModelIndexList mil = m->selectedRows();
    if (mil.size() == 0)
    {
        return;
    }

    MimeDataWithIndexes* myObject = new MimeDataWithIndexes;
    myObject->setIndexes(mil);
    
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard)
    {
        clipboard->setMimeData(myObject);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotPastePdmObjects()
{
    if (!currentIndex().isValid()) return;

    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (!myModel) return;

    QModelIndexList miList;
    miList << currentIndex();
    if (userConfirmedGridCaseGroupChange(miList))
    {
        caf::PdmObjectGroup objectGroup;
        createPdmObjectsFromClipboard(&objectGroup);
        if (objectGroup.objects().size() == 0) return;

        myModel->addObjects(currentIndex(), objectGroup);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::createPdmObjectsFromClipboard(caf::PdmObjectGroup* objectGroup)
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (!myModel) return;

    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) return;

    const MimeDataWithIndexes* mdWithIndexes = dynamic_cast<const MimeDataWithIndexes*>(clipboard->mimeData());
    if (!mdWithIndexes) return;

    QModelIndexList indexList = mdWithIndexes->indexes();
    for (int i = 0; i < indexList.size(); i++)
    {
        caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(indexList.at(i));
        objectGroup->addObject(uiItem->dataObject().p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::keyPressEvent(QKeyEvent* keyEvent)
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        if (dynamic_cast<RimCase*>(uiItem->dataObject().p())
            || dynamic_cast<RimReservoirView*>(uiItem->dataObject().p()))
        {
            if (keyEvent->matches(QKeySequence::Copy))
            {
                slotCopyPdmObjectToClipboard();
                keyEvent->setAccepted(true);
            
                return;
            }
        }

        if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem->dataObject().p())
            || dynamic_cast<RimCaseCollection*>(uiItem->dataObject().p())
            || dynamic_cast<RimCase*>(uiItem->dataObject().p())
            || dynamic_cast<RimReservoirView*>(uiItem->dataObject().p()))
        {
            if (keyEvent->matches(QKeySequence::Paste))
            {
                slotPastePdmObjects();
                keyEvent->setAccepted(true);

                return;
            }
        }
    }

    switch (keyEvent->key())
    {
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Select:
        {
            if (checkAndHandleToggleOfMultipleSelection())
            {
                keyEvent->setAccepted(true);

                return;
            }
        }
    }

    QTreeView::keyPressEvent(keyEvent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeView::hasClipboardValidData()
{
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard)
    {
        if (dynamic_cast<const MimeDataWithIndexes*>(clipboard->mimeData()))
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::dropEvent(QDropEvent* dropEvent)
{
    QModelIndexList affectedModels;

    if (dropEvent->dropAction() == Qt::MoveAction)
    {
        const MimeDataWithIndexes* myMimeData = qobject_cast<const MimeDataWithIndexes*>(dropEvent->mimeData());
        if (myMimeData)
        {
            affectedModels = myMimeData->indexes();
        }
    }

    QModelIndex dropIndex = indexAt(dropEvent->pos());
    if (dropIndex.isValid())
    {
        affectedModels.push_back(dropIndex);
    }

    if (userConfirmedGridCaseGroupChange(affectedModels))
    {
        QTreeView::dropEvent(dropEvent);
    }
}

//--------------------------------------------------------------------------------------------------
/// Displays a question to the user when a grid case group with statistical results is about to change
//--------------------------------------------------------------------------------------------------
bool RimUiTreeView::userConfirmedGridCaseGroupChange(const QModelIndexList& itemIndexList)
{
    if (itemIndexList.size() == 0) return true;

    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        caf::PdmObjectGroup pog;

        for (int i = 0; i < itemIndexList.size(); i++)
        {
            QModelIndex itemIndex = itemIndexList.at(i);
            if (!itemIndex.isValid()) continue;

            RimIdenticalGridCaseGroup* gridCaseGroup = myModel->gridCaseGroupFromItemIndex(itemIndex);
            if (gridCaseGroup)
            {
                if (hasAnyStatisticsResults(gridCaseGroup))
                {
                    if (pog.objects().count(gridCaseGroup) == 0)
                    {
                        pog.addObject(gridCaseGroup);
                    }
                }
            }
        }

        std::vector<caf::PdmPointer<RimIdenticalGridCaseGroup> > typedObjects;
        pog.objectsByType(&typedObjects);

        if (typedObjects.size() > 0)
        {
            RiuMainWindow* mainWnd = RiuMainWindow::instance();

            QMessageBox msgBox(mainWnd);
            msgBox.setIcon(QMessageBox::Question);

            QString questionText;
            if (typedObjects.size() == 1)
            {
                questionText = QString("This operation will invalidate statistics results in grid case group\n\"%1\".\n").arg(typedObjects[0]->name());
                questionText += "Computed results in this group will be deleted if you continue.";
            }
            else
            {
                questionText = "This operation will invalidate statistics results in grid case groups\n";
                for (size_t i = 0; i < typedObjects.size(); i++)
                {
                    questionText += QString("\"%1\"\n").arg(typedObjects[i]->name());
                }

                questionText += "Computed results in these groups will be deleted if you continue.";
            }

            msgBox.setText(questionText);
            msgBox.setInformativeText("Do you want to continue?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

            int ret = msgBox.exec();
            if (ret == QMessageBox::No)
            {
                return false;
            }
        }

    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeView::hasAnyStatisticsResults(RimIdenticalGridCaseGroup* gridCaseGroup)
{
    CVF_ASSERT(gridCaseGroup);

    for (size_t i = 0; i < gridCaseGroup->statisticsCaseCollection()->reservoirs().size(); i++)
    {
        RimStatisticsCase* rimStaticsCase = dynamic_cast<RimStatisticsCase*>(gridCaseGroup->statisticsCaseCollection()->reservoirs[i]);
        if (rimStaticsCase)
        {
            if (rimStaticsCase->hasComputedStatistics())
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::mousePressEvent(QMouseEvent* mouseEvent)
{
    // TODO: Handle multiple selection and changing state using mouse
    // This is a bit tricky due to the fact that there is no obvious way to trap if the check box is pressed
    // and not other parts of the check box GUI item
    
    /*
    if (checkAndHandleToggleOfMultipleSelection())
    {
        mouseEvent->setAccepted(true);
        
        return;
    }
    */

    QTreeView::mousePressEvent(mouseEvent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimUiTreeView::checkAndHandleToggleOfMultipleSelection()
{
    QModelIndex curr = currentIndex();

    // Check if the current model index supports checkable items
    if (model()->flags(curr) & Qt::ItemIsUserCheckable)
    {
        QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
        if (selectedIndexes.contains(curr))
        {
            QVariant currentState = model()->data(curr, Qt::CheckStateRole);

            // Toggle between Qt::Checked and Qt::UnChecked
            // Qt::PartiallyChecked is not handled
            int state = currentState.toInt();
            if (state == Qt::Checked)
            {
                state = Qt::Unchecked;
            }
            else
            {
                state = Qt::Checked;
            }

            RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
            myModel->setObjectToggleStateForSelection(selectedIndexes, state);

            caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(curr);

            RimWell* well = dynamic_cast<RimWell*>(uiItem->dataObject().p());
            if (well)
            {
                RimReservoirView* reservoirView = NULL;
                well->firstAncestorOfType(reservoirView);
                if (reservoirView)
                {
                    reservoirView->createDisplayModelAndRedraw();
                }
            }

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void setExpandedState(QStringList& nodes, QTreeView* view, QAbstractItemModel* model,
                      const QModelIndex startIndex, QString path)
{
    path += QString::number(startIndex.row()) + QString::number(startIndex.column());
    for (int i = 0; i < model->rowCount(startIndex); ++i)
    {
        QModelIndex nextIndex = model->index(i, 0, startIndex);
        QString nextPath = path + QString::number(nextIndex.row()) + QString::number(nextIndex.column());
        if(!nodes.contains(nextPath))
            continue;
        
        setExpandedState(nodes, view, model, model->index(i, 0, startIndex), path);
    }
    
    if (nodes.contains(path))
    {
        view->setExpanded( startIndex.sibling(startIndex.row(), 0), true );
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void storeExpandedState(QStringList & nodes, QTreeView * view, QAbstractItemModel * model,
                        const QModelIndex startIndex, QString path)
{
    path += QString::number(startIndex.row()) + QString::number(startIndex.column());
    for (int i = 0; i < model->rowCount(startIndex); ++i)
    {
        if(!view->isExpanded(model->index(i, 0, startIndex)))
            continue;

        storeExpandedState(nodes, view, model, model->index(i, 0, startIndex), path);
    }

    if (view->isExpanded(startIndex))
    {
        nodes << path;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::applyTreeViewStateFromString(const QString& treeViewState)
{
    if (this->model())
    {
        this->collapseAll();

        QStringList nodes = treeViewState.split(";");

        QString path;
        setExpandedState(nodes, this, this->model(), QModelIndex(), path);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::storeTreeViewStateToString(QString& treeViewState)
{
    if (this->model())
    {
        QStringList nodes;
        QString path;

        storeExpandedState(nodes, this, this->model(), QModelIndex(), path);

        treeViewState = nodes.join(";");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::setExpandedUpToRoot(const QModelIndex& itemIndex)
{
    QModelIndex mi = itemIndex;

    while (mi.isValid())
    {
        this->setExpanded(mi, true);
        mi = mi.parent();
    }
}

