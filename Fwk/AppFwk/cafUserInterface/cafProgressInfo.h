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

#include <cstddef>

class QString;

namespace caf
{
class ProgressInfo;

class ProgressTask
{
public:
    ProgressTask( ProgressInfo& parentTask );
    ~ProgressTask();

private:
    ProgressInfo& m_parentTask;
};

class ProgressInfo
{
public:
    ProgressInfo( size_t maxProgressValue, const QString& title, bool delayShowingProgress = false );

    ~ProgressInfo();
    void setProgressDescription( const QString& description );
    void setProgress( size_t progressValue );
    void incrementProgress();
    void setNextProgressIncrement( size_t nextStepSize );

    ProgressTask task( const QString& description, int stepSize = 1 );
};

class ProgressInfoBlocker
{
public:
    ProgressInfoBlocker();
    ~ProgressInfoBlocker();
};

class ProgressInfoStatic
{
public:
    static void start( size_t maxProgressValue, const QString& title, bool delayShowingProgress );

    static void setProgressDescription( const QString& description );
    static void setProgress( size_t progressValue );
    static void incrementProgress();
    static void setNextProgressIncrement( size_t nextStepSize );
    static bool isRunning();
    static void finished();
    static void setEnabled( bool enable );

private:
    static bool isUpdatePossible();

private:
    friend class ProgressInfoBlocker;
    static bool s_running;
    static bool s_disabled;
};

} // namespace caf
