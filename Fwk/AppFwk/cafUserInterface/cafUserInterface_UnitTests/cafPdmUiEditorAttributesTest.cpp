#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmLogging.h"
#include "cafPdmObject.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLabelEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiSliderEditor.h"

#include <QCheckBox>
#include <QComboBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include <functional>
#include <memory>
#include <vector>

using namespace caf;

class EditorAttributesTestObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    EditorAttributesTestObj()
    {
        CAF_PDM_InitObject( "EditorAttributesTestObj" );
        CAF_PDM_InitField( &m_int, "Int", 5, "Int Field" );
        CAF_PDM_InitField( &m_bool, "Bool", true, "Bool Field" );
        CAF_PDM_InitField( &m_text, "Text", QString( "text" ), "Text Field" );
        CAF_PDM_InitField( &m_double, "Double", 2.5, "Double Field" );
        CAF_PDM_InitField( &m_option, "Option", 0, "Option Field" );
    }

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override
    {
        QList<caf::PdmOptionItemInfo> options;
        if ( fieldNeedingOptions == &m_option )
        {
            options.push_back( caf::PdmOptionItemInfo( "A", 0 ) );
            options.push_back( caf::PdmOptionItemInfo( "B", 1 ) );
            options.push_back( caf::PdmOptionItemInfo( "C", 2 ) );
        }
        return options;
    }

    caf::PdmField<int>     m_int;
    caf::PdmField<bool>    m_bool;
    caf::PdmField<QString> m_text;
    caf::PdmField<double>  m_double;
    caf::PdmField<int>     m_option;
};
CAF_PDM_SOURCE_INIT( EditorAttributesTestObj, "EditorAttributesTestObj" );

namespace
{
class WarningCollector : public caf::PdmLogger
{
public:
    int  level() const override { return static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ); }
    void setLevel( int ) override {}
    void error( const QString& ) override {}
    void warning( const QString& message ) override { warnings.push_back( message ); }
    void info( const QString& ) override {}
    void debug( const QString& ) override {}

    std::vector<QString> warnings;
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// Collects PDM warnings so that tests can verify that supported attributes never trigger the
/// "Unsupported attribute" validation warning
//--------------------------------------------------------------------------------------------------
class PdmUiEditorAttributesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_collector = std::make_shared<WarningCollector>();
        caf::PdmLogging::registerLogger( m_collector );
    }

    void TearDown() override { caf::PdmLogging::unregisterLogger( m_collector ); }

    std::vector<QString>& warnings() { return m_collector->warnings; }

    std::shared_ptr<WarningCollector> m_collector;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, SliderRangeAndSpinBoxVisibility )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_int.uiCapability();
    ui->setAttribute( PdmUiSliderEditor::Keys::MINIMUM, 2 );
    ui->setAttribute( PdmUiSliderEditor::Keys::MAXIMUM, 40 );
    ui->setAttribute( PdmUiSliderEditor::Keys::STEP, 3 );
    ui->setAttribute( PdmUiSliderEditor::Keys::SHOW_SPIN_BOX, false );

    QWidget           parentWidget;
    PdmUiSliderEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* slider  = editor.editorWidget()->findChild<QSlider*>();
    auto* spinBox = editor.editorWidget()->findChild<QSpinBox*>();
    ASSERT_NE( slider, nullptr );
    ASSERT_NE( spinBox, nullptr );

    EXPECT_EQ( 2, slider->minimum() );
    EXPECT_EQ( 40, slider->maximum() );
    EXPECT_EQ( 3, slider->singleStep() );
    EXPECT_EQ( 2, spinBox->minimum() );
    EXPECT_EQ( 40, spinBox->maximum() );
    EXPECT_FALSE( spinBox->isVisibleTo( editor.editorWidget() ) );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, SliderDefaultsWithoutAttributes )
{
    EditorAttributesTestObj obj;

    QWidget           parentWidget;
    PdmUiSliderEditor editor;
    editor.setUiField( obj.m_int.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* slider = editor.editorWidget()->findChild<QSlider*>();
    ASSERT_NE( slider, nullptr );
    EXPECT_EQ( 0, slider->minimum() );
    EXPECT_EQ( 10, slider->maximum() );
    EXPECT_EQ( 5, slider->value() );
}

//--------------------------------------------------------------------------------------------------
/// Attributes set for a specific ui config must only affect that config
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, SliderUsesConfigSpecificAttributes )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_int.uiCapability();
    ui->setAttribute( PdmUiSliderEditor::Keys::MAXIMUM, 20 );
    ui->setAttribute( PdmUiSliderEditor::Keys::MAXIMUM, 99, "Special" );

    QWidget           parentWidget;
    PdmUiSliderEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );

    editor.updateUi( "" );
    auto* slider = editor.editorWidget()->findChild<QSlider*>();
    ASSERT_NE( slider, nullptr );
    EXPECT_EQ( 20, slider->maximum() );

    editor.updateUi( "Special" );
    EXPECT_EQ( 99, slider->maximum() );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, LineEditAttributes )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_text.uiCapability();
    ui->setAttribute( PdmUiLineEditor::Keys::MAXIMUM_WIDTH, 123 );
    ui->setAttribute( PdmUiLineEditor::Keys::PLACEHOLDER_TEXT, "type here" );

    QIntValidator validator( 0, 100 );
    ui->setAttribute<QValidator*>( PdmUiLineEditor::Keys::VALIDATOR, &validator );

    obj.m_text = "";

    QWidget         parentWidget;
    PdmUiLineEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* lineEdit = editor.editorWidget()->findChild<QLineEdit*>();
    if ( !lineEdit ) lineEdit = qobject_cast<QLineEdit*>( editor.editorWidget() );
    ASSERT_NE( lineEdit, nullptr );

    EXPECT_EQ( 123, lineEdit->maximumWidth() );
    EXPECT_EQ( "type here", lineEdit->placeholderText().toStdString() );
    EXPECT_EQ( &validator, lineEdit->validator() );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, CheckBoxNativeAttributeUsesFieldNameAsText )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_bool.uiCapability();
    ui->setAttribute( PdmUiCheckBoxEditor::Keys::USE_NATIVE_CHECKBOX, true );

    QWidget             parentWidget;
    PdmUiCheckBoxEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* checkBox = qobject_cast<QCheckBox*>( editor.editorWidget() );
    ASSERT_NE( checkBox, nullptr );
    EXPECT_TRUE( checkBox->isChecked() );
    EXPECT_EQ( "Bool Field", checkBox->text().toStdString() );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
