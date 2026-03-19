
#include "cafPdmField.h"

#include "MainWindow.h"

#include "CustomObjectEditor.h"
#include "MenuItemProducer.h"
#include "PdmObjects/ApplicationEnum.h"
#include "PdmObjects/ColorTriplet.h"
#include "PdmObjects/DemoPdmObject.h"
#include "PdmObjects/DemoPdmObjectGroup.h"
#include "PdmObjects/LabelsAndHyperlinks.h"
#include "PdmObjects/LineEditAndPushButtons.h"
#include "PdmObjects/ManyGroups.h"
#include "PdmObjects/OptionalFields.h"
#include "PdmObjects/SingleEditorPdmObject.h"
#include "PdmObjects/SmallDemoPdmObject.h"
#include "PdmObjects/SmallDemoPdmObjectA.h"
#include "PdmObjects/SmallGridDemoPdmObject.h"
#include "PdmObjects/TamComboBox.h"
#include "PdmObjects/ValidationTest.h"
#include "WidgetLayoutTest.h"

#ifdef TAP_USE_COMMAND_FRAMEWORK
#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"
#include "cafCmdSelectionHelper.h"
#endif

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmDocument.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmReferenceHelper.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTableView.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QUndoView>

MainWindow* MainWindow::sm_mainWindowInstance = nullptr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    caf::PdmUiItem::enableExtraDebugText( true );

    // Initialize command framework

    // Register default command features (add/delete item in list)

    QPixmap pix;
    pix.load( ":/images/curvePlot.png" );

    m_plotLabel = new QLabel( this );
    m_plotLabel->setPixmap( pix.scaled( 250, 100 ) );

    m_smallPlotLabel = new QLabel( this );
    m_smallPlotLabel->setPixmap( pix.scaled( 100, 50 ) );

    createActions();
    createDockPanels();
    buildTestModel();

    setPdmRoot( m_testRoot );

    sm_mainWindowInstance = this;
    caf::SelectionManager::instance()->setPdmRootObject( m_testRoot );

