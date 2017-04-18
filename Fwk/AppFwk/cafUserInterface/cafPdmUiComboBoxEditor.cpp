//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cafPdmUiComboBoxEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QComboBox>
#include <QLabel>
#include <QWheelEvent>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiComboBoxEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (!m_label.isNull())
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
        m_label->setEnabled(!field()->isUiReadOnly(uiConfigName));
    }

    if (!m_comboBox.isNull())
    {
        m_comboBox->setEnabled(!field()->isUiReadOnly(uiConfigName));

        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> options = field()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        m_comboBox->blockSignals(true);
        m_comboBox->clear();
        if (!options.isEmpty())
        {
            for (int i = 0; i < options.size(); i++)
            {
                m_comboBox->addItem(options[i].icon, options[i].optionUiText);
            }
            m_comboBox->setCurrentIndex(field()->uiValue().toInt());
        }
        else
        {
            m_comboBox->addItem(field()->uiValue().toString());
            m_comboBox->setCurrentIndex(0);
        }
        m_comboBox->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
// Special class used to prevent a combo box to steal focus when scrolling
// the QScrollArea using the mouse wheel
//
// Based on
// http://stackoverflow.com/questions/5821802/qspinbox-inside-a-qscrollarea-how-to-prevent-spin-box-from-stealing-focus-when
//--------------------------------------------------------------------------------------------------
class CustomQComboBox : public QComboBox
{
public:
    explicit CustomQComboBox(QWidget* parent = 0)
        : QComboBox(parent)
    {}

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void wheelEvent(QWheelEvent *e)
    {
        if (hasFocus())
        {
            QComboBox::wheelEvent(e);
        }
        else
        {
            // Ignore the event to make sure event is handled by another widget
            e->ignore();
        }
    }

protected:
    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void focusInEvent(QFocusEvent* e) override
    {
        setFocusPolicy(Qt::WheelFocus);
        QComboBox::focusInEvent(e);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    virtual void focusOutEvent(QFocusEvent* e) override
    {
        setFocusPolicy(Qt::StrongFocus);
        QComboBox::focusOutEvent(e);
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiComboBoxEditor::createEditorWidget(QWidget * parent)
{
    m_comboBox = new CustomQComboBox(parent);
    connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(slotIndexActivated(int)));

    return m_comboBox;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiComboBoxEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotIndexActivated(int index)
{
    QVariant v;
    v = index;

    QVariant uintValue(v.toUInt());
    this->setValueToField(uintValue);
}



} // end namespace caf