/// Editors must report attributes that they do not support, and only those
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, UnsupportedAttributeIsReportedByEditor )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_bool.uiCapability();
    ui->setAttribute( PdmUiSliderEditor::Keys::MINIMUM, 1 ); // Slider key, not supported by a check box

    QWidget             parentWidget;
    PdmUiCheckBoxEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    ASSERT_EQ( 1u, warnings().size() );
    EXPECT_TRUE( warnings().front().contains( PdmUiSliderEditor::Keys::MINIMUM ) );
}

//--------------------------------------------------------------------------------------------------
/// Editors write user input back to the field
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, CheckBoxClickWritesToField )
{
    EditorAttributesTestObj obj;

    QWidget             parentWidget;
    PdmUiCheckBoxEditor editor;
    editor.setUiField( obj.m_bool.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* checkBox = qobject_cast<QCheckBox*>( editor.editorWidget() );
    ASSERT_NE( checkBox, nullptr );

    checkBox->click();
    EXPECT_FALSE( obj.m_bool() );
    checkBox->click();
    EXPECT_TRUE( obj.m_bool() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, SliderMoveWritesToField )
{
    EditorAttributesTestObj obj;

    QWidget           parentWidget;
    PdmUiSliderEditor editor;
    editor.setUiField( obj.m_int.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* slider  = editor.editorWidget()->findChild<QSlider*>();
    auto* spinBox = editor.editorWidget()->findChild<QSpinBox*>();
    ASSERT_NE( slider, nullptr );
    ASSERT_NE( spinBox, nullptr );

    slider->setValue( 8 );
    EXPECT_EQ( 8, obj.m_int() );
    EXPECT_EQ( 8, spinBox->value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, LineEditTextWritesToField )
{
    EditorAttributesTestObj obj;

    QWidget         parentWidget;
    PdmUiLineEditor editor;
    editor.setUiField( obj.m_text.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* lineEdit = editor.editorWidget()->findChild<QLineEdit*>();
    if ( !lineEdit ) lineEdit = qobject_cast<QLineEdit*>( editor.editorWidget() );
    ASSERT_NE( lineEdit, nullptr );
    EXPECT_EQ( "text", lineEdit->text().toStdString() );

    lineEdit->setText( "changed" );
    emit lineEdit->editingFinished();
    EXPECT_EQ( "changed", obj.m_text().toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, DoubleSliderAttributes )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_double.uiCapability();
    ui->setAttribute( PdmUiDoubleSliderEditor::Keys::MINIMUM, 1.0 );
    ui->setAttribute( PdmUiDoubleSliderEditor::Keys::MAXIMUM, 5.0 );
    ui->setAttribute( PdmUiDoubleSliderEditor::Keys::DECIMALS, 2 );
    ui->setAttribute( PdmUiDoubleSliderEditor::Keys::SLIDER_TICK_COUNT, 400 );

    QWidget                 parentWidget;
    PdmUiDoubleSliderEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* slider   = editor.editorWidget()->findChild<QSlider*>();
    auto* lineEdit = editor.editorWidget()->findChild<QLineEdit*>();
    ASSERT_NE( slider, nullptr );
    ASSERT_NE( lineEdit, nullptr );

    EXPECT_EQ( 400, slider->maximum() );
    EXPECT_EQ( "2.5", lineEdit->text().toStdString() );

    // Halfway between min and max of the double range corresponds to 3.0 (1.0 + 0.5 * (5.0 - 1.0))
    slider->setValue( 200 );
    EXPECT_NEAR( 3.0, obj.m_double(), 0.05 );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, DoubleSliderDelayedUpdateDoesNotWriteWhileDragging )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_double.uiCapability();
    ui->setAttribute( PdmUiDoubleSliderEditor::Keys::DELAY_SLIDER_UPDATE_UNTIL_RELEASE, true );

    QWidget                 parentWidget;
    PdmUiDoubleSliderEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* slider = editor.editorWidget()->findChild<QSlider*>();
    ASSERT_NE( slider, nullptr );

    slider->setValue( slider->maximum() );
    EXPECT_DOUBLE_EQ( 2.5, obj.m_double() );

    emit slider->sliderReleased();
    EXPECT_NEAR( 10.0, obj.m_double(), 0.01 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, PushButtonText )
{
    EditorAttributesTestObj obj;

    QWidget               parentWidget;
    PdmUiPushButtonEditor editor;
    editor.setUiField( obj.m_bool.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* button = editor.editorWidget()->findChild<QPushButton*>();
    if ( !button ) button = qobject_cast<QPushButton*>( editor.editorWidget() );
    ASSERT_NE( button, nullptr );

    // Without a text attribute a bool field shows its state
    EXPECT_EQ( "On", button->text().toStdString() );

    obj.m_bool.uiCapability()->setAttribute( PdmUiPushButtonEditor::Keys::BUTTON_TEXT, "Run" );
    editor.updateUi();
    EXPECT_EQ( "Run", button->text().toStdString() );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, LabelLinkTextAndCallback )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_text.uiCapability();
    ui->setAttribute( PdmUiLabelEditor::Keys::LINK_TEXT, "<a href=\"target\">click</a>" );

    QString received;
    ui->setAttribute<std::function<void( const QString& )>>( PdmUiLabelEditor::Keys::LINK_ACTIVATED_CALLBACK,
                                                             [&received]( const QString& link ) { received = link; } );

    QWidget          parentWidget;
    PdmUiLabelEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* label = editor.editorWidget()->findChild<QLabel*>();
    if ( !label ) label = qobject_cast<QLabel*>( editor.editorWidget() );
    ASSERT_NE( label, nullptr );

    EXPECT_EQ( Qt::RichText, label->textFormat() );

    // Updating the editor again must not duplicate the callback connection
    editor.updateUi();
    int callCount = 0;
    ui->setAttribute<std::function<void( const QString& )>>( PdmUiLabelEditor::Keys::LINK_ACTIVATED_CALLBACK,
                                                             [&callCount]( const QString& ) { callCount++; } );
    editor.updateUi();
    emit label->linkActivated( "target" );
    EXPECT_EQ( 1, callCount );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, LabelWordWrapAttribute )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_text.uiCapability();
    ui->setAttribute( PdmUiLabelEditor::Keys::USE_WORD_WRAP, true );

    QWidget          parentWidget;
    PdmUiLabelEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* label = editor.editorWidget()->findChild<QLabel*>();
    if ( !label ) label = qobject_cast<QLabel*>( editor.editorWidget() );
    ASSERT_NE( label, nullptr );
    EXPECT_TRUE( label->wordWrap() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, ComboBoxOptionsAndAttributes )
{
    EditorAttributesTestObj obj;
    auto*                   ui = obj.m_option.uiCapability();
    ui->setAttribute( PdmUiComboBoxEditor::Keys::MINIMUM_WIDTH, 222 );
    ui->setAttribute( PdmUiComboBoxEditor::Keys::ENABLE_EDITABLE_CONTENT, true );
    ui->setAttribute( PdmUiComboBoxEditor::Keys::PLACEHOLDER_TEXT, "pick one" );

    QWidget             parentWidget;
    PdmUiComboBoxEditor editor;
    editor.setUiField( ui );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* comboBox = editor.editorWidget()->findChild<QComboBox*>();
    if ( !comboBox ) comboBox = qobject_cast<QComboBox*>( editor.editorWidget() );
    ASSERT_NE( comboBox, nullptr );

    EXPECT_EQ( 3, comboBox->count() );
    EXPECT_EQ( "B", comboBox->itemText( 1 ).toStdString() );
    EXPECT_EQ( 0, comboBox->currentIndex() );
    EXPECT_EQ( 222, comboBox->minimumWidth() );
    EXPECT_TRUE( comboBox->isEditable() );
    EXPECT_EQ( "pick one", comboBox->lineEdit()->placeholderText().toStdString() );
    EXPECT_TRUE( warnings().empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST_F( PdmUiEditorAttributesTest, ComboBoxSelectionWritesToField )
{
    EditorAttributesTestObj obj;

    QWidget             parentWidget;
    PdmUiComboBoxEditor editor;
    editor.setUiField( obj.m_option.uiCapability() );
    editor.createWidgets( &parentWidget );
    editor.updateUi();

    auto* comboBox = editor.editorWidget()->findChild<QComboBox*>();
    if ( !comboBox ) comboBox = qobject_cast<QComboBox*>( editor.editorWidget() );
    ASSERT_NE( comboBox, nullptr );

    comboBox->setCurrentIndex( 2 );
    emit comboBox->activated( 2 );
    EXPECT_EQ( 2, obj.m_option() );
}

//--------------------------------------------------------------------------------------------------
/// Every key an editor consumes must be part of the editor's supported set, otherwise valid use
/// triggers a runtime warning
//--------------------------------------------------------------------------------------------------
TEST( PdmUiEditorSupportedAttributesTest, KeysAreRegisteredAsSupported )
{
    EXPECT_TRUE( PdmUiDoubleSliderEditor::SUPPORTED_ATTRIBUTES.count(
        PdmUiDoubleSliderEditor::Keys::DELAY_SLIDER_UPDATE_UNTIL_RELEASE ) );
    EXPECT_TRUE( PdmUiLabelEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLabelEditor::Keys::LINK_TEXT ) );
    EXPECT_TRUE( PdmUiLabelEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLabelEditor::Keys::LINK_ACTIVATED_CALLBACK ) );
    EXPECT_TRUE( PdmUiLabelEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLabelEditor::Keys::USE_WORD_WRAP ) );
    EXPECT_TRUE( PdmUiPushButtonEditor::SUPPORTED_ATTRIBUTES.count( PdmUiPushButtonEditor::Keys::BUTTON_TEXT ) );
    EXPECT_TRUE( PdmUiPushButtonEditor::SUPPORTED_ATTRIBUTES.count( PdmUiPushButtonEditor::Keys::BUTTON_ICON ) );
    EXPECT_TRUE( PdmUiComboBoxEditor::SUPPORTED_ATTRIBUTES.count( PdmUiComboBoxEditor::Keys::MINIMUM_WIDTH ) );
    EXPECT_TRUE( PdmUiComboBoxEditor::SUPPORTED_ATTRIBUTES.count( PdmUiComboBoxEditor::Keys::ENABLE_EDITABLE_CONTENT ) );
    EXPECT_TRUE( PdmUiComboBoxEditor::SUPPORTED_ATTRIBUTES.count( PdmUiComboBoxEditor::Keys::PLACEHOLDER_TEXT ) );

    EXPECT_TRUE( PdmUiSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiSliderEditor::Keys::MINIMUM ) );
    EXPECT_TRUE( PdmUiSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiSliderEditor::Keys::MAXIMUM ) );
    EXPECT_TRUE( PdmUiSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiSliderEditor::Keys::SHOW_SPIN_BOX ) );
    EXPECT_TRUE( PdmUiSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiSliderEditor::Keys::STEP ) );

    EXPECT_TRUE( PdmUiDoubleSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiDoubleSliderEditor::Keys::MINIMUM ) );
    EXPECT_TRUE( PdmUiDoubleSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiDoubleSliderEditor::Keys::MAXIMUM ) );
    EXPECT_TRUE( PdmUiDoubleSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiDoubleSliderEditor::Keys::DECIMALS ) );
    EXPECT_TRUE( PdmUiDoubleSliderEditor::SUPPORTED_ATTRIBUTES.count( PdmUiDoubleSliderEditor::Keys::SLIDER_TICK_COUNT ) );

    EXPECT_TRUE( PdmUiLineEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLineEditor::Keys::VALIDATOR ) );
    EXPECT_TRUE( PdmUiLineEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLineEditor::Keys::PLACEHOLDER_TEXT ) );
    EXPECT_TRUE( PdmUiLineEditor::SUPPORTED_ATTRIBUTES.count( PdmUiLineEditor::Keys::MAXIMUM_WIDTH ) );

    EXPECT_TRUE( PdmUiCheckBoxEditor::SUPPORTED_ATTRIBUTES.count( PdmUiCheckBoxEditor::Keys::USE_NATIVE_CHECKBOX ) );
    EXPECT_TRUE( PdmUiCheckBoxEditor::SUPPORTED_ATTRIBUTES.count( PdmUiCheckBoxEditor::Keys::WORD_WRAP ) );
}
