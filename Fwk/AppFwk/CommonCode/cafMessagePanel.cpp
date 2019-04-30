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


#include "cafMessagePanel.h"

#include <QWidget>
#include <QDockWidget>
#include <QTextEdit>
#include <QBoxLayout>

namespace caf {


MessagePanel* MessagePanel::sm_messagePanelInstance = nullptr;

//==================================================================================================
///
/// \class MessagePanel
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MessagePanel::MessagePanel(QDockWidget* parent)
:   QWidget(parent)
{
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_textEdit);
    setLayout(layout);

    sm_messagePanelInstance = this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MessagePanel::showInfo(QString info)
{
    convertStringToHTML(&info);

    QString str = "<font color='green'>";
    str += info;
    str += "</font>";

    m_textEdit->append(str);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MessagePanel::showWarning(QString warn)
{
    convertStringToHTML(&warn);

    QString str = "<font color='maroon'>";
    str += warn;
    str += "</font>";
    
    m_textEdit->append(str);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MessagePanel::showError(QString error)
{
    convertStringToHTML(&error);

    QString str = "<b><font color='red'>";
    str += error;
    str += "</font></b>";

    m_textEdit->append(str);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MessagePanel::convertStringToHTML(QString* str)
{
    str->replace("\n", "<br>");
    str->replace(" ", "&nbsp;");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize MessagePanel::sizeHint () const
{
    // As small as possible fow now
    return QSize(20, 20);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool MessagePanel::isVisibleToUser()
{
    if (!isVisible()) return false;

    if (!m_textEdit) return false;
    if (!m_textEdit->isVisible()) return false;

    QRegion rgn = m_textEdit->visibleRegion();
    if (rgn.isEmpty()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MessagePanel* MessagePanel::instance()
{
    return sm_messagePanelInstance;
}

}
