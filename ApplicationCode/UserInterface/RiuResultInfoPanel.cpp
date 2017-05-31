/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiuResultInfoPanel.h"

#include <QDockWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

//==================================================================================================
///
/// \class RiuResultInfoPanel
/// \ingroup ResInsight
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuResultInfoPanel::RiuResultInfoPanel(QDockWidget* parent)
:   QWidget(parent)
{
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_textEdit);

    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultInfoPanel::setInfo(const QString& info)
{
    QString tmp(info);

    convertStringToHTML(&tmp);

    m_textEdit->setText(info);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuResultInfoPanel::convertStringToHTML(QString* str)
{
    str->replace("\n", "<br>");
    str->replace(" ", "&nbsp;");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize RiuResultInfoPanel::sizeHint () const
{
    // As small as possible for now
    return QSize(20, 20);
}
