

#include "cafPdmUiCheckBoxTristateEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafTristate.h"

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiCheckBoxTristateEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxTristateEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    CAF_ASSERT( !m_checkBox.isNull() );
    CAF_ASSERT( !m_label.isNull() );

    PdmUiFieldEditorHandle::updateLabelFromField( m_label, uiConfigName );

    m_checkBox->setEnabled( !uiField()->isUiReadOnly( uiConfigName ) );
    m_checkBox->setToolTip( uiField()->uiToolTip( uiConfigName ) );

    Tristate state = uiField()->uiValue().value<Tristate>();

    if ( state == Tristate::State::True )
    {
        m_checkBox->setCheckState( Qt::Checked );
    }
    else if ( state == Tristate::State::PartiallyTrue )
    {
        m_checkBox->setCheckState( Qt::PartiallyChecked );
    }
    else if ( state == Tristate::State::False )
    {
        m_checkBox->setCheckState( Qt::Unchecked );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxTristateEditor::createEditorWidget( QWidget* parent )
{
    m_checkBox = new QCheckBox( parent );
    m_checkBox->setTristate( true );

    connect( m_checkBox, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxTristateEditor::slotClicked( bool )
{
    Tristate state;

    if ( m_checkBox->checkState() == Qt::Checked )
    {
        state = Tristate::State::True;
    }
    else if ( m_checkBox->checkState() == Qt::PartiallyChecked )
    {
        state = Tristate::State::PartiallyTrue;
    }
    else if ( m_checkBox->checkState() == Qt::Unchecked )
    {
        state = Tristate::State::False;
    }

    QVariant v = QVariant::fromValue( state );

    this->setValueToField( v );
}

} // end namespace caf
