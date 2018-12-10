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

#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmField.h"
#include <QPointer>
#include <QAction>
#include <memory>

class RimSummaryCalculationVariable;
class QTextEdit;

//==================================================================================================
///  
///  
//==================================================================================================
class RiuExpressionContextMenuManager : public QObject
{
    Q_OBJECT

     static const std::map<QString, std::set<QString>> MENU_MAP;

public:
    RiuExpressionContextMenuManager() { }

    void attachTextEdit(QTextEdit* textEdit);

public slots:
    void slotMenuItems(QPoint point);

private slots:
    void slotShowText();

private:
    QPointer<QTextEdit>                         m_textEdit;
    int                                         m_textPosition;
    std::map<QString, std::unique_ptr<QAction>> m_actionCache;
};
