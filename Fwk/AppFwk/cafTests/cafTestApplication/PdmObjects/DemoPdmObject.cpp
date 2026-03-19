#include "DemoPdmObject.h"

#include "../MenuItemProducer.h"

#include "SmallDemoPdmObject.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableView.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiValueRangeEditor.h"

CAF_PDM_SOURCE_INIT( DemoPdmObject, "DemoPdmObject" );

DemoPdmObject::DemoPdmObject()
{
    CAF_PDM_InitObject( "Demo Object",
                        "",
                        "This object is a demo of the CAF framework",
                        "This object is a demo of the CAF framework" );

    CAF_PDM_InitField( &m_toggleField, "Toggle", false, "Toggle Field", "", "Toggle Field tooltip", " Toggle Field whatsthis" );

    CAF_PDM_InitField( &m_applyAutoOnChildObjectFields, "ApplyAutoValue", false, "Apply Auto Values" );
    m_applyAutoOnChildObjectFields.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_applyAutoOnChildObjectFields.uiCapability()->setAttribute( caf::PdmUiPushButtonEditor::Keys::BUTTON_TEXT,
                                                                 "Apply Auto Values" );

    CAF_PDM_InitField( &m_updateAutoValues, "UpdateAutoValue", false, "Update Auto Values" );
    m_updateAutoValues.uiCapability()->setUiEditorTypeName( caf::PdmUiPushButtonEditor::uiEditorTypeName() );
    m_updateAutoValues.uiCapability()->setAttribute( caf::PdmUiPushButtonEditor::Keys::BUTTON_TEXT, "Update Auto Values" );

    CAF_PDM_InitField( &m_doubleField,
                       "BigNumber",
                       0.0,
                       "Big Number",
                       "",
                       "Enter a big number here",
                       "This is a place you can enter a big real value if you want" );
    CAF_PDM_InitField( &m_intField,
                       "IntNumber",
                       0,
                       "Small Number",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_sizeField, "SizeField", size_t( 0 ), "Size Field", "", "", "" );
    CAF_PDM_InitField( &m_boolField,
                       "BooleanValue",
                       false,
                       "Boolean:",
                       "",
                       "Boolean:Enter some small number here",
                       "Boolean:This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_textField, "TextField", QString( "Demo Object Description Field" ), "Description Field", "", "", "" );
    CAF_PDM_InitField( &m_filePath, "FilePath", QString( "" ), "Filename", "", "", "" );
    CAF_PDM_InitField( &m_longText, "LongText", QString( "Test text" ), "Long Text", "", "", "" );

    CAF_PDM_InitField( &m_minMaxSlider, "MinMaxSlider", std::make_pair( 2.5, 10.1 ), "Min max slider", "", "", "" );
    m_minMaxSlider.uiCapability()->setUiEditorTypeName( caf::PdmUiValueRangeEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_multiSelectList, "MultiSelect", "Selection List", "", "List", "This is a multi selection list" );
    CAF_PDM_InitFieldNoDefault( &m_objectList, "ObjectList", "Objects list Field", "", "List", "This is a list of PdmObjects" );
    CAF_PDM_InitFieldNoDefault( &m_objectListOfSameType,
                                "m_objectListOfSameType",
                                "Same type Objects list Field",
                                "",
                                "Same type List",
                                "Same type list of PdmObjects" );
    m_objectListOfSameType.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_objectListOfSameType.uiCapability()->setCustomContextMenuEnabled( true );
    CAF_PDM_InitFieldNoDefault( &m_ptrField, "m_ptrField", "PtrField", "", "Same type List", "Same type list of PdmObjects" );

    m_filePath.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
    m_filePath.capability<caf::PdmUiFieldHandle>()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_longText.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_longText.capability<caf::PdmUiFieldHandle>()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    m_menuItemProducer = new MenuItemProducer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DemoPdmObject::buildTestData()
{
    m_textField = "Mitt Demo Obj";

    DemoPdmObject* demoObj2 = new DemoPdmObject;
    m_objectList.push_back( demoObj2 );
    m_objectList.push_back( new SmallDemoPdmObjectA() );
    SmallDemoPdmObject* smallObj3 = new SmallDemoPdmObject();
    m_objectList.push_back( smallObj3 );
    m_objectList.push_back( new SmallDemoPdmObject() );

    m_objectListOfSameType.push_back( new SmallDemoPdmObjectA() );
    m_objectListOfSameType.push_back( new SmallDemoPdmObjectA() );
    m_objectListOfSameType.push_back( new SmallDemoPdmObjectA() );
    m_objectListOfSameType.push_back( new SmallDemoPdmObjectA() );

    demoObj2->m_objectList.push_back( new SmallDemoPdmObjectA() );
    demoObj2->m_objectList.push_back( new SmallDemoPdmObjectA() );
    demoObj2->m_objectList.push_back( new SmallDemoPdmObject() );

    delete smallObj3;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* DemoPdmObject::userDescriptionField()
{
    return &m_textField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* DemoPdmObject::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DemoPdmObject::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_sizeField );

    uiOrdering.add( &m_applyAutoOnChildObjectFields );
    uiOrdering.add( &m_updateAutoValues );

    uiOrdering.add( &m_minMaxSlider );

    uiOrdering.add( &m_objectListOfSameType );
    uiOrdering.add( &m_ptrField );
    uiOrdering.add( &m_boolField );
    caf::PdmUiGroup* group1 = uiOrdering.addNewGroup( "Name1" );
    group1->add( &m_doubleField );
    caf::PdmUiGroup* group2 = uiOrdering.addNewGroup( "Name2" );
    group2->add( &m_intField );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> DemoPdmObject::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( &m_multiSelectList == fieldNeedingOptions )
    {
        options.push_back( caf::PdmOptionItemInfo( "Choice 1", "Choice1" ) );
        options.push_back( caf::PdmOptionItemInfo( "Choice 2", "Choice2" ) );
        options.push_back( caf::PdmOptionItemInfo( "Choice 3", "Choice3" ) );
        options.push_back( caf::PdmOptionItemInfo( "Choice 4", "Choice4" ) );
        options.push_back( caf::PdmOptionItemInfo( "Choice 5", "Choice5" ) );
        options.push_back( caf::PdmOptionItemInfo( "Choice 6", "Choice6" ) );
    }

    if ( &m_ptrField == fieldNeedingOptions )
    {
        for ( size_t i = 0; i < m_objectListOfSameType.size(); ++i )
        {
            caf::PdmUiObjectHandle* uiObject = caf::uiObj( m_objectListOfSameType[i] );
            if ( uiObject )
            {
                options.push_back( caf::PdmOptionItemInfo( uiObject->uiName(),
                                                           QVariant::fromValue( caf::PdmPointer<caf::PdmObjectHandle>(
                                                               m_objectListOfSameType[i] ) ) ) );
            }
        }
    }

    return options;
}

void DemoPdmObject::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    if ( changedField == &m_toggleField )
    {
        std::cout << "Toggle Field changed" << std::endl;
    }

    static int counter = 0;
    counter++;
    double doubleValue = 1.23456 + counter;
    int    intValue    = -1213141516 + counter;
    auto   enumValue   = SmallDemoPdmObjectA::TestEnumType::T2;

    if ( changedField == &m_applyAutoOnChildObjectFields )
    {
        auto objs = descendantsIncludingThisOfType<SmallDemoPdmObjectA>();
        for ( auto obj : objs )
        {
            obj->enableAutoValueForDouble( doubleValue );
            obj->enableAutoValueForInt( intValue );
            obj->enableAutoValueForTestEnum( enumValue );
        }

        m_applyAutoOnChildObjectFields = false;
    }

    if ( changedField == &m_updateAutoValues )
    {
        auto objs = descendantsIncludingThisOfType<SmallDemoPdmObjectA>();
        for ( auto obj : objs )
        {
            obj->setAutoValueForDouble( doubleValue );
            obj->setAutoValueForInt( intValue );
            obj->setAutoValueForTestEnum( enumValue );
        }

        m_updateAutoValues = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DemoPdmObject::onEditorWidgetsCreated()
{
    for ( auto e : m_longText.uiCapability()->connectedEditors() )
    {
        caf::PdmUiTextEditor* textEditor = dynamic_cast<caf::PdmUiTextEditor*>( e );
        if ( !textEditor ) continue;

        QWidget* containerWidget = textEditor->editorWidget();
        if ( !containerWidget ) continue;

        for ( auto qObj : containerWidget->children() )
        {
            QTextEdit* textEdit = dynamic_cast<QTextEdit*>( qObj );
            if ( textEdit )
            {
                m_menuItemProducer->attachTextEdit( textEdit );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void DemoPdmObject::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                             QMenu*                     menu,
                                             QWidget*                   fieldEditorWidget )
{
    if ( fieldNeedingMenu == &m_objectListOfSameType )
    {
        caf::PdmUiTableView::addActionsToMenu( menu, &m_objectListOfSameType );
    }
}
