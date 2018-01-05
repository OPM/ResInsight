/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuExpressionContextMenuManager.h"
#include <QTextEdit>
#include <QMenu>

const std::map<QString, std::set<QString>> RiuExpressionContextMenuManager::MENU_MAP =
{
    {
        "Basic Operators", 
        {
            "+", "-", "*", "/", "x^n"
        }
    },
    {
        "Assignment Operators",
        {
            ":="
        }
    },
    {
        "Scalar Functions",
        {
            "avg(x)", "max(x)", "min(x)", "sum(x)"
        }
    },
    {
        "Vector Functions",
        {
            "abs(x)", "ceil(x)", "floor(x)", "frac(x)", 
            "log(x)", "log10(x)", "pow(x, n)", "round(x)",
            "sgn(x)", "sqrt(x)", "trunc(x)"
        }
    },
    {
        "Trigonometry Functions",
        {
            "acos(x)", "acosh(x)", "asin(x)", "asinh(x)", "atan(x)", "atanh(x)", 
            "cos(x)", "cosh(x)", "cot(x)", "csc(x)", "sec(x)", "sin(x)", "sinc(x)", "sinh(x)",
            "tan(x)", "tanh(x)", "rad2deg(x)", "deg2grad(x)", "deg2rad(x)", "grad2deg(x)"
        }
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuExpressionContextMenuManager::attachTextEdit(QTextEdit* textEdit)
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
void RiuExpressionContextMenuManager::slotMenuItems(QPoint point)
{
    QMenu menu;

    for (const std::pair<QString, std::set<QString>>& subMenuPair : MENU_MAP)
    {
        QMenu* subMenu = menu.addMenu(subMenuPair.first);

        for (const QString& menuItemText : subMenuPair.second)
        {
            if (m_actionCache.count(menuItemText) == 0)
            {
                QAction* action = new QAction(menuItemText, this);
                connect(action, SIGNAL(triggered()), SLOT(slotShowText()));
                m_actionCache.insert(std::make_pair(menuItemText, std::unique_ptr<QAction>(action)));
            }

            subMenu->addAction(m_actionCache[menuItemText].get());
        }
    }

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
void RiuExpressionContextMenuManager::slotShowText()
{
    if (m_textEdit)
    {
        QAction* action = qobject_cast<QAction *>(sender());
        if (action)
        {
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.setPosition(m_textPosition);

            m_textEdit->setTextCursor(cursor);
            m_textEdit->insertPlainText(action->text());
        }
    }
}