#ifdef TAP_USE_COMMAND_FRAMEWORK
    caf::CmdExecCommandManager::instance()->enableUndoCommandSystem( true );
    undoView->setStack( caf::CmdExecCommandManager::instance()->undoStack() );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::createDockPanels()
{
    {
        QDockWidget* dockWidget = new QDockWidget( "PdmTreeView - controls property view", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiTreeView = new caf::PdmUiTreeView( dockWidget );
        dockWidget->setWidget( m_pdmUiTreeView );
        m_pdmUiTreeView->treeView()->setContextMenuPolicy( Qt::CustomContextMenu );

        QObject::connect( m_pdmUiTreeView->treeView(),
                          SIGNAL( customContextMenuRequested( const QPoint& ) ),
                          SLOT( slotCustomMenuRequestedForProjectTree( const QPoint& ) ) );

        m_pdmUiTreeView->treeView()->setSelectionMode( QAbstractItemView::ExtendedSelection );
        m_pdmUiTreeView->enableSelectionManagerUpdating( true );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "CustomObjectEditor", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_customObjectEditor = new caf::CustomObjectEditor;
        QWidget* w           = m_customObjectEditor->getOrCreateWidget( this );
        dockWidget->setWidget( w );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "cafPropertyView", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiPropertyView = new caf::PdmUiPropertyView( dockWidget );
        dockWidget->setWidget( m_pdmUiPropertyView );

        addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "PdmTreeView2  - controls table view", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiTreeView2 = new caf::PdmUiTreeView( dockWidget );
        m_pdmUiTreeView2->enableDefaultContextMenu( true );
        m_pdmUiTreeView2->enableSelectionManagerUpdating( true );
        dockWidget->setWidget( m_pdmUiTreeView2 );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "cafTableView", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        m_pdmUiTableView = new caf::PdmUiTableView( dockWidget );

        dockWidget->setWidget( m_pdmUiTableView );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
    }

    {
        QDockWidget* dockWidget = new QDockWidget( "Undo stack", this );
        dockWidget->setObjectName( "dockWidget" );
        dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

        undoView = new QUndoView( this );
        dockWidget->setWidget( undoView );

        addDockWidget( Qt::RightDockWidgetArea, dockWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::buildTestModel()
{
    m_testRoot = new DemoPdmObjectGroup;

    ManyGroups* manyGroups = new ManyGroups;
    m_testRoot->objects.push_back( manyGroups );

    DemoPdmObject* demoObject = new DemoPdmObject;
    m_testRoot->objects.push_back( demoObject );

    SmallDemoPdmObject* smallObj1 = new SmallDemoPdmObject;
    m_testRoot->objects.push_back( smallObj1 );

    SmallDemoPdmObjectA* smallObj2 = new SmallDemoPdmObjectA;
    m_testRoot->objects.push_back( smallObj2 );

    SmallGridDemoPdmObject* smallGridObj = new SmallGridDemoPdmObject;
    m_testRoot->objects.push_back( smallGridObj );

    SingleEditorPdmObject* singleEditorObj = new SingleEditorPdmObject;
    m_testRoot->objects.push_back( singleEditorObj );

    m_testRoot->objects.push_back( new LineEditAndPushButtons );
    m_testRoot->objects.push_back( new LabelsAndHyperlinks );
    m_testRoot->objects.push_back( new OptionalFields );

    auto tamComboBox = new TamComboBox;
    m_testRoot->objects.push_back( tamComboBox );

    demoObject->buildTestData();

    m_testRoot->objects.push_back( new ApplicationEnum );

    // Add validation test object
    ValidationTestObject* validationTest = new ValidationTestObject;
    m_testRoot->objects.push_back( validationTest );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setPdmRoot( caf::PdmObjectHandle* pdmRoot )
{
    caf::PdmUiObjectHandle* uiObject = uiObj( pdmRoot );

    m_pdmUiTreeView->setPdmItem( uiObject );

    connect( m_pdmUiTreeView, SIGNAL( selectionChanged() ), SLOT( slotSimpleSelectionChanged() ) );

    // Set up test of using a field as a root item
    // Hack, because we know that pdmRoot is a PdmObjectGroup ...

    std::vector<caf::PdmFieldHandle*> fields;
    if ( pdmRoot )
    {
        fields = pdmRoot->fields();
    }

    if ( fields.size() )
    {
        caf::PdmFieldHandle*   field         = fields[0];
        caf::PdmUiFieldHandle* uiFieldHandle = field->uiCapability();
        if ( uiFieldHandle )
        {
            m_pdmUiTreeView2->setPdmItem( uiFieldHandle );
            uiFieldHandle->updateConnectedEditors();
        }
    }

    m_pdmUiTreeView2->setPdmItem( uiObject );

    connect( m_pdmUiTreeView2, SIGNAL( selectionChanged() ), SLOT( slotShowTableView() ) );

    // Wire up ManyGroups object
    std::vector<ManyGroups*> obj;
    if ( pdmRoot )
    {
        obj = pdmRoot->descendantsIncludingThisOfType<ManyGroups>();
    }

    m_customObjectEditor->removeWidget( m_plotLabel );
    m_customObjectEditor->removeWidget( m_smallPlotLabel );

    if ( obj.size() == 1 )
    {
        m_customObjectEditor->setPdmObject( obj[0] );

        m_customObjectEditor->defineGridLayout( 5, 4 );

        m_customObjectEditor->addBlankCell( 0, 0 );
        m_customObjectEditor->addWidget( m_plotLabel, 0, 1, 1, 2 );
        m_customObjectEditor->addWidget( m_smallPlotLabel, 1, 2, 2, 1 );
    }
    else
    {
        m_customObjectEditor->setPdmObject( nullptr );
    }

    m_customObjectEditor->updateUi();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::setTreeViewSelection( caf::PdmObjectHandle* obj )
{
    if ( auto uiObj = caf::uiObj( obj ) )
    {
        m_pdmUiTreeView->selectAsCurrentItem( uiObj );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* MainWindow::root() const
{
    return m_testRoot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_pdmUiTreeView->setPdmItem( nullptr );
    m_pdmUiTreeView2->setPdmItem( nullptr );
    m_pdmUiPropertyView->showProperties( nullptr );
    m_pdmUiTableView->setChildArrayField( nullptr );

    releaseTestData();

    delete m_pdmUiTreeView;
    delete m_pdmUiTreeView2;
    delete m_pdmUiPropertyView;
    delete m_pdmUiTableView;
    delete m_customObjectEditor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    if ( m_testRoot )
    {
        m_testRoot->objects.deleteChildren();
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
    {
        QAction* loadAction = new QAction( "Load Project", this );
        QAction* saveAction = new QAction( "Save Project", this );

        connect( loadAction, SIGNAL( triggered() ), SLOT( slotLoadProject() ) );
        connect( saveAction, SIGNAL( triggered() ), SLOT( slotSaveProject() ) );

        QMenu* menu = menuBar()->addMenu( "&File" );
        menu->addAction( loadAction );
        menu->addAction( saveAction );
    }

    {
        QAction* editInsert    = new QAction( "&Insert", this );
        QAction* editRemove    = new QAction( "&Remove", this );
        QAction* editRemoveAll = new QAction( "Remove all", this );

        connect( editInsert, SIGNAL( triggered() ), SLOT( slotInsert() ) );
        connect( editRemove, SIGNAL( triggered() ), SLOT( slotRemove() ) );
        connect( editRemoveAll, SIGNAL( triggered() ), SLOT( slotRemoveAll() ) );

        QMenu* menu = menuBar()->addMenu( "&Edit" );
        menu->addAction( editInsert );
        menu->addAction( editRemove );
        menu->addAction( editRemoveAll );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotInsert()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems( selection );

    for ( size_t i = 0; i < selection.size(); ++i )
    {
        caf::PdmUiFieldHandle*                          uiFh  = dynamic_cast<caf::PdmUiFieldHandle*>( selection[i] );
        caf::PdmChildArrayField<caf::PdmObjectHandle*>* field = nullptr;

        if ( uiFh ) field = dynamic_cast<caf::PdmChildArrayField<caf::PdmObjectHandle*>*>( uiFh->fieldHandle() );

        if ( field )
        {
            field->push_back( new DemoPdmObject );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotRemove()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems( selection );

    for ( size_t i = 0; i < selection.size(); ++i )
    {
        caf::PdmObjectHandle* obj = dynamic_cast<caf::PdmObjectHandle*>( selection[i] );
        if ( obj )
        {
            caf::PdmFieldHandle* field = obj->parentField();

            // Ordering is important

            field->removeChild( obj );

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
void MainWindow::slotRemoveAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSimpleSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems( selection );
    caf::PdmObjectHandle* obj = nullptr;

    if ( selection.size() )
    {
        caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>( selection[0] );
        if ( pdmUiObj ) obj = pdmUiObj->objectHandle();
    }

    m_pdmUiPropertyView->showProperties( obj );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotShowTableView()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView2->selectedUiItems( selection );
    caf::PdmUiFieldHandle*         uiFieldHandle         = nullptr;
    caf::PdmChildArrayFieldHandle* childArrayFieldHandle = nullptr;

    if ( selection.size() )
    {
        caf::PdmUiItem* pdmUiItem = selection[0];

        uiFieldHandle = dynamic_cast<caf::PdmUiFieldHandle*>( pdmUiItem );
        if ( uiFieldHandle )
        {
            childArrayFieldHandle = dynamic_cast<caf::PdmChildArrayFieldHandle*>( uiFieldHandle->fieldHandle() );
        }

        if ( childArrayFieldHandle )
        {
            if ( !childArrayFieldHandle->hasSameFieldCountForAllObjects() )
            {
                uiFieldHandle         = nullptr;
                childArrayFieldHandle = nullptr;
            }
        }
    }

    m_pdmUiTableView->setChildArrayField( childArrayFieldHandle );

    if ( uiFieldHandle )
    {
        uiFieldHandle->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotLoadProject()
{
    QString fileName = QFileDialog::getOpenFileName( nullptr,
                                                     tr( "Open Project File" ),
                                                     "test.proj",
                                                     "Project Files (*.proj);;All files(*.*)" );
    if ( !fileName.isEmpty() )
    {
        setPdmRoot( nullptr );
        releaseTestData();

        m_testRoot = new DemoPdmObjectGroup;
        m_testRoot->setFileName( fileName );
        m_testRoot->readFile();
        m_testRoot->resolveReferencesRecursively();
        m_testRoot->initAfterReadRecursively();

        setPdmRoot( m_testRoot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSaveProject()
{
    QString fileName = QFileDialog::getSaveFileName( nullptr,
                                                     tr( "Save Project File" ),
                                                     "test.proj",
                                                     "Project Files (*.proj);;All files(*.*)" );
    if ( !fileName.isEmpty() )
    {
        m_testRoot->setFileName( fileName );
        m_testRoot->writeFile();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MainWindow::slotCustomMenuRequestedForProjectTree( const QPoint& )
{
    QObject*   senderObj = this->sender();
    QTreeView* treeView  = dynamic_cast<QTreeView*>( senderObj );
    if ( treeView )
    {
        QMenu menu;
        caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget( m_pdmUiTreeView );

        caf::CmdFeatureMenuBuilder menuBuilder;

        menuBuilder << "cafToggleItemsOnFeature";
        menuBuilder << "cafToggleItemsOffFeature";
        menuBuilder << "cafToggleItemsFeature";
        menuBuilder << "Separator";
        menuBuilder << "cafToggleItemsOnOthersOffFeature";

        menuBuilder.appendToMenu( &menu );

        menu.exec( QCursor::pos() );
        caf::CmdFeatureManager::instance()->setCurrentContextMenuTargetWidget( nullptr );
    }
}
