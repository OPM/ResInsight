
#include "MainWindow.h"
#include "WidgetLayoutTest.h"

#include <QDockWidget>
#include <QTreeView>
#include <QAction>
#include <QMenuBar>

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmDocument.h"
#include "cafAppEnum.h"
#include "cafUiTreeModelPdm.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTextEditor.h"



class DemoPdmObjectGroup: public caf::PdmObjectGroup
{
    CAF_PDM_HEADER_INIT;
public:

    DemoPdmObjectGroup() 
    {
        CAF_PDM_InitObject("Demo Object Group", "", "This group object is a demo of the CAF framework", "This group object is a demo of the CAF framework")
    }
};

CAF_PDM_SOURCE_INIT(DemoPdmObjectGroup, "DemoPdmObjectGroup");

class SmallDemoPdmObject: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    SmallDemoPdmObject() 
    {   
        CAF_PDM_InitObject("Small Demo Object", ":/images/win/filenew.png", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want" );
        CAF_PDM_InitField(&m_intField, "IntNumber", 0,  "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");
    }

    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
};

CAF_PDM_SOURCE_INIT(SmallDemoPdmObject, "SmallDemoPdmObject");


class SmallDemoPdmObjectA: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    enum TestEnumType
    {
        T1, T2, T3
    };


    SmallDemoPdmObjectA() 
    {   
        CAF_PDM_InitObject("Small Demo Object A", "", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");

        CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField, "IntNumber", 0,  "Small Number", "", "Enter some small number here","This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_testEnumField, "TestEnumValue", caf::AppEnum<TestEnumType>(T1), "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");

        m_testEnumField.setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
    }

    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
    caf::PdmField< caf::AppEnum<TestEnumType> > m_testEnumField;

};

CAF_PDM_SOURCE_INIT(SmallDemoPdmObjectA, "SmallDemoPdmObjectA");

namespace caf
{
    template<>
    void AppEnum<SmallDemoPdmObjectA::TestEnumType>::setUp()
    {
        addItem(SmallDemoPdmObjectA::T1,           "T1",         "An A letter");
        addItem(SmallDemoPdmObjectA::T2,           "T2",         "A B letter");
        addItem(SmallDemoPdmObjectA::T3,           "T3",         "A B C letter");
        setDefault(SmallDemoPdmObjectA::T1);

    }

}
Q_DECLARE_METATYPE(caf::AppEnum<SmallDemoPdmObjectA::TestEnumType>);




