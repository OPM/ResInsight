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

#include <QApplication>
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
        PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);
    }

    // Handle attributes
    PdmUiComboBoxEditorAttribute attributes;
    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &attributes);
    }

    if (!m_comboBox.isNull())
    {
        m_comboBox->setEnabled(!uiField()->isUiReadOnly(uiConfigName));

        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> options = uiField()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        m_comboBox->blockSignals(true);
        m_comboBox->clear();
        if (!options.isEmpty())
        {
            for (int i = 0; i < options.size(); i++)
            {
                m_comboBox->addItem(options[i].icon(), options[i].optionUiText());
            }
            m_comboBox->setCurrentIndex(uiField()->uiValue().toInt());
        }
        else
        {
            m_comboBox->addItem(uiField()->uiValue().toString());
            m_comboBox->setCurrentIndex(0);
        }

        if (attributes.adjustWidthToContents)
        {
            m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        }

        m_comboBox->blockSignals(false);
    }

    if (!m_layout.isNull())
    {
        if (attributes.showPreviousAndNextButtons)
        {
            if (m_previousItemButton.isNull())
            {
                m_previousItemButton = new QToolButton(m_placeholder);
                connect(m_previousItemButton, SIGNAL(clicked()), this, SLOT(slotPreviousButtonPressed()));

                m_previousItemButton->setToolTip("Previous");
            }

            if (m_nextItemButton.isNull())
            {
                m_nextItemButton = new QToolButton(m_placeholder);
                connect(m_nextItemButton, SIGNAL(clicked()), this, SLOT(slotNextButtonPressed()));

                m_nextItemButton->setToolTip("Next");
            }

            m_layout->insertWidget(1, m_previousItemButton);
            m_layout->insertWidget(2, m_nextItemButton);

            if (m_comboBox->count() == 0 || m_comboBox->currentIndex() <= 0)
            {
                QIcon disabledIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp).pixmap(16, 16, QIcon::Disabled));
                m_previousItemButton->setIcon(disabledIcon);
            }
            else
            {
                m_previousItemButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
            }

            if (m_comboBox->count() == 0 || m_comboBox->currentIndex() >= m_comboBox->count() - 1)
            {
                QIcon disabledIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown).pixmap(16, 16, QIcon::Disabled));
                m_nextItemButton->setIcon(disabledIcon);
            }
            else
            {
                m_nextItemButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
            }

            // Update button texts
            if (!attributes.nextButtonText.isEmpty())
            {
                m_nextItemButton->setToolTip(attributes.nextButtonText);
            }

            if (!attributes.prevButtonText.isEmpty())
            {
                m_previousItemButton->setToolTip(attributes.prevButtonText);
            }
        }
        else
        {
            if (m_previousItemButton)
            {
                m_layout->removeWidget(m_previousItemButton);
                m_previousItemButton->deleteLater();
            }

            if (m_nextItemButton)
            {
                m_layout->removeWidget(m_nextItemButton);
                m_nextItemButton->deleteLater();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins PdmUiComboBoxEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_comboBox->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int heightDiff   = editorSize.height() - labelSize.height();

    QMargins contentMargins = m_label->contentsMargins();
    if (heightDiff > 0)
    {
        contentMargins.setTop(contentMargins.top() + heightDiff / 2);
        contentMargins.setBottom(contentMargins.bottom() + heightDiff / 2);
    }
    return contentMargins;
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
    explicit CustomQComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
    {}

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void wheelEvent(QWheelEvent *e) override
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
    void focusInEvent(QFocusEvent* e) override
    {
        setFocusPolicy(Qt::WheelFocus);
        QComboBox::focusInEvent(e);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void focusOutEvent(QFocusEvent* e) override
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
    m_comboBox->setFocusPolicy(Qt::StrongFocus);

    m_placeholder = new QWidget(parent);

    m_layout = new QHBoxLayout(m_placeholder);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_comboBox);

    connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(slotIndexActivated(int)));

    return m_placeholder;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotNextButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() + 1;

    if (indexCandidate < m_comboBox->count())
    {
        slotIndexActivated(indexCandidate);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotPreviousButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() - 1;

    if (indexCandidate > -1 && indexCandidate < m_comboBox->count())
    {
        slotIndexActivated(indexCandidate);
    }
}

} // end namespace caf
