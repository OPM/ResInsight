/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuTextDialog.h"
#include "RiuTools.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QMenu>



//--------------------------------------------------------------------------------------------------
/// 
/// 
///  RiuQPlainTextEdit
/// 
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier )
    {
        slotCopyContentToClipboard();
        e->setAccepted(true);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(e);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotCopyContentToClipboard()
{
    QTextCursor cursor(this->textCursor());

    QString textForClipboard;

    QString selText = cursor.selectedText();
    if (!selText.isEmpty())
    {
        QTextDocument doc;
        doc.setPlainText(selText);

        textForClipboard = doc.toPlainText();
    }

    if (textForClipboard.isEmpty())
    {
        textForClipboard = this->toPlainText();
    }

    if (!textForClipboard.isEmpty())
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            clipboard->setText(textForClipboard);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuQPlainTextEdit::slotSelectAll()
{
    this->selectAll();
}

//--------------------------------------------------------------------------------------------------
/// 
/// 
/// RiuTextDialog
/// 
/// 
//--------------------------------------------------------------------------------------------------
RiuTextDialog::RiuTextDialog(QWidget* parent)
    : QDialog(parent, RiuTools::defaultDialogFlags())
{
    m_textEdit = new RiuQPlainTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    QFont font("Courier", 8);
    m_textEdit->setFont(font);

    m_textEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_textEdit);
    setLayout(layout);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTextDialog::setText(const QString& text)
{
    m_textEdit->setPlainText(text);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuTextDialog::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Copy");
        actionToSetup->setIcon(QIcon(":/Copy.png"));
        actionToSetup->setShortcuts(QKeySequence::Copy);

        connect(actionToSetup, SIGNAL(triggered()), m_textEdit, SLOT(slotCopyContentToClipboard()));

        menu.addAction(actionToSetup);
    }

    {
        QAction* actionToSetup = new QAction(this);

        actionToSetup->setText("Select All");
        actionToSetup->setShortcuts(QKeySequence::SelectAll);

        connect(actionToSetup, SIGNAL(triggered()), m_textEdit, SLOT(slotSelectAll()));

        menu.addAction(actionToSetup);
    }

    menu.exec(event->globalPos());
}



