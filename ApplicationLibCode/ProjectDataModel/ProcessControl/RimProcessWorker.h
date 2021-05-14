/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021    Equinor ASA
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

#include <QMutex>
#include <QObject>
#include <QString>

class RimProcess;

class RimProcessWorker : public QObject
{
public:
    RimProcessWorker( std::list<RimProcess*>* procList, QMutex* listLock, bool* exitFlag );
    ~RimProcessWorker() override;

public slots:
    void runWorker();

signals:
    void processDone( RimProcess* process );

private:
    std::list<RimProcess*>* m_procList;
    QMutex*                 m_listLock;
    bool*                   m_exitFlag;
};
