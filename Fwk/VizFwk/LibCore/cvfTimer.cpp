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


#include "cvfBase.h"
#include "cvfTimer.h"

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)
#endif

#if defined(CVF_LINUX) || defined(CVF_ANDROID)
#include <time.h>
#endif

#if defined(CVF_IOS) || defined(CVF_OSX)
#include "mach/mach_time.h"
#endif


namespace cvf {



//=================================================================================================
// 
// PrivateTimerState
// 
//=================================================================================================
#ifdef WIN32
class PrivateTimerState
{
public:
	PrivateTimerState()
	{
		m_ticksPerSecond = 0;
		m_startTick = 0;	
		m_lastLapTick = 0;

        LARGE_INTEGER freq;
        if (QueryPerformanceFrequency(&freq))
        {
            m_ticksPerSecond = freq.QuadPart;
        }
    }

    __int64 m_ticksPerSecond;
	__int64 m_startTick;			
	__int64 m_lastLapTick;			
};
#endif //WIN32

#if defined(CVF_LINUX) || defined(CVF_ANDROID)
class PrivateTimerState
{
public:
    PrivateTimerState()
    {
        m_timeStart.tv_sec = 0;
        m_timeStart.tv_nsec = 0;
        m_timeMark.tv_sec = 0;
        m_timeMark.tv_nsec = 0;
    }

    timespec    m_timeStart;
    timespec    m_timeMark;
};
#endif // CVF_LINUX || CVF_ANDROID


#if defined(CVF_IOS) || defined(CVF_OSX)
class PrivateTimerState
{
public:
	PrivateTimerState()
	{
        mach_timebase_info(&m_info);
        m_startTime = 0;
        m_lastLapTime = 0;
	}

    mach_timebase_info_data_t   m_info;
    uint64_t                    m_startTime;
    uint64_t                    m_lastLapTime;
};
#endif // CVF_IOS || CVF_OSX


//--------------------------------------------------------------------------------------------------
/// Static helper on Linux to compute difference between two timespecs
//--------------------------------------------------------------------------------------------------
#if defined(CVF_LINUX) || defined(CVF_ANDROID)
static timespec ComputeTimespecDiff(const timespec start, const timespec end)
{
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) 
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } 
    else 
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}
#endif



//==================================================================================================
///
/// \class cvf::Timer
/// \ingroup Core
///
/// Timer class
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Start the timer.
//--------------------------------------------------------------------------------------------------
Timer::Timer()
{
	m_timerState = new PrivateTimerState;

    restart();
}


//--------------------------------------------------------------------------------------------------
/// Cleanup
//--------------------------------------------------------------------------------------------------
Timer::~Timer()
{
	delete m_timerState;
}


//--------------------------------------------------------------------------------------------------
/// Restart timer and laptime
//--------------------------------------------------------------------------------------------------
void Timer::restart()
{
    CVF_ASSERT(m_timerState);

#ifdef WIN32

    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);
    m_timerState->m_startTick = tick.QuadPart;
    m_timerState->m_lastLapTick = m_timerState->m_startTick;

#elif defined(CVF_LINUX) || defined(CVF_ANDROID)

    clock_gettime(CLOCK_MONOTONIC, &m_timerState->m_timeStart);
    m_timerState->m_timeMark = m_timerState->m_timeStart;

#elif defined(CVF_IOS) || defined(CVF_OSX)

    m_timerState->m_startTime = mach_absolute_time();
    m_timerState->m_lastLapTime   = m_timerState->m_startTime;

#endif
}


//--------------------------------------------------------------------------------------------------
/// Get the time elapsed since the start, in seconds
//--------------------------------------------------------------------------------------------------
double Timer::time() const
{
#ifdef WIN32

    if (m_timerState->m_ticksPerSecond == 0) return 0.0;
    LARGE_INTEGER nowTick;
    QueryPerformanceCounter(&nowTick);
    return static_cast<double>((nowTick.QuadPart - m_timerState->m_startTick))/static_cast<double>(m_timerState->m_ticksPerSecond);

#elif defined(CVF_LINUX) || defined(CVF_ANDROID)

    timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    timespec timeDiff = ComputeTimespecDiff(m_timerState->m_timeStart, timeNow);
    double timeUsedSec = static_cast<double>(timeDiff.tv_sec) + static_cast<double>(timeDiff.tv_nsec)/1000000000.0;
    return timeUsedSec;

#elif defined(CVF_IOS) || defined(CVF_OSX)

    uint64_t timeNow = mach_absolute_time();
    uint64_t timeUsed = timeNow - m_timerState->m_startTime;
    timeUsed *= m_timerState->m_info.numer;
    uint64_t timeUsedNanoSec = timeUsed/m_timerState->m_info.denom;
    double timeUsedSec = timeUsedNanoSec/1.0e9;
    return timeUsedSec;

#endif
}


//--------------------------------------------------------------------------------------------------
///  Get elapsed time since last lap, in seconds
//--------------------------------------------------------------------------------------------------
double Timer::lapTime()
{
#ifdef WIN32

    if (m_timerState->m_ticksPerSecond == 0) return 0.0;
    LARGE_INTEGER nowTick;
    QueryPerformanceCounter(&nowTick);
    double lapTime = static_cast<double>((nowTick.QuadPart - m_timerState->m_lastLapTick))/static_cast<double>(m_timerState->m_ticksPerSecond);
    m_timerState->m_lastLapTick = nowTick.QuadPart;

#elif defined(CVF_LINUX) || defined(CVF_ANDROID)

    timespec timeNow;
    clock_gettime(CLOCK_MONOTONIC, &timeNow);
    timespec timeDiff = ComputeTimespecDiff(m_timerState->m_timeMark, timeNow);
    m_timerState->m_timeMark = timeNow;
    double lapTime = static_cast<double>(timeDiff.tv_sec) + static_cast<double>(timeDiff.tv_nsec)/1000000000.0;

#elif defined(CVF_IOS) || defined(CVF_OSX)

    uint64_t timeNow = mach_absolute_time();
    uint64_t timeUsed = timeNow - m_timerState->m_lastLapTime;
    timeUsed *= m_timerState->m_info.numer;
    uint64_t timeUsedNanoSec = timeUsed/m_timerState->m_info.denom;
    double lapTime = timeUsedNanoSec/1.0e9;
    m_timerState->m_lastLapTime = timeNow;

#endif

    return lapTime;
}


} // namespace cvf

