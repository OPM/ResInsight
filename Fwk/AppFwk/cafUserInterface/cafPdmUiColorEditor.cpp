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


#include "cafPdmUiColorEditor.h"

#include "cafPdmUiDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"

#include <QColor>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>


namespace caf
{

CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiColorEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiColorEditor::PdmUiColorEditor()
{
    m_color = QColor::Invalid;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiColorEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(!m_label.isNull());

    PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);

    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }

    QColor col = uiField()->uiValue().value<QColor>();
    setColor(col);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiColorEditor::createEditorWidget(QWidget * parent)
{
    QWidget* placeholder = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout(placeholder);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    m_colorPixmapLabel = new QLabel(parent);
    m_colorTextLabel = new QLabel(parent);

    QToolButton* button = new QToolButton(parent);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    button->setText(QLatin1String("..."));

    layout->addWidget(m_colorPixmapLabel);
    layout->addWidget(m_colorTextLabel);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));
    layout->addWidget(button);

    connect(button, SIGNAL(clicked()), this, SLOT(colorSelectionClicked()));

    return placeholder;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiColorEditor::createLabelWidget(QWidget * parent)
{
    m_label = new QLabel(parent);
    return m_label;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiColorEditor::colorSelectionClicked()
{
    QColorDialog::ColorDialogOptions flags;
    if (m_attributes.showAlpha)
    {
        flags |= QColorDialog::ShowAlphaChannel;
    }

    QColor newColor = QColorDialog::getColor(m_color, m_colorPixmapLabel, "Select color", flags);
    if (newColor.isValid() && newColor != m_color)
    {
        setColor(newColor);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiColorEditor::setColor(const QColor& color)
{
    if (m_color != color)
    {
        m_color = color;

        QPixmap tmp(16, 16);
        tmp.fill(m_color);
        m_colorPixmapLabel->setPixmap(tmp);

        QString colorString;
        if (!color.isValid())
        {
            colorString = "Undefined";
        }
        else
        {
            colorString = QString("[%1, %2, %3]").arg(QString::number(color.red())).arg(QString::number(color.green())).arg(QString::number(color.blue()));
            
            if (m_attributes.showAlpha)
            {
                colorString += QString(" (%4)").arg(QString::number(color.alpha()));
            }
        }

        m_colorTextLabel->setText(colorString);
    }
    
    QVariant v;
    v = m_color;
    this->setValueToField(v);
}


} // end namespace caf
