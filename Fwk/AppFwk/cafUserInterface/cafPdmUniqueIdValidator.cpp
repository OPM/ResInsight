//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUniqueIdValidator.h"

#include <QApplication>
#include <QMainWindow>
#include <QStatusBar>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUniqueIdValidator::PdmUniqueIdValidator(const std::set<int>& usedIds,
                                           bool                 multipleSelectionOfSameFieldsSelected,
                                           const QString&       errorMessage,
                                           QObject*             parent)
    : QValidator(parent)
    , m_usedIds(usedIds)
    , m_nextValidValue(0)
    , m_multipleSelectionOfSameFieldsSelected(multipleSelectionOfSameFieldsSelected)
    , m_errorMessage(errorMessage)
{
    computeNextValidId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QValidator::State PdmUniqueIdValidator::validate(QString& currentString, int&) const
{
    if (m_multipleSelectionOfSameFieldsSelected)
    {
        return QValidator::Invalid;
    }

    if (currentString.isEmpty())
    {
        return QValidator::Intermediate;
    }

    bool isValidInteger = false;
    int  currentValue   = currentString.toInt(&isValidInteger);

    if (!isValidInteger)
    {
        return QValidator::Invalid;
    }

    if (currentValue < 0)
    {
        return QValidator::Invalid;
    }

    if (m_usedIds.find(currentValue) != m_usedIds.end())
    {
        foreach (QWidget* widget, QApplication::topLevelWidgets())
        {
            if (widget->inherits("QMainWindow"))
            {
                QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget);
                if (mainWindow && mainWindow->statusBar())
                {
                    mainWindow->statusBar()->showMessage(m_errorMessage, 3000);
                }
            }
        }

        return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUniqueIdValidator::fixup(QString& editorText) const
{
    editorText = QString::number(m_nextValidValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmUniqueIdValidator::computeNextValidId()
{
    if (!m_usedIds.empty())
    {
        m_nextValidValue = *m_usedIds.rbegin();
    }
    else
    {
        m_nextValidValue = 1;
    }

    return m_nextValidValue;
}
