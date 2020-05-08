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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"

#include "cafFactory.h"
#include "cafQShortenedLabel.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QWheelEvent>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiComboBoxEditor);


/* GIMP RGBA C-Source image dump (StepDown.c) */

static const struct {
    unsigned int 	 width;
    unsigned int 	 height;
    unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
    unsigned char	 pixel_data[16 * 16 * 4 + 1];
} stepDownImageData = {
    16, 16, 4,
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000AAA\001\030\030\030\001"
    "\037\037\037\001\020\020\020\001\004\004\004\001\016\016\016\001!!!\001\"\"\"\001(((\001\060\060\060\001$$"
    "$\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000UUU\014FFF\242\030\030\030\256\037\037\037\256"
    "\022\022\022\256\005\005\005\256\021\021\021\256'''\256...\256\061\061\061\256\067\067\067"
    "\256&&&\256AAAzTTT\010\000\000\000\000\000\000\000\000xxx\014```\273\033\033\033\377&&&\377\""
    "\"\"\377\017\017\017\377\"\"\"\377LLL\377___\377^^^\377^^^\377AAA\376OOOXTT"
    "T\001\000\000\000\000\000\000\000\000\000\000\000\000JJJ\071+++\343&&&\377%%%\377\017\017\017\377'''\377"
    "WWW\377]]]\377hhh\377WWW\376NNN\300\177\177\177\032\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000KKK\004\066\066\066z\040\040\040\370\"\"\"\377\014\014\014\377$$$\377SSS\377"
    "ccc\377bbb\377NNN\362\202\202\202=\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\064\064\064\040===\312\032\032\032\375\017\017\017\377$$$\377WWW\377bbb"
    "\377MMM\374LLL\200iii\006\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000W"
    "WW\001AAA\063###\330\007\007\007\377(((\377VVV\377UUU\377WWW\314\217\217\217\040\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000;;;\001\066\066"
    "\066}\027\027\027\371(((\377TTT\377FFF\360\\\\\\C\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000TTT\015\025\025\025\036\040\040\040!<<<<???\360\"\"\""
    "\377===\377ddd\266GGG\062\026\026\026\061\040\040\040\066\"\"\"\022\000\000\000\000\000\000\000\000"
    "\000\000\000\000HHH\015\071\071\071\256\007\007\007\314\015\015\015\316\024\024\024\326\034\034\034"
    "\374\022\022\022\377!!!\377###\335###\326\035\035\035\336\032\032\032\343///\220A"
    "AA\010\000\000\000\000\000\000\000\000bbb\014QQQ\264%%%\355$$$\363\035\035\035\352\034\034\034\351"
    "&&&\353$$$\344)))\346\061\061\061\345\066\066\066\350\062\062\062\335\064\064\064\201"
    "???\007\000\000\000\000\000\000\000\000\000\000\000\000SSS\023@@@?\070\070\070E---=,,,<///>\"\"\"\067&&"
    "&\070$$$\070---:CCC\060;;;\015\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000",
};

QIcon createStepDownIcon()
{
    QImage img(stepDownImageData.pixel_data,stepDownImageData.width, stepDownImageData.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage(img);

    return QIcon(pxMap);
}

static const QIcon& stepDownIcon()
{
    static QIcon expandDownIcon(createStepDownIcon());
    return expandDownIcon;
}

/* GIMP RGBA C-Source image dump (StepUp.c) */

static const struct {
    unsigned int 	 width;
    unsigned int 	 height;
    unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
    unsigned char	 pixel_data[16 * 16 * 4 + 1];
} stepUpImageData = {
    16, 16, 4,
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000;;;\015CCC\060---:"
    "$$$\070&&&\070\"\"\"\067///>,,,<---=\070\070\070E@@@?SSS\023\000\000\000\000\000\000\000\000\000\000"
    "\000\000???\007\064\064\064\201\062\062\062\335\066\066\066\350\061\061\061\345)))\346$$$\344"
    "&&&\353\034\034\034\351\035\035\035\352$$$\363%%%\355QQQ\264bbb\014\000\000\000\000\000\000"
    "\000\000AAA\010///\220\032\032\032\343\035\035\035\336###\326###\335!!!\377\022\022\022"
    "\377\034\034\034\374\024\024\024\326\015\015\015\316\007\007\007\314\071\071\071\256HHH\015"
    "\000\000\000\000\000\000\000\000\000\000\000\000\"\"\"\022\040\040\040\066\026\026\026\061GGG\062ddd\266=="
    "=\377\"\"\"\377???\360<<<<\040\040\040!\025\025\025\036TTT\015\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\\\\\\CFFF\360TTT\377(((\377\027\027"
    "\027\371\066\066\066};;;\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\217\217\217\040WWW\314UUU\377VVV\377(((\377\007\007\007\377###\330"
    "AAA\063WWW\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000iii\006LLL\200M"
    "MM\374bbb\377WWW\377$$$\377\017\017\017\377\032\032\032\375===\312\064\064\064\040"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\202\202\202=NNN\362bbb\377"
    "ccc\377SSS\377$$$\377\014\014\014\377\"\"\"\377\040\040\040\370\066\066\066zKKK\004"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\177\177\177\032NNN\300WWW\376hhh\377]]]\377"
    "WWW\377'''\377\017\017\017\377%%%\377&&&\377+++\343JJJ\071\000\000\000\000\000\000\000\000\000"
    "\000\000\000TTT\001OOOXAAA\376^^^\377^^^\377___\377LLL\377\"\"\"\377\017\017\017\377"
    "\"\"\"\377&&&\377\033\033\033\377```\273xxx\014\000\000\000\000\000\000\000\000TTT\010AAAz&&&"
    "\256\067\067\067\256\061\061\061\256...\256'''\256\021\021\021\256\005\005\005\256\022\022"
    "\022\256\037\037\037\256\030\030\030\256FFF\242UUU\014\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000$$$\001\060\060\060\001(((\001\"\"\"\001!!!\001\016\016\016\001\004\004\004\001\020\020\020\001\037"
    "\037\037\001\030\030\030\001AAA\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000",
};

