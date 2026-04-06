
#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmFieldTraits.h"
#include "cafPdmObject.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiNumberFormat.h"

#include <QCheckBox>
#include <QLineEdit>

#include <utility>

using namespace caf;

class CheckBoxAndTextTestObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    CheckBoxAndTextTestObj()
    {
        CAF_PDM_InitObject( "CheckBoxAndTextTestObj" );

        auto defaultPair = std::make_pair( false, QString( "" ) );
        CAF_PDM_InitField( &m_checkableText, "CheckableText", defaultPair, "Checkable Text" );

        auto doublePair = std::make_pair( true, 3.14 );
        CAF_PDM_InitField( &m_checkableDouble, "CheckableDouble", doublePair, "Checkable Double" );
    }

    caf::PdmField<std::pair<bool, QString>> m_checkableText;
    caf::PdmField<std::pair<bool, double>>  m_checkableDouble;
};
CAF_PDM_SOURCE_INIT( CheckBoxAndTextTestObj, "CheckBoxAndTextTestObj" );

//--------------------------------------------------------------------------------------------------
/// Test default field values for pair<bool, QString>
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, DefaultValues )
{
    CheckBoxAndTextTestObj obj;

    EXPECT_EQ( obj.m_checkableText().first, false );
    EXPECT_EQ( obj.m_checkableText().second.toStdString(), "" );

    EXPECT_EQ( obj.m_checkableDouble().first, true );
    EXPECT_DOUBLE_EQ( obj.m_checkableDouble().second, 3.14 );
}

//--------------------------------------------------------------------------------------------------
/// Test setting and getting pair<bool, QString> field values
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, SetAndGetPairValues )
{
    CheckBoxAndTextTestObj obj;

    obj.m_checkableText = std::make_pair( true, QString( "hello" ) );
    EXPECT_EQ( obj.m_checkableText().first, true );
    EXPECT_EQ( obj.m_checkableText().second.toStdString(), "hello" );

    obj.m_checkableText = std::make_pair( false, QString( "world" ) );
    EXPECT_EQ( obj.m_checkableText().first, false );
    EXPECT_EQ( obj.m_checkableText().second.toStdString(), "world" );
}

//--------------------------------------------------------------------------------------------------
/// Test QVariant conversion round-trip for pair<bool, QString>
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, QVariantRoundTrip )
{
    auto original = std::make_pair( true, QString( "test value" ) );

    QVariant variant = caf::pdmToVariant( original );

    EXPECT_TRUE( variant.canConvert<QList<QVariant>>() );
    QList<QVariant> lst = variant.toList();
    EXPECT_EQ( lst.size(), 2 );
    EXPECT_EQ( lst[0].toBool(), true );
    EXPECT_EQ( lst[1].toString().toStdString(), "test value" );

    std::pair<bool, QString> restored;
    caf::pdmFromVariant( variant, restored );

    EXPECT_EQ( restored.first, original.first );
    EXPECT_EQ( restored.second, original.second );
}

//--------------------------------------------------------------------------------------------------
/// Test QVariant conversion for unchecked state
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, QVariantUncheckedState )
{
    auto original = std::make_pair( false, QString( "disabled text" ) );

    QVariant variant = caf::pdmToVariant( original );

    QList<QVariant> lst = variant.toList();
    EXPECT_EQ( lst[0].toBool(), false );
    EXPECT_EQ( lst[1].toString().toStdString(), "disabled text" );
}

//--------------------------------------------------------------------------------------------------
/// Test number formatting used by the editor for double values
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, NumberFormattingForDoubleValues )
{
    auto pair = std::make_pair( true, 1234.5678 );

    QVariant        variant = caf::pdmToVariant( pair );
    QList<QVariant> lst     = variant.toList();
    EXPECT_EQ( lst.size(), 2 );

    bool   valueOk = false;
    double value   = lst[1].toDouble( &valueOk );
    EXPECT_TRUE( valueOk );

    // Test the formatting path used in configureAndUpdateUi
    QString fixedText = PdmUiNumberFormat::valueToText( value, NumberFormatType::FIXED, 2 );
    EXPECT_EQ( fixedText.toStdString(), "1234.57" );

    QString scientificText = PdmUiNumberFormat::valueToText( value, NumberFormatType::SCIENTIFIC, 3 );
    EXPECT_EQ( scientificText.toStdString(), "1.235e+03" );
}

