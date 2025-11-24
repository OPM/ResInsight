
#include "ApplicationEnum.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiRadioButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

template <>
void caf::AppEnum<ApplicationEnum::MyEnumType>::setUp()
{
    addItem( ApplicationEnum::MyEnumType::T1, "T1", "T 1" );
    addItem( ApplicationEnum::MyEnumType::T2, "T2", "T 2" );
    addItem( ApplicationEnum::MyEnumType::T3, "T3", "T 3" );
    addItem( ApplicationEnum::MyEnumType::T4, "T4", "T 4" );
    addItem( ApplicationEnum::MyEnumType::T5, "T5", "T 5" );
    addItem( ApplicationEnum::MyEnumType::T6, "T5", "T 6" );
    addItem( ApplicationEnum::MyEnumType::T7, "T6", "T 7" );

    setDefault( ApplicationEnum::MyEnumType::T4 );
}

CAF_PDM_SOURCE_INIT( ApplicationEnum, "ApplicationEnum" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ApplicationEnum::ApplicationEnum()
{
    CAF_PDM_InitObject( "Application Enum", "", "", "" );

    // Enum field displaying all defined enums
    CAF_PDM_InitFieldNoDefault( &m_enumField, "EnumField", "All Enums" );

    // Enum field displaying a subset of the defined enums using the static function setEnumSubset()
    CAF_PDM_InitField( &m_enum2Field, "Enum2Field", MyEnumType::T6, "Subset using setEnumSubset()" );
    caf::AppEnum<MyEnumType>::setEnumSubset( &m_enum2Field, { MyEnumType::T2, MyEnumType::T6 } );

    // Enum field displaying a subset of the defined enums using calculateValueOptions()
    CAF_PDM_InitFieldNoDefault( &m_enum3Field, "Enum3Field", "Subset using calculateValueOptions()" );
    m_enum3Field = MyEnumType::T2;

    // Radio button enum field example
    CAF_PDM_InitField( &m_radioButtonEnumField, "RadioButtonEnumField", MyEnumType::T1, "Radio Button Example" );
    m_radioButtonEnumField.uiCapability()->setUiEditorTypeName( caf::PdmUiRadioButtonEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> ApplicationEnum::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_enum3Field )
    {
        QList<caf::PdmOptionItemInfo> options;

        options.push_back(
            caf::PdmOptionItemInfo( caf::AppEnum<ApplicationEnum::MyEnumType>::uiText( ApplicationEnum::MyEnumType::T2 ),
                                    ApplicationEnum::MyEnumType::T2 ) );

        options.push_back(
            caf::PdmOptionItemInfo( caf::AppEnum<ApplicationEnum::MyEnumType>::uiText( ApplicationEnum::MyEnumType::T3 ),
                                    ApplicationEnum::MyEnumType::T3 ) );

        return options;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ApplicationEnum::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_enumField );
    uiOrdering.add( &m_enum2Field );
    uiOrdering.add( &m_radioButtonEnumField );
    uiOrdering.add( &m_enum3Field );
}
