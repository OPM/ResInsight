#include "SmallDemoPdmObject.h"

#include "cafIconProvider.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QMenu>
#include <QVariant>

#include <iostream>

CAF_PDM_SOURCE_INIT( SmallDemoPdmObject, "SmallDemoPdmObject" );

SmallDemoPdmObject::SmallDemoPdmObject()
{
    CAF_PDM_InitObject( "Small Demo Object",
                        ":/images/win/filenew.png",
                        "This object is a demo of the CAF framework",
                        "This object is a demo of the CAF framework" );

    CAF_PDM_InitField( &m_toggleField,
                       "Toggle",
                       false,
                       "Add Items To Multi Select",
                       "",
                       "Toggle Field tooltip",
                       " Toggle Field whatsthis" );
    CAF_PDM_InitField( &m_doubleField,
                       "BigNumber",
                       0.0,
                       "Big Number",
                       "",
                       "Enter a big number here",
                       "This is a place you can enter a big real value if you want" );
    m_doubleField.uiCapability()->setCustomContextMenuEnabled( true );

    CAF_PDM_InitField( &m_intField,
                       "IntNumber",
                       0,
                       "Small Number",
                       "",
                       "Enter some small number here",
                       "This is a place you can enter a small integer value if you want" );
    CAF_PDM_InitField( &m_textField,
                       "TextField",
                       QString( "" ),
                       "Text",
                       "",
                       "Text tooltip",
                       "This is a place you can enter a small integer value if you want" );

    CAF_PDM_InitFieldNoDefault( &m_booleanAndDoubleField, "BooleanAndDoubleField", "Checkable double" );
    CAF_PDM_InitFieldNoDefault( &m_booleanAndTextField, "BooleanAndTextField", "Checkable text" );
    m_booleanAndTextField.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );

    m_proxyDoubleField.registerSetMethod( this, &SmallDemoPdmObject::setDoubleMember );
    m_proxyDoubleField.registerGetMethod( this, &SmallDemoPdmObject::doubleMember );
    CAF_PDM_InitFieldNoDefault( &m_proxyDoubleField, "ProxyDouble", "Proxy Double" );

    CAF_PDM_InitFieldNoDefault( &m_colorTriplets, "colorTriplets", "color Triplets", "", "", "" );

    CAF_PDM_InitField( &m_fileName, "FileName", caf::FilePath( "filename" ), "File Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fileNameList, "FileNameList", "File Name List", "", "", "" );
    m_fileNameList.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    m_proxyDoubleField = 0;
    if ( !( m_proxyDoubleField == 3 ) )
    {
        std::cout << "Double is not 3 " << std::endl;
    }

    CAF_PDM_InitFieldNoDefault( &m_multiSelectList, "SelectedItems", "Multi Select Field", "", "", "" );
    m_multiSelectList.xmlCapability()->setIOReadable( false );
    m_multiSelectList.xmlCapability()->setIOWritable( false );
    m_multiSelectList.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    m_multiSelectList.v().push_back( "First" );
    m_multiSelectList.v().push_back( "Second" );
    m_multiSelectList.v().push_back( "Third" );

    m_colorTriplets.push_back( new ColorTriplet );
    m_colorTriplets.push_back( new ColorTriplet );
    m_colorTriplets.push_back( new ColorTriplet );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> SmallDemoPdmObject::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_multiSelectList )
    {
        QString text;

        text = "First";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );

        text = "Second";
        options.push_back(
            caf::PdmOptionItemInfo::createHeader( text, false, caf::IconProvider( ":/images/win/textbold.png" ) ) );

        {
            text                            = "Second_a";
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo( text, text, true );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        {
            text = "Second_b";
            caf::PdmOptionItemInfo itemInfo =
                caf::PdmOptionItemInfo( text, text, false, caf::IconProvider( ":/images/win/filenew.png" ) );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        int additionalSubItems = 2;
        for ( auto i = 0; i < additionalSubItems; i++ )
        {
            text                            = "Second_b_" + QString::number( i );
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        static int s_additionalSubItems = 0;
        if ( m_toggleField() )
        {
            s_additionalSubItems++;
        }
        for ( auto i = 0; i < s_additionalSubItems; i++ )
        {
            text                            = "Second_b_" + QString::number( i );
            caf::PdmOptionItemInfo itemInfo = caf::PdmOptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        text = "Third";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );

        text = "Fourth";
        options.push_back( caf::PdmOptionItemInfo( text, text ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* SmallDemoPdmObject::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObject::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    if ( changedField == &m_toggleField )
    {
        std::cout << "Toggle Field changed" << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObject::setDoubleMember( const double& d )
{
    m_doubleMember = d;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double SmallDemoPdmObject::doubleMember() const
{
    return m_doubleMember;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObject::defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu,
                                                  QMenu*                     menu,
                                                  QWidget*                   fieldEditorWidget )
{
    menu->addAction( "test" );
    menu->addAction( "other test <<>>" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObject::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_booleanAndDoubleField );
    uiOrdering.add( &m_booleanAndTextField );

    uiOrdering.add( &m_doubleField );
    uiOrdering.add( &m_intField );
    QString dynamicGroupName = QString( "Dynamic Group Text (%1)" ).arg( m_intField() );

    caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword( dynamicGroupName, "MyTest" );
    group->add( &m_textField );
    group->add( &m_proxyDoubleField );
}
