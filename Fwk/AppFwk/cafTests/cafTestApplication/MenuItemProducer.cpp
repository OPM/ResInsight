//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "MenuItemProducer.h"

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QTextCursor>
#include <QTextEdit>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
MenuItemProducer::MenuItemProducer() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MenuItemProducer::attachTextEdit(QTextEdit* textEdit)
{
    if (m_textEdit != textEdit)
    {
        textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(textEdit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotMenuItems(QPoint)));
    }

    m_textEdit = textEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MenuItemProducer::slotMenuItems(QPoint point)
{
    QMenu   menu;
    QAction act("Testing", this);
    connect(&act, SIGNAL(triggered()), SLOT(slotShowText()));

    menu.addAction(&act);

    QPoint globalPoint = point;
    if (m_textEdit)
    {
        globalPoint = m_textEdit->mapToGlobal(point);

        m_textPosition = m_textEdit->textCursor().position();
    }

    menu.exec(globalPoint);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void MenuItemProducer::slotShowText()
{
    if (m_textEdit)
    {
        QAction* action = qobject_cast<QAction*>(sender());
        if (action)
        {
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.setPosition(m_textPosition);

            m_textEdit->setTextCursor(cursor);
            m_textEdit->insertPlainText(action->text());
        }
    }
}