class DemoPdmObject: public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    DemoPdmObject() 
    {   
        CAF_PDM_InitObject( "Demo Object", "", "This object is a demo of the CAF framework", "This object is a demo of the CAF framework");
            
        CAF_PDM_InitField(&m_doubleField,   "BigNumber",    0.0,        "Big Number",   "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField,      "IntNumber",    0,          "Small Number", "", "Enter some small number here",  "This is a place you can enter a small integer value if you want" );
        CAF_PDM_InitField(&m_boolField,     "BooleanValue", false,      "Boolean:" ,    "", "Boolean:Enter some small number here", "Boolean:This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField,     "TextField",    QString(""), "",            "", "", "");
        CAF_PDM_InitField(&m_filePath,      "FilePath",    QString(""),  "Filename",            "", "", "");
        CAF_PDM_InitField(&m_longText,      "LongText",    QString("Test text"),  "Long Text",            "", "", "");

        CAF_PDM_InitFieldNoDefault(&m_multiSelectList, "MultiSelect", "Selection List", "", "List" , "This is a multi selection list"  );

        CAF_PDM_InitFieldNoDefault(&m_objectList, "ObjectList", "Objects list", "", "List" , "This is a list of PdmObjects"  );

        m_filePath.setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
        m_filePath.setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        m_longText.setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
        m_longText.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
    {
        uiOrdering.add(&m_boolField);
        caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Name1");
        group1->add(&m_doubleField);
        caf::PdmUiGroup* group2 = uiOrdering.addNewGroup("Name2"); 
        group2->add(&m_intField);
        caf::PdmUiGroup* group3 = group2->addNewGroup("Name3");
        group3->add(&m_textField);

        //uiConfig->add(&f3);
        //uiConfig->forgetRemainingFields();
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) 
    {
        QList<caf::PdmOptionItemInfo> options;
        if (&m_multiSelectList == fieldNeedingOptions)
        {

            options.push_back(caf::PdmOptionItemInfo("Choice 1", "Choice1"));
            options.push_back(caf::PdmOptionItemInfo("Choice 2", "Choice2"));
            options.push_back(caf::PdmOptionItemInfo("Choice 3", "Choice3"));
            options.push_back(caf::PdmOptionItemInfo("Choice 4", "Choice4"));
            options.push_back(caf::PdmOptionItemInfo("Choice 5", "Choice5"));
            options.push_back(caf::PdmOptionItemInfo("Choice 6", "Choice6"));

        }
        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    // Fields
    caf::PdmField<bool>     m_boolField;
    caf::PdmField<double>   m_doubleField;
    caf::PdmField<int>      m_intField;
    caf::PdmField<QString>  m_textField;
    
    caf::PdmField<QString>  m_filePath;

    caf::PdmField<QString>  m_longText;
    caf::PdmField<std::vector<QString> >      m_multiSelectList;


    caf::PdmPointersField< caf::PdmObject*  > m_objectList;
};

CAF_PDM_SOURCE_INIT(DemoPdmObject, "DemoPdmObject");



MainWindow* MainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    m_treeView = NULL;
    m_treeModelPdm = NULL;

    createActions();
    createDockPanels();

    buildTestModel();
    setPdmRoot(m_testRoot);

    sm_mainWindowInstance = this;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget("Workspace", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_treeView = new QTreeView(dockWidget);
        dockWidget->setWidget(m_treeView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
    /*
    {
        QDockWidget* dockWidget = new QDockWidget("WidgetLayoutTest", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        WidgetLayoutTest* widgetLayoutTest = new WidgetLayoutTest(dockWidget);
        dockWidget->setWidget(widgetLayoutTest);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
    */

    {
        QDockWidget* dockWidget = new QDockWidget("cafPropertyView", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_pdmUiPropertyView = new caf::PdmUiPropertyView(dockWidget);
        dockWidget->setWidget(m_pdmUiPropertyView);

        addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::buildTestModel()
{
    m_testRoot = new DemoPdmObjectGroup;
    
    DemoPdmObject* demoObject = new DemoPdmObject;
    m_testRoot->addObject(demoObject);

    SmallDemoPdmObject* smallObj1 = new SmallDemoPdmObject;
    m_testRoot->addObject(smallObj1);

    SmallDemoPdmObjectA* smallObj2 = new SmallDemoPdmObjectA;
    m_testRoot->addObject(smallObj2);

    demoObject->m_objectList.push_back(new DemoPdmObject);
    demoObject->m_objectList.push_back(new SmallDemoPdmObjectA());
    demoObject->m_objectList.push_back(new SmallDemoPdmObject());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::setPdmRoot(caf::PdmObject* pdmRoot)
{
    caf::PdmUiTreeItem* treeItemRoot = caf::UiTreeItemBuilderPdm::buildViewItems(NULL, 0, pdmRoot);

    if (!m_treeModelPdm) 
    {
        m_treeModelPdm = new caf::UiTreeModelPdm(this);
    }

    m_treeModelPdm->setTreeItemRoot(treeItemRoot);

    assert(m_treeView);
    m_treeView->setModel(m_treeModelPdm);

    if (treeItemRoot)
    {
        if (m_treeView->selectionModel())
        {
            connect(m_treeView->selectionModel(), SIGNAL(selectionChanged( const QItemSelection & , const QItemSelection & )), SLOT(slotSelectionChanged( const QItemSelection & , const QItemSelection & )));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    m_treeView->setModel(NULL);
    if (m_treeModelPdm)
    {
        delete m_treeModelPdm;
    }

    if (m_testRoot)
    {
        m_testRoot->deleteObjects();
        delete m_testRoot;
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
    QAction* editInsert     = new QAction("&Insert", this);
    QAction* editRemove     = new QAction("&Remove", this);
    QAction* editRemoveAll  = new QAction("Remove all", this);

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
    QModelIndex index = m_treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_treeView->model();

    if (!model->insertRow(0, index))
        return;

    QModelIndex child = model->index(0, 0, index);


// (
//     for (int column = 0; column < model->columnCount(index); ++column)
//     {
//         QModelIndex child = model->index(0, column, index);
//         model->setData(child, QVariant("[No data]"), Qt::EditRole);
//         if (!model->headerData(column, Qt::Horizontal).isValid())
//             model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"),
//             Qt::EditRole);
//     }
// )

    m_treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    QModelIndex index = m_treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = m_treeView->model();
    model->removeRow(index.row(), index.parent());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemoveAll()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected )
{
    if (selected.indexes().size() == 1)
    {
        QModelIndex mi = selected.indexes()[0];
        caf::PdmUiTreeItem* treeItem = m_treeModelPdm->getTreeItemFromIndex(mi);
        if (treeItem && treeItem->dataObject())
        {
            m_pdmUiPropertyView->showProperties(treeItem->dataObject());
        }
    }
    else
    {
        m_pdmUiPropertyView->showProperties(NULL);
    }
}
