#include "SmallDemoPdmObjectA.h"

#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QVariant>

#include <iostream>

CAF_PDM_SOURCE_INIT( SmallDemoPdmObjectA, "SmallDemoPdmObjectA" );

namespace caf
{
template <>
void AppEnum<SmallDemoPdmObjectA::TestEnumType>::setUp()
{
    addItem( SmallDemoPdmObjectA::TestEnumType::T1, "T1", "An A letter" );
    addItem( SmallDemoPdmObjectA::TestEnumType::T2, "T2", "A B letter" );
    addItem( SmallDemoPdmObjectA::TestEnumType::T3, "T3", "A B C letter" );
    setDefault( SmallDemoPdmObjectA::TestEnumType::T1 );
}

} // namespace caf

Q_DECLARE_METATYPE( caf::AppEnum<SmallDemoPdmObjectA::TestEnumType> );

SmallDemoPdmObjectA::SmallDemoPdmObjectA()
{
    CAF_PDM_InitObject( "Small Demo Object A",
                        "",
                        "This object is a demo of the CAF framework",
                        "This object is a demo of the CAF framework" );

    CAF_PDM_InitField( &m_toggleField,
                       "Toggle",
                       false,
                       "Toggle Field much text much text much much text much text muchmuch text much text "
                       "muchmuch "
                       "text much "
                       "text muchmuch text much text muchmuch text much text much",
                       "",
                       "Toggle Field tooltip",
                       " Toggle Field whatsthis" );

    CAF_PDM_InitField( &m_pushButtonField, "Push", false, "Button Field", "", "", " " );
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
    CAF_PDM_InitField( &m_textField, "TextField", QString( "Small Demo Object A" ), "Name Text Field", "", "", "" );
    CAF_PDM_InitField( &m_testEnumField, "TestEnumValue", caf::AppEnum<TestEnumType>( TestEnumType::T1 ), "EnumField", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_ptrField, "m_ptrField", "PtrField", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_proxyEnumField, "ProxyEnumValue", "ProxyEnum", "", "", "" );
    m_proxyEnumField.registerSetMethod( this, &SmallDemoPdmObjectA::setEnumMember );
    m_proxyEnumField.registerGetMethod( this, &SmallDemoPdmObjectA::enumMember );
    m_proxyEnumMember = TestEnumType::T2;
    m_proxyEnumField.uiCapability()->setAttribute<bool>( "showPreviousAndNextButtons", true );

    CAF_PDM_InitFieldNoDefault( &m_multipleAppEnum, "MultipleAppEnumValue", "MultipleAppEnumValue", "", "", "" );
    m_multipleAppEnum.capability<caf::PdmUiFieldHandle>()->setUiEditorTypeName(
        caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_highlightedEnum, "HighlightedEnum", "HighlightedEnum", "", "", "" );
    m_highlightedEnum.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* SmallDemoPdmObjectA::objectToggleField()
{
    return &m_toggleField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* SmallDemoPdmObjectA::userDescriptionField()
{
    return &m_textField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::migrateFieldContent( QString& fieldContent, caf::PdmFieldHandle* fieldHandle )
{
    if ( fieldHandle == &m_textField )
    {
        fieldContent = "Migrated Text Field Content";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue )
{
    if ( changedField == &m_toggleField )
    {
        std::cout << "Toggle Field changed" << std::endl;
    }
    else if ( changedField == &m_highlightedEnum )
    {
        std::cout << "Highlight value " << m_highlightedEnum().uiText().toStdString() << std::endl;
    }
    else if ( changedField == &m_pushButtonField )
    {
        std::cout << "Push Button pressed " << std::endl;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::setEnumMember( const caf::AppEnum<TestEnumType>& val )
{
    m_proxyEnumMember = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<SmallDemoPdmObjectA::TestEnumType> SmallDemoPdmObjectA::enumMember() const
{
    return m_proxyEnumMember;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::enableAutoValueForTestEnum( TestEnumType value )
{
    auto enumValue = static_cast<std::underlying_type_t<TestEnumType>>( value );
    m_testEnumField.uiCapability()->enableAndSetAutoValue( enumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::enableAutoValueForDouble( double value )
{
    m_doubleField.uiCapability()->enableAndSetAutoValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::enableAutoValueForInt( double value )
{
    m_intField.uiCapability()->enableAndSetAutoValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::setAutoValueForTestEnum( TestEnumType value )
{
    auto enumValue = static_cast<std::underlying_type_t<TestEnumType>>( value );
    m_testEnumField.uiCapability()->setAutoValue( enumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::setAutoValueForDouble( double value )
{
    m_doubleField.uiCapability()->setAutoValue( value );
    m_doubleField.uiCapability()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::setAutoValueForInt( double value )
{
    m_intField.uiCapability()->setAutoValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> SmallDemoPdmObjectA::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( &m_ptrField == fieldNeedingOptions )
    {
        caf::PdmFieldHandle*               field   = this->parentField();
        std::vector<caf::PdmObjectHandle*> objects = field->children();
        for ( size_t i = 0; i < objects.size(); ++i )
        {
            QString userDesc;

            caf::PdmUiObjectHandle* uiObject = caf::uiObj( objects[i] );
            if ( uiObject )
            {
                if ( uiObject->userDescriptionField() )
                {
                    caf::PdmUiFieldHandle* uiFieldHandle = uiObject->userDescriptionField()->uiCapability();
                    if ( uiFieldHandle )
                    {
                        userDesc = uiFieldHandle->uiValue().toString();
                    }
                }

                options.push_back(
                    caf::PdmOptionItemInfo( uiObject->uiName() + "(" + userDesc + ")",
                                            QVariant::fromValue( caf::PdmPointer<caf::PdmObjectHandle>( objects[i] ) ) ) );
            }
        }
    }
    else if ( &m_multipleAppEnum == fieldNeedingOptions )
    {
        for ( size_t i = 0; i < caf::AppEnum<TestEnumType>::size(); ++i )
        {
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<TestEnumType>::uiTextFromIndex( i ),
                                                       caf::AppEnum<TestEnumType>::fromIndex( i ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                 QString                    uiConfigName,
                                                 caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_multipleAppEnum )
    {
        caf::PdmUiTreeSelectionEditorAttribute* attr = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->currentIndexFieldHandle = &m_highlightedEnum;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SmallDemoPdmObjectA::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTableViewPushButtonEditorAttribute* attr =
        dynamic_cast<caf::PdmUiTableViewPushButtonEditorAttribute*>( attribute );
    if ( attr )
    {
        attr->registerPushButtonTextForFieldKeyword( m_pushButtonField.keyword(), "Edit" );
    }
}
