
#include "cafPdmField.h"

#include "MainWindow.h"

#include "CustomObjectEditor.h"
#include "ManyGroups.h"
#include "WidgetLayoutTest.h"

#include <QDockWidget>
#include <QTreeView>
#include <QAction>
#include <QMenuBar>
#include <QUndoView>


#include "cafAppEnum.h"

#ifdef TAP_USE_COMMAND_FRAMEWORK
#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafCmdFeatureManager.h"
#endif 

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
#include "cafPdmUiTreeSelectionEditor.h"



class DemoPdmObjectGroup : public caf::PdmObjectCollection
{
    CAF_PDM_HEADER_INIT;
public:

    DemoPdmObjectGroup() 
    {
        objects.uiCapability()->setUiHidden(true);
        
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

        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Add Items To Multi Select", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want" );
        CAF_PDM_InitField(&m_intField, "IntNumber", 0,  "Small Number", "", "Enter some small number here", "This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString(""), "Text", "", "Text tooltip", "This is a place you can enter a small integer value if you want");

        m_proxyDoubleField.registerSetMethod(this, &SmallDemoPdmObject::setDoubleMember);
        m_proxyDoubleField.registerGetMethod(this, &SmallDemoPdmObject::doubleMember);
        CAF_PDM_InitFieldNoDefault(&m_proxyDoubleField, "ProxyDouble", "Proxy Double", "", "", "");

        m_proxyDoubleField = 0;
        if (!(m_proxyDoubleField == 3)) { std::cout << "Double is not 3 " << std::endl; }
    
        CAF_PDM_InitFieldNoDefault(&m_multiSelectList, "SelectedItems", "Multi Select Field", "", "", "");
        m_multiSelectList.xmlCapability()->setIOReadable(false);
        m_multiSelectList.xmlCapability()->setIOWritable(false);
        m_multiSelectList.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());

