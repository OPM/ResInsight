//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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
#include "cafQShortenedLabel.h"

#include <QApplication>
#include <QFontMetrics>
#include <QResizeEvent>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QShortenedLabel::QShortenedLabel(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : QLabel(parent, f)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize QShortenedLabel::minimumSizeHint() const
{
    int minimumWidth = 0;

    QFontMetrics fontMetrics = QApplication::fontMetrics();
    QString fullLabelText = fullText();
    if (!fullLabelText.isEmpty())
    {
        minimumWidth = 10;

        QStringList words = fullLabelText.split(" ");
        if (!words.empty())
        {
            int textMinimumWidth = std::min(fontMetrics.width(fullLabelText), fontMetrics.width(words.front() + "..."));
            minimumWidth = std::max(minimumWidth, textMinimumWidth);
        }
    }
    QSize minimumSize = QLabel::minimumSizeHint();
    minimumSize.setWidth(minimumWidth);
    return minimumSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize QShortenedLabel::sizeHint() const
{
    QFontMetrics fontMetrics = QApplication::fontMetrics();
    QString      labelText   = fullText();
    QSize        size        = QLabel::sizeHint();
    size.setWidth(fontMetrics.width(labelText));
    return size;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::resizeEvent(QResizeEvent* event)
{
    QString labelText = fullText();
    QFontMetrics fontMetrics = QApplication::fontMetrics();

    if (fontMetrics.width(labelText) < event->size().width())
    {
        setDisplayText(labelText);
    }
    else
    {
        int width = std::max(minimumSizeHint().width(), event->size().width());
        QString elidedText = fontMetrics.elidedText(labelText, Qt::ElideRight, width);
        setDisplayText(elidedText);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QShortenedLabel::setDisplayText(const QString& shortText)
{
    // Store original text if we haven't already done so.
    if (m_fullLengthText.isEmpty())
    {
        m_fullLengthText = text();
    }
    setText(shortText);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QShortenedLabel::fullText() const
{
    if (!m_fullLengthText.isEmpty())
    {
        return m_fullLengthText;
    }
    return text();
}
