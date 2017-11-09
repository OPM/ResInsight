/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiuMessagePanel.h"

#include <QDockWidget>
#include <QMenu>
#include <QPlainTextEdit>
#include <QVBoxLayout>



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMessagePanel::RiuMessagePanel(QDockWidget* parent)
:   QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_textEdit = new QPlainTextEdit;
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_textEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_textEdit, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotShowContextMenu(const QPoint&)));

    layout->addWidget(m_textEdit);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::addMessage(RILogLevel messageLevel, const QString& msg)
{
    QColor clr(Qt::black);
    if      (messageLevel == RI_LL_ERROR)    clr = Qt::red;
    else if (messageLevel == RI_LL_WARNING)  clr = QColor(220,100,10);
    else if (messageLevel == RI_LL_DEBUG)    clr = QColor(100,100,200);

    QTextCharFormat form = m_textEdit->currentCharFormat();
    form.setForeground(clr);
    form.setFontWeight(messageLevel == RI_LL_ERROR ? QFont::DemiBold : QFont::Normal);
    form.setFontItalic(messageLevel == RI_LL_DEBUG ? true : false);
    m_textEdit->setCurrentCharFormat(form);
    m_textEdit->appendPlainText(msg);

    m_textEdit->moveCursor(QTextCursor::End);
    m_textEdit->ensureCursorVisible();

    if (messageLevel == RI_LL_ERROR || messageLevel == RI_LL_WARNING)
    {
        QDockWidget* parentDockWidget = dynamic_cast<QDockWidget*>(this->parent());
        if (parentDockWidget && !parentDockWidget->isVisible())
        {
            parentDockWidget->toggleViewAction()->trigger();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuMessagePanel::sizeHint() const
{
    return QSize(20, 20);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::slotShowContextMenu(const QPoint& pos)
{
    QMenu* menu = m_textEdit->createStandardContextMenu();

    menu->addSeparator();
    menu->addAction("Clear All &Messages", this, SLOT(slotClearMessages()));
    
    menu->exec(m_textEdit->mapToGlobal(pos));

    delete menu;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanel::slotClearMessages()
{
    m_textEdit->clear();

    RiaLogging::info("Message window cleared.");
}



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMessagePanelLogger::RiuMessagePanelLogger(RiuMessagePanel* messagePanel)
:   m_messagePanel(messagePanel),
    m_logLevel(RI_LL_WARNING)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RiuMessagePanelLogger::level() const
{
    return m_logLevel;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::setLevel(int logLevel)
{
    m_logLevel = logLevel;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::error(const char* message)
{
    writeToMessagePanel(RI_LL_ERROR, message);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::warning(const char* message)
{
    writeToMessagePanel(RI_LL_WARNING, message);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::info(const char* message)
{
    writeToMessagePanel(RI_LL_INFO, message);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::debug(const char* message)
{
    writeToMessagePanel(RI_LL_DEBUG, message);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMessagePanelLogger::writeToMessagePanel(RILogLevel messageLevel, const char* message)
{
    if (messageLevel > m_logLevel)
    {
        return;
    }

    if (m_messagePanel)
    {
        m_messagePanel->addMessage(messageLevel, message);
    }
}


