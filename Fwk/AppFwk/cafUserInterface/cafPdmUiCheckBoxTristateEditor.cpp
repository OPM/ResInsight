

#include "cafPdmUiCheckBoxTristateEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"
#include "cafTristate.h"



namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiCheckBoxTristateEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxTristateEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_checkBox.isNull());
    CAF_ASSERT(!m_label.isNull());

    {
        QIcon ic = field()->uiIcon(uiConfigName);
        if (!ic.isNull())
        {
            m_label->setPixmap(ic.pixmap(ic.actualSize(QSize(64, 64))));
        }
        else
        {
            m_label->setText(field()->uiName(uiConfigName));
        }
    }

    m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_label->setToolTip(field()->uiToolTip(uiConfigName));

    m_checkBox->setEnabled(!field()->isUiReadOnly(uiConfigName));
    m_checkBox->setToolTip(field()->uiToolTip(uiConfigName));

    Tristate state = field()->uiValue().value<Tristate>();
    
    if (state == Tristate::State::True)
    {
        m_checkBox->setCheckState(Qt::Checked);
    }
    else if (state == Tristate::State::PartiallyTrue)
    {
        m_checkBox->setCheckState(Qt::PartiallyChecked);
    }
    else if (state == Tristate::State::False)
    {
        m_checkBox->setCheckState(Qt::Unchecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxTristateEditor::createEditorWidget(QWidget * parent)
{
    m_checkBox = new QCheckBox(parent);
    m_checkBox->setTristate(true);

    connect(m_checkBox, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)));
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiCheckBoxTristateEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiCheckBoxTristateEditor::slotClicked(bool)
{
    Tristate state;

    if (m_checkBox->checkState() == Qt::Checked)
    {
        state = Tristate::State::True;
    }
    else if (m_checkBox->checkState() == Qt::PartiallyChecked)
    {
        state = Tristate::State::PartiallyTrue;
    }
    else if (m_checkBox->checkState() == Qt::Unchecked)
    {
        state = Tristate::State::False;
    }

    QVariant v = QVariant::fromValue(state);

    this->setValueToField(v);
}


} // end namespace caf
