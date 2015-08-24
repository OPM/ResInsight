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

//#include "RiaStdInclude.h"

#include "RimUiTreeView.h"


#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RifEclipseInputFileTools.h"
#include "RigCaseCellResultsData.h"
#include "RigSingleWellResultsData.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimBinaryExportSettings.h"
#include "RimCalcScript.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseStatisticsCaseCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimExportInputPropertySettings.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechPropertyFilter.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimMimeData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimScriptCollection.h"
#include "RimUiTreeModelPdm.h"
#include "RimWellPathCollection.h"
#include "RiuMainWindow.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include "RimTreeViewStateSerializer.h"



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
            QMenu menu;

            if (dynamic_cast<RimGeoMechView*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Copy View"), this, SLOT(slotCopyPdmObjectToClipboard()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteView()));
            }
            else if (dynamic_cast<RimEclipseView*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Copy View"), this, SLOT(slotCopyPdmObjectToClipboard()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteView()));
            }
            else if (dynamic_cast<RimCellRangeFilterCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New Range Filter"), this, SLOT(slotAddRangeFilter()));
                menu.addAction(QString("Slice I Filter"), this, SLOT(slotAddSliceFilterI()));
                menu.addAction(QString("Slice J Filter"), this, SLOT(slotAddSliceFilterJ()));
                menu.addAction(QString("Slice K Filter"), this, SLOT(slotAddSliceFilterK()));
            }
            else if (dynamic_cast<RimCellRangeFilter*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Insert Range Filter"), this, SLOT(slotAddRangeFilter()));
                menu.addAction(QString("Slice I Filter"), this, SLOT(slotAddSliceFilterI()));
                menu.addAction(QString("Slice J Filter"), this, SLOT(slotAddSliceFilterJ()));
                menu.addAction(QString("Slice K Filter"), this, SLOT(slotAddSliceFilterK()));
                menu.addSeparator();
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteRangeFilter()));
            }
            else if (dynamic_cast<RimEclipsePropertyFilterCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New Property Filter"), this, SLOT(slotAddPropertyFilter()));
            }
            else if (dynamic_cast<RimEclipsePropertyFilter*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Insert Property Filter"), this, SLOT(slotAddPropertyFilter()));
                menu.addSeparator();
                menu.addAction(QString("Delete"), this, SLOT(slotDeletePropertyFilter()));
            }
            else if (dynamic_cast<RimGeoMechPropertyFilterCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New Property Filter"), this, SLOT(slotAddGeoMechPropertyFilter()));
            }
            else if (dynamic_cast<RimGeoMechPropertyFilter*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Insert Property Filter"), this, SLOT(slotAddGeoMechPropertyFilter()));
                menu.addSeparator();
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteGeoMechPropertyFilter()));
            }
            else if (dynamic_cast<RimCalcScript*>(uiItem->dataObject().p()))
            {
                RiaApplication* app = RiaApplication::instance();

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
            }
            else if (dynamic_cast<RimEclipseInputPropertyCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Add Input Property"), this, SLOT(slotAddInputProperty()));
            }
            else if (dynamic_cast<RimEclipseInputProperty*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Delete"), this, SLOT(slotDeleteObjectFromContainer()));
                menu.addAction(QString("Save Property To File"), this, SLOT(slotWriteInputProperty()));
            }
            else if (dynamic_cast<RimEclipseCellColors*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Save Property To File"), this, SLOT(slotWriteBinaryResultAsInputProperty()));
            }
            else if (dynamic_cast<RimEclipseStatisticsCaseCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New Statistics Case"), this, SLOT(slotNewStatisticsCase()));
            }
            else if (dynamic_cast<RimEclipseStatisticsCase*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Compute"), this, SLOT(slotComputeStatistics()));
                menu.addAction(QString("Close"), this, SLOT(slotCloseCase()));
            }
            else if (dynamic_cast<RimGeoMechCase*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("Close"), this, SLOT(slotCloseGeomechCase()));
                menu.addAction(m_pasteAction);
            }
            else if (dynamic_cast<RimEclipseCase*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Copy"), this, SLOT(slotCopyPdmObjectToClipboard()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Close"), this, SLOT(slotCloseCase()));
                menu.addAction(QString("New View"), this, SLOT(slotAddView()));
                menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
            }
            else if (dynamic_cast<RimIdenticalGridCaseGroup*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
                menu.addAction(m_pasteAction);
                menu.addAction(QString("Close"), this, SLOT(slotDeleteObjectFromPdmChildArrayField()));
            }
            else if (dynamic_cast<RimCaseCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(m_pasteAction);

                // Check if parent field is a StatisticsCaseCollection
                RimCaseCollection* rimCaseCollection = dynamic_cast<RimCaseCollection*>(uiItem->dataObject().p());
                if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(rimCaseCollection))
                {
                    menu.addAction(QString("New Statistics Case"), this, SLOT(slotNewStatisticsCase()));
                }
            }
            else if (dynamic_cast<RimScriptCollection*>(uiItem->dataObject().p()) || dynamic_cast<RimCalcScript*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Add Script Path"), this, SLOT(slotAddScriptPath()));
                menu.addAction(QString("Delete Script Path"), this, SLOT(slotDeleteScriptPath()));
            }
            else if (dynamic_cast<RimWellPathCollection*>(uiItem->dataObject().p()))
            {
                menu.addAction(QString("Delete All Well Paths"), this, SLOT(slotDeleteAllWellPaths()));
                
                RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
                ruiMainWindow->appendActionsContextMenuForPdmObject(uiItem->dataObject().p(), &menu);
            }
            else if (dynamic_cast<RimEclipseCaseCollection*>(uiItem->dataObject().p()))
            {
                RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
                ruiMainWindow->appendActionsContextMenuForPdmObject(uiItem->dataObject().p(), &menu);
                menu.addAction(QString("New Grid Case Group"), this, SLOT(slotAddCaseGroup()));
            }

            // Execute script on selection of cases
            RiuMainWindow* ruiMainWindow = RiuMainWindow::instance();
            if (ruiMainWindow)
            {
                std::vector<RimCase*> cases;
                ruiMainWindow->selectedCases(cases);

                if (cases.size() > 0)
                {
                    QMenu* executeMenu = menu.addMenu("Execute script");

                    RiaApplication* app = RiaApplication::instance();
                    RimProject* proj = app->project();
                    if (proj && proj->scriptCollection())
                    {
                        RimScriptCollection* rootScriptCollection = proj->scriptCollection();

                        // Root script collection holds a list of subdirectories of user defined script folders
                        for (size_t i = 0; i < rootScriptCollection->subDirectories.size(); i++)
                        {
                            RimScriptCollection* subDir = rootScriptCollection->subDirectories[i];

                            if (subDir)
                            {
                                appendScriptItems(executeMenu, subDir);
                            }
                        }
                    }

                    menu.addSeparator();
                    menu.addMenu(executeMenu);
                }
            }

            appendToggleItemActions(menu);
            menu.exec(event->globalPos());
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
// OBSOLETE - see RicEclipsePropertyFilterInsert
void RimUiTreeView::slotAddPropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimEclipsePropertyFilter* propFilter = myModel->addPropertyFilter(currentIndex(), insertedIndex);
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
// OBSOLETE - see RicGeoMechPropertyFilterInsert
void RimUiTreeView::slotAddGeoMechPropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimGeoMechPropertyFilter* propFilter = myModel->addGeoMechPropertyFilter(currentIndex(), insertedIndex);
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
void RimUiTreeView::slotDeleteGeoMechPropertyFilter()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteGeoMechPropertyFilter(currentIndex());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicRangeFilterNew
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
// OBSOLETE - see RicRangeFilterNewSliceI
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

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicRangeFilterNewSliceJ
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

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicRangeFilterNewSliceK
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

        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED);
        rangeFilterCollection->reservoirView()->scheduleGeometryRegen(RANGE_FILTERED_INACTIVE);

        rangeFilterCollection->reservoirView()->createDisplayModelAndRedraw();

        setCurrentIndex(insertedIndex);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicEditScriptFeature
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
// OBSOLETE - see RicNewScriptFeature
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
// OBSOLETE - see RicExecuteScriptForCasesFeature
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
            // TODO: Must rename RimCalcScript::absolutePath to absoluteFileName, as the code below is confusing
            // absolutePath() is a function in QFileInfo
            QFileInfo fi(calcScript->absolutePath());
            QString octaveFunctionSearchPath = fi.absolutePath();

            QStringList arguments = app->octaveArguments();
            arguments.append("--path");
            arguments << octaveFunctionSearchPath;
            arguments << calcScript->absolutePath();

            RiaApplication::instance()->launchProcess(octavePath, arguments);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicExecuteScriptFeature
