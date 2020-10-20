// clang-format off

#include "MainWindow.h"

#include "WidgetLayoutTest.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTableView.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDockWidget>
#include <QMenuBar>
#include <QTreeView>
#include <QUndoView>

#ifdef TAP_USE_COMMAND_FRAMEWORK
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdSelectionHelper.h"
#endif
#include "TapProject.h"

MainWindow* MainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    // Initialize command framework

    // Register default command features (add/delete item in list)

    createActions();
    createDockPanels();

    buildTestModel();
    setPdmRoot(m_project);

    sm_mainWindowInstance = this;
    caf::SelectionManager::instance()->setPdmRootObject(m_project);

#ifdef TAP_USE_COMMAND_FRAMEWORK
    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem(true);
    undoView->setStack(caf::CmdExecCommandManager::instance()->undoStack());
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("PdmTreeView - controls property view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTreeView = new caf::PdmUiTreeView(dockWidget);
        dockWidget->setWidget(m_pdmUiTreeView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafPropertyView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("PdmTreeView2  - controls table view", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTreeView2 = new caf::PdmUiTreeView(dockWidget);
        m_pdmUiTreeView2->enableDefaultContextMenu(true);
        m_pdmUiTreeView2->enableSelectionManagerUpdating(true);
        dockWidget->setWidget(m_pdmUiTreeView2);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("cafTableView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiTableView = new caf::PdmUiTableView(dockWidget);

        dockWidget->setWidget(m_pdmUiTableView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    {
        QDockWidget* dockWidget = new QDockWidget("Undo stack", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        undoView = new QUndoView(this);
        dockWidget->setWidget(undoView);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::buildTestModel()
{
    m_project = new TapProject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setPdmRoot(caf::PdmObjectHandle* pdmRoot)
{
    caf::PdmUiObjectHandle* uiObject = uiObj(pdmRoot);
    if (uiObject)
    {
        m_pdmUiTreeView->setPdmItem(uiObject);
    }

    connect(m_pdmUiTreeView, SIGNAL(selectionChanged()), SLOT(slotSimpleSelectionChanged()));

    // Set up test of using a field as a root item
    // Hack, because we know that pdmRoot is a PdmObjectGroup ...

    std::vector<caf::PdmFieldHandle*> fields;
    pdmRoot->fields(fields);
    if (fields.size())
    {
        caf::PdmFieldHandle*   field         = fields[0];
        caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if (uiFieldHandle)
        {
            m_pdmUiTreeView2->setPdmItem(uiFieldHandle);
            uiFieldHandle->updateConnectedEditors();
        }
    }

    if (uiObject)
    {
        m_pdmUiTreeView2->setPdmItem(uiObject);
    }

    connect(m_pdmUiTreeView2, SIGNAL(selectionChanged()), SLOT(slotShowTableView()));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_pdmUiTreeView->setPdmItem(NULL);
    m_pdmUiTreeView2->setPdmItem(NULL);
    m_pdmUiPropertyView->showProperties(NULL);
    m_pdmUiTableView->setChildArrayField(NULL);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    if (m_project)
    {
        delete m_project;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow* MainWindow::instance()
{
    return sm_mainWindowInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createActions()
{
    // Create actions
    QAction* editInsert    = new QAction("&Insert", this);
    QAction* editRemove    = new QAction("&Remove", this);
    QAction* editRemoveAll = new QAction("Remove all", this);

    connect(editInsert, SIGNAL(triggered()), SLOT(slotInsert()));
    connect(editRemove, SIGNAL(triggered()), SLOT(slotRemove()));
    connect(editRemoveAll, SIGNAL(triggered()), SLOT(slotRemoveAll()));

    // Create menus
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(editInsert);
    editMenu->addAction(editRemove);
    editMenu->addAction(editRemoveAll);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotInsert()
{
    /*
        std::vector<caf::PdmUiItem*> selection;
        m_pdmUiTreeView->selectedUiItems(selection);

        for (size_t i = 0; i < selection.size(); ++i)
        {
            caf::PdmUiFieldHandle* uiFh = dynamic_cast<caf::PdmUiFieldHandle*>(selection[i]);
            caf::PdmChildArrayField< caf::PdmObjectHandle*> * field = NULL;

            if (uiFh) field = dynamic_cast<caf::PdmChildArrayField< caf::PdmObjectHandle*> *>(uiFh->fieldHandle());
            









            if (field)
            {
                field->push_back(new DemoPdmObject);
                field->capability<caf::PdmUiFieldHandle>()->updateConnectedEditors();

                return;
            }
            #if 0
            caf::PdmChildArrayFieldHandle* listField = NULL;

            if (uiFh) listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>(uiFh->fieldHandle());

            if (listField)
            {
                caf::PdmObjectHandle* obj = listField->createAppendObject();
                listField->capability<caf::PdmUiFieldHandle>()->updateConnectedEditors();
            }
            #endif
        }
    */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);

    for (size_t i = 0; i < selection.size(); ++i)
    {
        caf::PdmObjectHandle* obj = dynamic_cast<caf::PdmObjectHandle*>(selection[i]);
        if (obj)
        {
            caf::PdmFieldHandle* field = obj->parentField();

            // Ordering is important

            field->removeChildObject(obj);

            // Delete object
            delete obj;

            // Update editors
            field->uiCapability()->updateConnectedEditors();

            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemoveAll() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSimpleSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);
    caf::PdmObjectHandle*          obj       = NULL;
    caf::PdmChildArrayFieldHandle* listField = NULL;

    if (selection.size())
    {
        caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>(selection[0]);
        if (pdmUiObj) obj = pdmUiObj->objectHandle();
    }

    m_pdmUiPropertyView->showProperties(obj);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotShowTableView()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView2->selectedUiItems(selection);
    caf::PdmObjectHandle*          obj       = NULL;
    caf::PdmChildArrayFieldHandle* listField = NULL;

    if (selection.size())
    {
        caf::PdmUiItem* pdmUiItem = selection[0];

        caf::PdmUiFieldHandle* guiField = dynamic_cast<caf::PdmUiFieldHandle*>(pdmUiItem);

        if (guiField) listField = dynamic_cast<caf::PdmChildArrayFieldHandle*>(guiField->fieldHandle());

        if (listField)
        {
            if (!listField->hasSameFieldCountForAllObjects())
            {
                listField = NULL;
            }
        }
    }

    m_pdmUiTableView->setChildArrayField(listField);

    if (listField)
    {
        listField->uiCapability()->updateConnectedEditors();
    }
}

// clang-format on
