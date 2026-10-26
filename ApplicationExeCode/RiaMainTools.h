/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include <QtGlobal>

class QString;

namespace RiaMainTools
{
void initializeSingletons();
void releaseSingletonAndFactoryObjects();
void deleteStaleSettingsLockFiles();

// True once crash logging has started. The crash handler pumps the Qt event loop while flushing
// telemetry, so timer-driven application logic must check this flag and stay idle to avoid
// re-entering or tearing down objects the crashed code still uses.
bool isCrashHandlingInProgress();

// Remember the latest Qt warning/critical/fatal messages. They are included in crash reports, as
// Qt prints the reason for an abort (e.g. lost GL context) to stderr right before aborting.
void recordQtMessage( QtMsgType type, const QString& msg );
}; // namespace RiaMainTools
