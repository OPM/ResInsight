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

#include <QString>
#include <QStringList>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace RiaHpcTools
{
QStringList availableQueues( RiaDefines::BatchSchedulerType scheduler );

void                              stopJob( RiaDefines::BatchSchedulerType scheduler, QString jobId );
QString                           findJobId( RiaDefines::BatchSchedulerType scheduler, QStringList stdOut );
std::vector<std::vector<QString>> listJobs( RiaDefines::BatchSchedulerType scheduler );

// helpers
QStringList decodeSlurmQueues( QStringList stdOut );
QStringList decodeLsfQueues( QStringList stdOut );

const std::vector<std::vector<QString>> decodeSlurmJobs( QStringList stdOut );
const std::vector<std::vector<QString>> decodeLsfJobs( QStringList stdOut );

} // namespace RiaHpcTools