        m_multiSelectList.v().push_back("First");
        m_multiSelectList.v().push_back("Second");
        m_multiSelectList.v().push_back("Third");
    }


    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
    caf::PdmProxyValueField<double> m_proxyDoubleField;

    caf::PdmField<std::vector<QString> > m_multiSelectList;


    caf::PdmField<bool>     m_toggleField;
    virtual caf::PdmFieldHandle* objectToggleField() 
    {
        return &m_toggleField;
    }

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) 
    {
        if (changedField == &m_toggleField)
        {
            std::cout << "Toggle Field changed" << std::endl;
        }
    }

    void setDoubleMember(const double& d) { m_doubleMember = d; std::cout << "setDoubleMember" << std::endl; }
    double doubleMember() const { std::cout << "doubleMember" << std::endl; return m_doubleMember; }


    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override
    {
        QList<caf::PdmOptionItemInfo> options;

        if (fieldNeedingOptions == &m_multiSelectList)
        {
            QString text;

            text = "First";
            options.push_back(caf::PdmOptionItemInfo(text, text));

            text = "Second";
            options.push_back(caf::PdmOptionItemInfo::createHeader(text, false, QIcon(QString(":/images/win/textbold.png"))));

            {
                text = "Second_a";
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text, true);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            {
                text = "Second_b";
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text, false, QIcon(QString(":/images/win/filenew.png")));
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            int additionalSubItems = 2;
            for (auto i = 0; i < additionalSubItems; i++)
            {
                text = "Second_b_" + QString::number(i);
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            static int s_additionalSubItems = 0;
            if (m_toggleField())
            {
                s_additionalSubItems++;
            }
            for (auto i = 0; i < s_additionalSubItems; i++)
            {
                text = "Second_b_" + QString::number(i);
                caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo(text, text);
                itemInfo.setLevel(1);
                options.push_back(itemInfo);
            }

            text = "Third";
            options.push_back(caf::PdmOptionItemInfo(text, text));

            text = "Fourth";
            options.push_back(caf::PdmOptionItemInfo(text, text));
        }

        return options;

    }

private:
    double m_doubleMember;

protected:
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override
    {
        uiOrdering.add(&m_doubleField);
        uiOrdering.add(&m_intField);

        QString dynamicGroupName = QString("Dynamic Group Text (%1)").arg(m_intField);

        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword(dynamicGroupName, "MyTest");
        group->add(&m_textField);
        group->add(&m_proxyDoubleField);
    }
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

        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField, "BigNumber", 0.0, "Big Number", "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField, "IntNumber", 0,  "Small Number", "", "Enter some small number here","This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField, "TextField", QString("Small Demo Object A"), "Name Text Field", "", "", "");
        CAF_PDM_InitField(&m_testEnumField, "TestEnumValue", caf::AppEnum<TestEnumType>(T1), "EnumField", "", "", "");
        CAF_PDM_InitFieldNoDefault(&m_ptrField, "m_ptrField", "PtrField", "", "", "");

        CAF_PDM_InitFieldNoDefault(&m_proxyEnumField, "ProxyEnumValue", "ProxyEnum", "", "", "");
        m_proxyEnumField.registerSetMethod(this, &SmallDemoPdmObjectA::setEnumMember);
        m_proxyEnumField.registerGetMethod(this, &SmallDemoPdmObjectA::enumMember);
        m_proxyEnumMember = T2;

        m_testEnumField.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

        CAF_PDM_InitFieldNoDefault(&m_multipleAppEnum, "MultipleAppEnumValue", "MultipleAppEnumValue", "", "", "");
        m_multipleAppEnum.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
        CAF_PDM_InitFieldNoDefault(&m_highlightedEnum, "HighlightedEnum", "HighlightedEnum", "", "", "");
        m_highlightedEnum.uiCapability()->setUiHidden(true);
    }

    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
    caf::PdmField< caf::AppEnum<TestEnumType> > m_testEnumField;
    caf::PdmPtrField<SmallDemoPdmObjectA*> m_ptrField;

    caf::PdmProxyValueField< caf::AppEnum<TestEnumType> > m_proxyEnumField;
    void setEnumMember(const caf::AppEnum<TestEnumType>& val) { m_proxyEnumMember = val; }
    caf::AppEnum<TestEnumType> enumMember() const { return m_proxyEnumMember; }
    TestEnumType m_proxyEnumMember;


    // vector of app enum
    caf::PdmField< std::vector< caf::AppEnum<TestEnumType> > > m_multipleAppEnum;
    caf::PdmField< caf::AppEnum<TestEnumType> > m_highlightedEnum;

    caf::PdmField<bool>     m_toggleField;
    virtual caf::PdmFieldHandle* objectToggleField() 
    {
        return &m_toggleField;
    }

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) 
    {
        if (changedField == &m_toggleField)
        {
            std::cout << "Toggle Field changed" << std::endl;
        }
        else if (changedField == &m_highlightedEnum)
        {
            std::cout << "Highlight value " << m_highlightedEnum() <<  std::endl;
        }
    }

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
    {
        QList<caf::PdmOptionItemInfo> options;
       
        if (&m_ptrField == fieldNeedingOptions)
        {
            caf::PdmFieldHandle* field;
            std::vector<caf::PdmObjectHandle*> objects;
            field = this->parentField();

            field->childObjects(&objects);

            for (size_t i = 0; i < objects.size(); ++i)
            {
                QString userDesc;

                caf::PdmUiObjectHandle* uiObject = caf::uiObj(objects[i]);
                if (uiObject)
                {
                    if (uiObject->userDescriptionField())
                    {
                        caf::PdmUiFieldHandle* uiFieldHandle = uiObject->userDescriptionField()->uiCapability();
                        if (uiFieldHandle)
                        {
                            userDesc = uiFieldHandle->uiValue().toString();
                        }
                    }

                    options.push_back(caf::PdmOptionItemInfo(uiObject->uiName() + "(" + userDesc + ")", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(objects[i]))));
                }
            }
        }
        else if (&m_multipleAppEnum == fieldNeedingOptions)
        {
            for (size_t i = 0; i < caf::AppEnum<TestEnumType>::size(); ++i)
            {
                options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<TestEnumType>::uiTextFromIndex(i), caf::AppEnum<TestEnumType>::fromIndex(i)));
            }
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual caf::PdmFieldHandle* userDescriptionField()
    {
        return &m_textField;
    }

