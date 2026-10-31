/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaHpcDefines.h"
#include "RiaWslTools.h"

#include <QMetaType>
#include <QString>
#include <QStringList>

#include <map>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace RiaHpcTools
{

struct BatchJobInformation
{
    BatchJobInformation( QStringList parts );
    BatchJobInformation() {};
    const QStringList asStringList() const;
    QString           id;
    QString           name;
    QString           runTime;
    QString           status;
    QString           queue;
    QString           where;
};

using BatchJobInformationMap = std::map<QString, BatchJobInformation>;

QStringList availableQueues( RiaDefines::BatchSchedulerType scheduler, const RiaWslTools::WslOptions& wslOptions );

QString findJobId( RiaDefines::BatchSchedulerType scheduler, QStringList stdOut );

void stopJob( RiaDefines::BatchSchedulerType scheduler, QString jobId, const RiaWslTools::WslOptions& wslOptions );
const RiaHpcTools::BatchJobInformationMap listJobs( RiaDefines::BatchSchedulerType scheduler, const RiaWslTools::WslOptions& wslOptions );

// helpers
QStringList decodeSlurmQueues( QStringList stdOut );
QStringList decodeLsfQueues( QStringList stdOut );

const RiaHpcTools::BatchJobInformationMap decodeSlurmJobs( QStringList stdOut );
const RiaHpcTools::BatchJobInformationMap decodeLsfJobs( QStringList stdOut );

} // namespace RiaHpcTools

Q_DECLARE_METATYPE( RiaHpcTools::BatchJobInformationMap )
