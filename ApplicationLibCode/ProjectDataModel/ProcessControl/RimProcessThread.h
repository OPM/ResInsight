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

#include <QString>
#include <QThread>

class RimProcess;

class RimProcessThread : public QThread
{
    Q_OBJECT

public:
    RimProcessThread( RimProcess* process )
        : m_process( process )
    {
    }

protected:
    void run() override
    {
        QString result = m_process->execute();

        emit processDone( result );
    }

signals:
    void processDone( const QString resultText );

private:
    RimProcess* m_process;
};
