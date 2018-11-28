/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA 2016
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

#include <QObject>
#include <QString>

#include <vector>

class QAction;

//==================================================================================================
//
// 
//
//==================================================================================================
class RiuRecentFileActionProvider : public QObject
{
    Q_OBJECT

public:
    explicit RiuRecentFileActionProvider(int maxActionCount = 9);
    ~RiuRecentFileActionProvider() override;

    void addFileName(const QString& fileName);
    std::vector<QAction*> actions() const;

private slots:
    void slotOpenRecentFile();

private:
    void createActions();
    void updateActions();
    void removeFileName(const QString& fileName);

private:
    int m_maxActionCount;

    std::vector<QAction*> m_recentFileActions;
    QAction* m_separatorAction;
};