QIcon createStepUpIcon()
{
    QImage img(stepUpImageData.pixel_data,stepUpImageData.width, stepUpImageData.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage(img);

    return QIcon(pxMap);
}

static const QIcon& stepUpIcon()
{
    static QIcon stepUpIcon(createStepUpIcon());
    return stepUpIcon;
}


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
    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }

    if (!m_comboBox.isNull())
    {
        m_comboBox->setEnabled(!uiField()->isUiReadOnly(uiConfigName));
        m_comboBox->setToolTip(uiField()->uiToolTip(uiConfigName));


        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> options = uiField()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        m_comboBox->blockSignals(true);
        m_comboBox->clear();
        QListView* listView = dynamic_cast<QListView*>(m_comboBox->view());
        if (listView)
        {
            listView->setSpacing(2);
        }

        if (!options.isEmpty())
        {
            for (const auto& option : options)
            {
                auto icon = option.icon();
                if (icon)
                    m_comboBox->addItem(*icon, option.optionUiText());
                else
                    m_comboBox->addItem(option.optionUiText());
                m_comboBox->setIconSize(m_attributes.iconSize);
            }
            m_comboBox->setCurrentIndex(uiField()->uiValue().toInt());
        }
        else
        {
            m_comboBox->addItem(uiField()->uiValue().toString());
            m_comboBox->setCurrentIndex(0);
        }

        if (m_attributes.adjustWidthToContents)
        {
            m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        }
        else if (m_attributes.minimumContentsLength > 0)
        {
            m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
            m_comboBox->setMinimumContentsLength(m_attributes.minimumContentsLength);
            // Make sure the popup adjusts to the content even if the widget itself doesn't
            QFont font = m_comboBox->view()->font();
            
            int maxTextWidth = 0;
            bool labelsElided = false;
            for (const PdmOptionItemInfo& option : options)
            {
                QString label = option.optionUiText();
                if (label.size() > m_attributes.maximumMenuContentsLength)
                {
                    label.resize(m_attributes.maximumMenuContentsLength);
                    labelsElided = true;
                }
                maxTextWidth  = std::max(maxTextWidth, QFontMetrics(font).boundingRect(label).width());
            }

            int marginWidth = m_comboBox->view()->contentsMargins().left() + m_comboBox->view()->contentsMargins().right();
            m_comboBox->view()->setMinimumWidth(maxTextWidth + marginWidth);
            m_comboBox->view()->setTextElideMode(labelsElided ? Qt::ElideMiddle : Qt::ElideNone);
        }

        if (m_attributes.enableEditableContent)
        {
            m_comboBox->setEditable(true);

            m_comboBox->lineEdit()->setPlaceholderText(m_attributes.placeholderText);
        }

        if (m_attributes.minimumWidth != -1)
        {
            m_comboBox->setMinimumWidth(m_attributes.minimumWidth);
        }

        m_comboBox->blockSignals(false);
    }

    if (!m_layout.isNull())
    {
        if (m_attributes.showPreviousAndNextButtons)
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
                QIcon disabledIcon(stepUpIcon().pixmap(16, 16, QIcon::Disabled));
                m_previousItemButton->setIcon(disabledIcon);
            }
            else
            {
                m_previousItemButton->setIcon(stepUpIcon());
            }

            if (m_comboBox->count() == 0 || m_comboBox->currentIndex() >= m_comboBox->count() - 1)
            {
                QIcon disabledIcon(stepDownIcon().pixmap(16, 16, QIcon::Disabled));
                m_nextItemButton->setIcon(disabledIcon);
            }
            else
            {
                m_nextItemButton->setIcon(stepDownIcon());
            }

            // Update button texts
            if (!m_attributes.nextButtonText.isEmpty())
            {
                m_nextItemButton->setToolTip(m_attributes.nextButtonText);
            }

            if (!m_attributes.prevButtonText.isEmpty())
            {
                m_previousItemButton->setToolTip(m_attributes.prevButtonText);
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
    m_label = new QShortenedLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiComboBoxEditor::slotIndexActivated(int index)
{
    if (m_attributes.enableEditableContent)
    {
        // Use the text directly, as the item text could be entered directly by the user

        auto text = m_comboBox->itemText(index);
        this->setValueToField(text);
    }
    else
    {
        // Use index as data carrier to PDM field
        // The index will be used as a lookup in a list of option items

        QVariant v;
        v = index;

        QVariant uintValue(v.toUInt());
        this->setValueToField(uintValue);
    }
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