protected:
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override
    {
        if (field == &m_multipleAppEnum)
        {
            caf::PdmUiTreeSelectionEditorAttribute* attr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>(attribute);
            if (attr)
            {
                attr->fieldToReceiveCurrentFieldValue = m_highlightedEnum.uiCapability();
            }
        }
    }

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
            
        CAF_PDM_InitField(&m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis");
        CAF_PDM_InitField(&m_doubleField,   "BigNumber",    0.0,        "Big Number",   "", "Enter a big number here", "This is a place you can enter a big real value if you want");
        CAF_PDM_InitField(&m_intField,      "IntNumber",    0,          "Small Number", "", "Enter some small number here",  "This is a place you can enter a small integer value if you want" );
        CAF_PDM_InitField(&m_boolField,     "BooleanValue", false,      "Boolean:" ,    "", "Boolean:Enter some small number here", "Boolean:This is a place you can enter a small integer value if you want");
        CAF_PDM_InitField(&m_textField,     "TextField",    QString("Demo Object Description Field"), "Description Field",            "", "", "");
        CAF_PDM_InitField(&m_filePath,      "FilePath",    QString(""),  "Filename",            "", "", "");
        CAF_PDM_InitField(&m_longText,      "LongText",    QString("Test text"),  "Long Text",            "", "", "");

        CAF_PDM_InitFieldNoDefault(&m_multiSelectList, "MultiSelect", "Selection List", "", "List" , "This is a multi selection list"  );
        CAF_PDM_InitFieldNoDefault(&m_objectList, "ObjectList", "Objects list Field", "", "List" , "This is a list of PdmObjects"  );
        CAF_PDM_InitFieldNoDefault(&m_objectListOfSameType, "m_objectListOfSameType", "Same type Objects list Field", "", "Same type List" , "Same type list of PdmObjects"  );
        CAF_PDM_InitFieldNoDefault(&m_ptrField, "m_ptrField", "PtrField", "", "Same type List", "Same type list of PdmObjects");

        m_filePath.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
        m_filePath.capability<caf::PdmUiFieldHandle>()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
        m_longText.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
        m_longText.capability<caf::PdmUiFieldHandle>()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
    {
        uiOrdering.add(&m_ptrField);
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

        if (&m_ptrField == fieldNeedingOptions)
        {
            for (size_t i = 0; i < m_objectListOfSameType.size(); ++i)
            {
                caf::PdmUiObjectHandle* uiObject = caf::uiObj(m_objectListOfSameType[i]);
                if (uiObject)
                {
                    options.push_back(caf::PdmOptionItemInfo(uiObject->uiName(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(m_objectListOfSameType[i]))));
                }
            }
        }

        if (useOptionsOnly) *useOptionsOnly = true;

        return options;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual caf::PdmFieldHandle* userDescriptionField() 
    {
        return &m_textField;
    }



    // Fields
    caf::PdmField<bool>     m_boolField;
    caf::PdmField<double>   m_doubleField;
    caf::PdmField<int>      m_intField;
    caf::PdmField<QString>  m_textField;
    
    caf::PdmField<QString>  m_filePath;

    caf::PdmField<QString>  m_longText;
    caf::PdmField<std::vector<QString> >      m_multiSelectList;


    caf::PdmChildArrayField< caf::PdmObjectHandle*  > m_objectList;
    caf::PdmChildArrayField< SmallDemoPdmObjectA*  > m_objectListOfSameType;
    caf::PdmPtrField<SmallDemoPdmObjectA*> m_ptrField;


    caf::PdmField<bool>     m_toggleField;
    virtual caf::PdmFieldHandle* objectToggleField() 
    {
        return &m_toggleField;
    }

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) 
    {
       if (changedField == &m_toggleField)
       {
           std::cout << "Toggle Field changed" << std::endl;
       }
    }

};

CAF_PDM_SOURCE_INIT(DemoPdmObject, "DemoPdmObject");



