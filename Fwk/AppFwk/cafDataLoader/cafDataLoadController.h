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

#pragma once

#include "cafDataLoader.h"

#include <map>
#include <memory>

#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QWaitCondition>

namespace caf
{

class ProgressInfo;
class PdmObject;

//==================================================================================================
///
//==================================================================================================
class DataLoadController : public caf::SignalObserver
{
public:
    static DataLoadController* instance();

    DataLoadController( const DataLoadController& )            = delete;
    DataLoadController& operator=( const DataLoadController& ) = delete;

    void registerDataLoader( const QString& objectType, const QString& dataType, std::shared_ptr<DataLoader> dataLoader );

    void loadData( caf::PdmObject& object, const QString& dateType, ProgressInfo& progressInfo );

    void blockUntilDone( const QString& dataType );

    void onTaskFinished( const caf::SignalEmitter* emitter, QString dataType, int taskId );

private:
    DataLoadController();

    std::map<std::pair<QString, QString>, std::shared_ptr<caf::DataLoader>> m_dataLoaders;
    std::map<QString, int>                                                  m_pendingTasksByType;
    int                                                                     m_taskId;

    QMutex         m_mutex;
    QWaitCondition m_waitCondition;
};

} // end namespace caf