void RimUiTreeView::slotExecuteScriptForSelectedCases()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (!action) return;

    QString encodedModelIndex = action->data().toString();
    QModelIndex mi = RimTreeViewStateSerializer::getModelIndexFromString(model(), encodedModelIndex);

    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (!myModel) return;

    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(mi);
    if (uiItem)
    {
        RimCalcScript* calcScript = dynamic_cast<RimCalcScript*>(uiItem->dataObject().p());
        if (!calcScript) return;

        RiaApplication* app = RiaApplication::instance();
        QString octavePath = app->octavePath();
        if (!octavePath.isEmpty())
        {
            // TODO: Must rename RimCalcScript::absolutePath to absoluteFileName, as the code below is confusing
            // absolutePath() is a function in QFileInfo
            QFileInfo fi(calcScript->absolutePath());
            QString octaveFunctionSearchPath = fi.absolutePath();

            QStringList arguments = app->octaveArguments();
            arguments.append("--path");
            arguments << octaveFunctionSearchPath;
            arguments << calcScript->absolutePath();

            // Get case ID from selected cases in selection model
            std::vector<int> caseIdsInSelection;
            {
                QItemSelectionModel* m = selectionModel();
                CVF_ASSERT(m);

                caf::PdmObjectGroup group;

                QModelIndexList mil = m->selectedRows();

                myModel->populateObjectGroupFromModelIndexList(mil, &group);

                std::vector<caf::PdmPointer<RimEclipseCase> > typedObjects;
                group.objectsByType(&typedObjects);

                for (size_t i = 0; i < typedObjects.size(); i++)
                {
                    RimEclipseCase* rimReservoir = typedObjects[i];
                    caseIdsInSelection.push_back(rimReservoir->caseId);
                }
            }

            if (caseIdsInSelection.size() > 0)
            {
                RiaApplication::instance()->launchProcessForMultipleCases(octavePath, arguments, caseIdsInSelection);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicNewViewFeature
void RimUiTreeView::slotAddView()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    std::vector<caf::PdmUiItem*> selection;
    this->selectedUiItems(selection);

    RimView* newView = myModel->addReservoirView(selection);
    QModelIndex insertedIndex = myModel->getModelIndexFromPdmObject(newView);

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
    std::vector<caf::PdmUiItem*> selection;
    this->selectedUiItems(selection);
    myModel->deleteReservoirViews(selection);

    RiaApplication* app = RiaApplication::instance();
    app->setActiveReservoirView(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    caf::PdmObjectHandle* pdmObject = NULL;

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
// OBSOLETE - see RicAddEclipseInputPropertyFeature
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

    RimEclipseInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimEclipseInputPropertyCollection*>(uiItem->dataObject().p());
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
// OBSOLETE - see RicSaveEclipseInputPropertyFeature
void RimUiTreeView::slotWriteInputProperty()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimEclipseInputProperty* inputProperty = dynamic_cast<RimEclipseInputProperty*>(uiItem->dataObject().p());
    if (!inputProperty) return;

    {
        bool isResolved = false;
        if (inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED || inputProperty->resolvedState == RimEclipseInputProperty::RESOLVED_NOT_SAVED)
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
    RimEclipseInputCase* inputReservoir = NULL;
    {
        RimEclipseInputPropertyCollection* inputPropertyCollection = dynamic_cast<RimEclipseInputPropertyCollection*>(inputProperty->parentField()->ownerObject());
        if (!inputPropertyCollection) return;

        inputReservoir = dynamic_cast<RimEclipseInputCase*>(inputPropertyCollection->parentField()->ownerObject());
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

    caf::PdmUiPropertyViewDialog propertyDialog(this, &exportSettings, "Export Eclipse Property to Text File", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        bool isOk = RifEclipseInputFileTools::writePropertyToTextFile(exportSettings.fileName, inputReservoir->reservoirData(), 0, inputProperty->resultName, exportSettings.eclipseKeyword);
        if (isOk)
        {
            inputProperty->fileName = exportSettings.fileName;
            inputProperty->eclipseKeyword = exportSettings.eclipseKeyword;
            inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;

            inputProperty->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicSaveEclipseResultAsInputProperty
void RimUiTreeView::slotWriteBinaryResultAsInputProperty()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimEclipseCellColors* resultColors = dynamic_cast<RimEclipseCellColors*>(uiItem->dataObject().p());
    if (!resultColors) return;
    if (!resultColors->reservoirView()) return;
    if (!resultColors->reservoirView()->eclipseCase()) return;
    if (!resultColors->reservoirView()->eclipseCase()->reservoirData()) return;

    RimBinaryExportSettings exportSettings;
    exportSettings.eclipseKeyword = resultColors->resultVariable();

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
            projectFolder = resultColors->reservoirView()->eclipseCase()->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + resultColors->resultVariable();

        exportSettings.fileName = outputFileName;
    }

    caf::PdmUiPropertyViewDialog propertyDialog(this, &exportSettings, "Export Binary Eclipse Data to Text File", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        size_t timeStep = resultColors->reservoirView()->currentTimeStep();
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(resultColors->porosityModel());

        bool isOk = RifEclipseInputFileTools::writeBinaryResultToTextFile(exportSettings.fileName, resultColors->reservoirView()->eclipseCase()->reservoirData(), porosityModel, timeStep, resultColors->resultVariable(), exportSettings.eclipseKeyword, exportSettings.undefinedValue);
        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - See RicCloseCaseFeature
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
            myModel->populateObjectGroupFromModelIndexList(mil, &group);

            std::vector<caf::PdmPointer<RimEclipseCase> > typedObjects;
            group.objectsByType(&typedObjects);

            for (size_t i = 0; i < typedObjects.size(); i++)
            {
                RimEclipseCase* rimReservoir = typedObjects[i];
                myModel->deleteReservoir(rimReservoir);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicNewStatisticsCaseFeature
void RimUiTreeView::slotNewStatisticsCase()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        QModelIndex insertedIndex;
        RimEclipseStatisticsCase* newObject = myModel->addStatisticalCalculation(currentIndex(), insertedIndex);
        setCurrentIndex(insertedIndex);

        setExpanded(insertedIndex, true);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicComputeStatisticsFeature
void RimUiTreeView::slotComputeStatistics()
{
    QModelIndex index = currentIndex();
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());

    RimEclipseStatisticsCase* statisticsCase = dynamic_cast<RimEclipseStatisticsCase*>(uiItem->dataObject().p());
    if (!statisticsCase) return;

    statisticsCase->computeStatistics();

    statisticsCase->scheduleACTIVEGeometryRegenOnReservoirViews();
    statisticsCase->updateConnectedEditorsAndReservoirViews();

    if (statisticsCase->reservoirViews.size() == 0)
    {
        slotAddView();
    }



}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicEclipseCaseNewGroup
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
void RimUiTreeView::slotDeleteObjectFromPdmChildArrayField()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteObjectFromPdmChildArrayField(currentIndex());
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
        if (objectGroup.objects.size() == 0) return;

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
    myModel->populateObjectGroupFromModelIndexList(indexList, objectGroup);
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
        if (dynamic_cast<RimEclipseCase*>(uiItem->dataObject().p())
            || dynamic_cast<RimEclipseView*>(uiItem->dataObject().p()))
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
            || dynamic_cast<RimEclipseCase*>(uiItem->dataObject().p())
            || dynamic_cast<RimEclipseView*>(uiItem->dataObject().p()))
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
            executeSelectionToggleOperation(TOGGLE);
            keyEvent->setAccepted(true);

            return;
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
// OBSOLETE - See RiuDragDrop
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
                    if (std::find(pog.objects.begin(), pog.objects.end(), gridCaseGroup) == pog.objects.end())
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
        RimEclipseStatisticsCase* rimStaticsCase = dynamic_cast<RimEclipseStatisticsCase*>(gridCaseGroup->statisticsCaseCollection()->reservoirs[i]);
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
void RimUiTreeView::setExpandedUpToRoot(const QModelIndex& itemIndex)
{
    QModelIndex mi = itemIndex;

    while (mi.isValid())
    {
        this->setExpanded(mi, true);
        mi = mi.parent();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicAddScriptPathFeature
void RimUiTreeView::slotAddScriptPath()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(this, "Select script folder");
    if (!selectedFolder.isEmpty())
    {
        QString filePathString = RiaApplication::instance()->preferences()->scriptDirectories();

        QChar separator(';');
        if (!filePathString.isEmpty() && !filePathString.endsWith(separator, Qt::CaseInsensitive))
        {
            filePathString += separator;
        }
        
        filePathString += selectedFolder;

        RiaApplication::instance()->preferences()->scriptDirectories = filePathString;
        RiaApplication::instance()->applyPreferences();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - see RicDeleteScriptPathFeature
void RimUiTreeView::slotDeleteScriptPath()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
    if (uiItem)
    {
        if (dynamic_cast<RimScriptCollection*>(uiItem->dataObject().p()))
        {
            RimScriptCollection* scriptCollection = dynamic_cast<RimScriptCollection*>(uiItem->dataObject().p());
            QString toBeRemoved = scriptCollection->directory;

            QString originalFilePathString = RiaApplication::instance()->preferences()->scriptDirectories();
            QString filePathString = originalFilePathString.remove(toBeRemoved);

            // Remove duplicate separators
            QChar separator(';');
            QString regExpString = QString("%1{1,5}").arg(separator);
            filePathString.replace(QRegExp(regExpString), separator);

            // Remove separator at end
            if (filePathString.endsWith(separator))
            {
                filePathString = filePathString.left(filePathString.size() - 1);
            }

            RiaApplication::instance()->preferences()->scriptDirectories = filePathString;
            RiaApplication::instance()->applyPreferences();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicToggleItemsFeatureImpl/RimProject
void RimUiTreeView::appendToggleItemActions(QMenu& contextMenu)
{
    if (selectionModel() && selectionModel()->selectedIndexes().size() > 0)
    {
        foreach (QModelIndex index, selectionModel()->selectedIndexes())
        {
            if (!index.isValid()) return;

            if (!(model()->flags(index) & Qt::ItemIsUserCheckable)) return;
        }

        if (contextMenu.actions().size() > 0)
        {
            contextMenu.addSeparator();
        }

        if (selectionModel()->selectedIndexes().size() > 1)
        {
            contextMenu.addAction(QString("On"), this, SLOT(slotToggleItemsOn()));
            contextMenu.addAction(QString("Off"), this, SLOT(slotToggleItemsOff()));
            contextMenu.addAction(QString("Toggle"), this, SLOT(slotToggleItems()));
        }
        else
        {
            QModelIndex mIdx = selectionModel()->selectedIndexes()[0];
            caf::PdmUiTreeItem* treeItem = caf::UiTreeModelPdm::getTreeItemFromIndex(mIdx);
            if (treeItem && treeItem->childCount())
            {
                contextMenu.addAction(QString("Sub Items On"), this, SLOT(slotToggleItemsOn()));
                contextMenu.addAction(QString("Sub Items Off"), this, SLOT(slotToggleItemsOff()));
                contextMenu.addAction(QString("Toggle Sub items"), this, SLOT(slotToggleItems()));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicToggleItemsFeature
void RimUiTreeView::slotToggleItems()
{
    executeSelectionToggleOperation(TOGGLE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicToggleItemsFeatureImpl
void RimUiTreeView::executeSelectionToggleOperation(SelectionToggleType toggleState)
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());

    myModel->setObjectToggleStateForSelection(selectionModel()->selectedIndexes(), toggleState);

    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicToggleItemsOnFeature
void RimUiTreeView::slotToggleItemsOn()
{
    executeSelectionToggleOperation(TOGGLE_ON);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicToggleItemsOffFeature
void RimUiTreeView::slotToggleItemsOff()
{
    executeSelectionToggleOperation(TOGGLE_OFF);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::appendScriptItems(QMenu* menu, RimScriptCollection* scriptCollection)
{
    CVF_ASSERT(menu);

    QDir dir(scriptCollection->directory);
    QMenu* subMenu = menu->addMenu(dir.dirName());

    for (size_t i = 0; i < scriptCollection->calcScripts.size(); i++)
    {
        RimCalcScript* calcScript = scriptCollection->calcScripts[i];
        QFileInfo fi(calcScript->absolutePath());

        QString menuText = fi.baseName();
        QAction* scriptAction = subMenu->addAction(menuText, this, SLOT(slotExecuteScriptForSelectedCases()));

        QModelIndex mi;
        RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
        if (myModel)
        {
            mi = myModel->getModelIndexFromPdmObject(calcScript);
        }

        QString encodedModelIndex;
        RimTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndex);

        scriptAction->setData(QVariant(encodedModelIndex));
    }

    for (size_t i = 0; i < scriptCollection->subDirectories.size(); i++)
    {
        RimScriptCollection* subDir = scriptCollection->subDirectories[i];

        appendScriptItems(subMenu, subDir);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE RicWellPathsDeleteAllFeature
void RimUiTreeView::slotDeleteAllWellPaths()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    if (myModel)
    {
        myModel->deleteAllWellPaths(currentIndex());

        caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(currentIndex());
        if (uiItem && uiItem->dataObject())
        {
            RimWellPathCollection* wellPathCollection = dynamic_cast<RimWellPathCollection*>(uiItem->dataObject().p());
            if (wellPathCollection)
            {
                wellPathCollection->scheduleGeometryRegenAndRedrawViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimUiTreeView::selectedUiItems(std::vector<caf::PdmUiItem*>& objects)
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    QModelIndexList idxList = this->selectionModel()->selectedIndexes();

    for (int i = 0; i < idxList.size(); i++)
    {
        caf::PdmUiTreeItem* uiItem = myModel->getTreeItemFromIndex(idxList[i]);
        if (uiItem)
        {
            caf::PdmUiItem* item = uiObj(uiItem->dataObject());
            objects.push_back(item);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
// OBSOLETE - See RicCloseCaseFeature
void RimUiTreeView::slotCloseGeomechCase()
{
    RimUiTreeModelPdm* myModel = dynamic_cast<RimUiTreeModelPdm*>(model());
    std::vector<caf::PdmUiItem*> selection;
    this->selectedUiItems(selection);
    myModel->deleteGeoMechCases(selection);

}