MainWindow* MainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MainWindow::MainWindow()
{
    // Initialize command framework

    // Register default command features (add/delete item in list)
 
    QPixmap pix;
    pix.load(":/images/curvePlot.png");

    m_plotLabel = new QLabel(this);
    m_plotLabel->setPixmap(pix.scaled(250, 100));

    m_smallPlotLabel = new QLabel(this);
    m_smallPlotLabel->setPixmap(pix.scaled(100, 50));

    createActions();
    createDockPanels();
    buildTestModel();

    setPdmRoot(m_testRoot);

    sm_mainWindowInstance = this;
    caf::SelectionManager::instance()->setPdmRootObject(m_testRoot);

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
        QDockWidget* dockWidget = new QDockWidget("CustomObjectEditor", this);
        dockWidget->setObjectName("dockWidget");
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        m_customObjectEditor = new caf::CustomObjectEditor;
        QWidget* w = m_customObjectEditor->getOrCreateWidget(this);
        dockWidget->setWidget(w);

        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
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
        m_pdmUiTableView->setSelectionRole(caf::SelectionManager::CURRENT);
        m_pdmUiTableView->enableDefaultContextMenu(true);

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
    m_testRoot = new DemoPdmObjectGroup;

    ManyGroups* manyGroups = new ManyGroups;
    m_testRoot->objects.push_back(manyGroups);

    DemoPdmObject* demoObject = new DemoPdmObject;
    m_testRoot->objects.push_back(demoObject);

    SmallDemoPdmObject* smallObj1 = new SmallDemoPdmObject;
    m_testRoot->objects.push_back(smallObj1);

    SmallDemoPdmObjectA* smallObj2 = new SmallDemoPdmObjectA;
    m_testRoot->objects.push_back(smallObj2);

    DemoPdmObject* demoObj2 = new DemoPdmObject;
   
    demoObject->m_textField = "Mitt Demo Obj";
    demoObject->m_objectList.push_back(demoObj2);
    demoObject->m_objectList.push_back(new SmallDemoPdmObjectA());
    SmallDemoPdmObject* smallObj3 = new SmallDemoPdmObject();
    demoObject->m_objectList.push_back(smallObj3);
    demoObject->m_objectList.push_back(new SmallDemoPdmObject());

    demoObject->m_objectListOfSameType.push_back(new SmallDemoPdmObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoPdmObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoPdmObjectA());
    demoObject->m_objectListOfSameType.push_back(new SmallDemoPdmObjectA());


    demoObj2->m_objectList.push_back(new SmallDemoPdmObjectA());
    demoObj2->m_objectList.push_back(new SmallDemoPdmObjectA());
    demoObj2->m_objectList.push_back(new SmallDemoPdmObject());
    
    delete smallObj3;
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
        caf::PdmFieldHandle* field = fields[0];
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

    // Wire up ManyGroups object
    std::vector<ManyGroups*> obj;
    pdmRoot->descendantsIncludingThisOfType(obj);
    if (obj.size() == 1)
    {
        m_customObjectEditor->defineGridLayout(5, 4);
        m_customObjectEditor->setPdmObject(obj[0]);

        m_customObjectEditor->addBlankCell(0, 0);
        m_customObjectEditor->addWidget(m_plotLabel, 0, 1, 1, 2);
        m_customObjectEditor->addWidget(m_smallPlotLabel, 1, 2, 2, 1);

        m_customObjectEditor->updateUi();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_pdmUiTreeView->setPdmItem(NULL);
    m_pdmUiTreeView2->setPdmItem(NULL);
    m_pdmUiPropertyView->showProperties(NULL);
    m_pdmUiTableView->setListField(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::releaseTestData()
{
    if (m_testRoot)
    {
        m_testRoot->objects.deleteAllChildObjects();
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
        caf::PdmObjectHandle* obj = dynamic_cast< caf::PdmObjectHandle*>(selection[i]);
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
void MainWindow::slotRemoveAll()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MainWindow::slotSimpleSelectionChanged()
{
    std::vector<caf::PdmUiItem*> selection;
    m_pdmUiTreeView->selectedUiItems(selection);
    caf::PdmObjectHandle* obj = NULL;
    caf::PdmChildArrayFieldHandle* listField = NULL;

    if (selection.size())
    {
        caf::PdmUiObjectHandle* pdmUiObj = dynamic_cast<caf::PdmUiObjectHandle*>( selection[0] );
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
    caf::PdmObjectHandle* obj = NULL;
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

    m_pdmUiTableView->setListField(listField);

    if (listField)
    {
        listField->uiCapability()->updateConnectedEditors();
    }
}