//--------------------------------------------------------------------------------------------------
/// Test QVariant round-trip for pair<bool, double>
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, QVariantRoundTripDouble )
{
    auto original = std::make_pair( true, 42.5 );

    QVariant variant = caf::pdmToVariant( original );

    std::pair<bool, double> restored;
    caf::pdmFromVariant( variant, restored );

    EXPECT_EQ( restored.first, original.first );
    EXPECT_DOUBLE_EQ( restored.second, original.second );
}

//--------------------------------------------------------------------------------------------------
/// Test field uiValue returns correct QVariant list
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, FieldUiValue )
{
    CheckBoxAndTextTestObj obj;

    obj.m_checkableText = std::make_pair( true, QString( "ui test" ) );

    QVariant uiValue = obj.m_checkableText.uiCapability()->uiValue();
    EXPECT_TRUE( uiValue.canConvert<QList<QVariant>>() );

    QList<QVariant> lst = uiValue.toList();
    EXPECT_EQ( lst.size(), 2 );
    EXPECT_EQ( lst[0].toBool(), true );
    EXPECT_EQ( lst[1].toString().toStdString(), "ui test" );
}

//--------------------------------------------------------------------------------------------------
/// Test configureAndUpdateUi with checked string field
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, ConfigureAndUpdateUi_CheckedStringField )
{
    CheckBoxAndTextTestObj obj;
    obj.m_checkableText = std::make_pair( true, QString( "my text" ) );

    QWidget parentWidget;

    PdmUiCheckBoxAndTextEditor editor;
    editor.setUiField( obj.m_checkableText.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* editorWidget = editor.editorWidget();
    auto* checkBox     = editorWidget->findChild<QCheckBox*>();
    auto* lineEdit     = editorWidget->findChild<QLineEdit*>();

    ASSERT_NE( checkBox, nullptr );
    ASSERT_NE( lineEdit, nullptr );

    EXPECT_TRUE( checkBox->isChecked() );
    EXPECT_EQ( lineEdit->text().toStdString(), "my text" );
    EXPECT_TRUE( lineEdit->isEnabled() );
}

//--------------------------------------------------------------------------------------------------
/// Test configureAndUpdateUi with unchecked field disables line edit
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, ConfigureAndUpdateUi_UncheckedDisablesLineEdit )
{
    CheckBoxAndTextTestObj obj;
    obj.m_checkableText = std::make_pair( false, QString( "disabled text" ) );

    QWidget parentWidget;

    PdmUiCheckBoxAndTextEditor editor;
    editor.setUiField( obj.m_checkableText.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* editorWidget = editor.editorWidget();
    auto* checkBox     = editorWidget->findChild<QCheckBox*>();
    auto* lineEdit     = editorWidget->findChild<QLineEdit*>();

    ASSERT_NE( checkBox, nullptr );
    ASSERT_NE( lineEdit, nullptr );

    EXPECT_FALSE( checkBox->isChecked() );
    EXPECT_EQ( lineEdit->text().toStdString(), "disabled text" );
    EXPECT_FALSE( lineEdit->isEnabled() );
}

//--------------------------------------------------------------------------------------------------
/// Test configureAndUpdateUi with double field and number formatting attributes
//--------------------------------------------------------------------------------------------------
TEST( PdmUiCheckBoxAndTextEditorTest, ConfigureAndUpdateUi_DoubleWithNumberFormat )
{
    CheckBoxAndTextTestObj obj;
    obj.m_checkableDouble = std::make_pair( true, 1234.5678 );
    obj.m_checkableDouble.uiCapability()->setAttribute( PdmUiCheckBoxAndTextEditor::Keys::DECIMALS, 2 );
    obj.m_checkableDouble.uiCapability()->setAttribute( PdmUiCheckBoxAndTextEditor::Keys::NUMBER_FORMAT,
                                                        static_cast<int>( NumberFormatType::FIXED ) );

    QWidget parentWidget;

    PdmUiCheckBoxAndTextEditor editor;
    editor.setUiField( obj.m_checkableDouble.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* editorWidget = editor.editorWidget();
    auto* checkBox     = editorWidget->findChild<QCheckBox*>();
    auto* lineEdit     = editorWidget->findChild<QLineEdit*>();

    ASSERT_NE( checkBox, nullptr );
    ASSERT_NE( lineEdit, nullptr );

    EXPECT_TRUE( checkBox->isChecked() );
    EXPECT_EQ( lineEdit->text().toStdString(), "1234.57" );
    EXPECT_TRUE( lineEdit->isEnabled() );
}
