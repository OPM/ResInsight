//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfBase.h"
#include "cvfMutex.h"

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)
#endif

#ifdef CVF_LINUX
#include <pthread.h>
#endif

namespace cvf {


//==================================================================================================
//
// Win32 implementation using critical section
//
//==================================================================================================
#ifdef WIN32
class MutexImpl
{
public:
    MutexImpl()
    {
        ::InitializeCriticalSection(&m_critSection);
    }

    ~MutexImpl()
    {
        ::DeleteCriticalSection(&m_critSection);
    }

    void lock()
    {
        ::EnterCriticalSection(&m_critSection);
    }

    void unlock()
    {
        ::LeaveCriticalSection(&m_critSection);
    }

private:
    CRITICAL_SECTION    m_critSection;
};
#endif


//==================================================================================================
//
// Linux implementation using POSIX/Pthreads
//
//==================================================================================================
#ifdef CVF_LINUX
class MutexImpl
{
public:
    MutexImpl()
    {
        pthread_mutexattr_t mutexAttribs;
        
        int errCode = 0;
        CVF_UNUSED(errCode);
        errCode = pthread_mutexattr_init(&mutexAttribs);
        CVF_ASSERT(errCode == 0);

        // Use a recursive mutex to be aligned with Win32 implementation
        errCode = pthread_mutexattr_settype(&mutexAttribs, PTHREAD_MUTEX_RECURSIVE);
        CVF_ASSERT(errCode == 0);

        errCode = pthread_mutex_init(&m_mutex, &mutexAttribs);
        CVF_ASSERT(errCode == 0);

        // We're done with the attribs object
        errCode = pthread_mutexattr_destroy(&mutexAttribs);
        CVF_ASSERT(errCode == 0);
    }

    ~MutexImpl()
    {
        int errCode = pthread_mutex_destroy(&m_mutex);
        CVF_UNUSED(errCode);
        CVF_TIGHT_ASSERT(errCode == 0);
    }

    void lock()
    {
        int errCode = pthread_mutex_lock(&m_mutex);
        CVF_UNUSED(errCode);
        CVF_TIGHT_ASSERT(errCode == 0);
    }

    void unlock()
    {
        int errCode = pthread_mutex_unlock(&m_mutex);
        CVF_UNUSED(errCode);
        CVF_TIGHT_ASSERT(errCode == 0);
    }

private:
    pthread_mutex_t     m_mutex;
};
#endif



//==================================================================================================
///
/// \class cvf::Mutex
/// \ingroup Core
///
/// Implements a recursive mutex where the same thread can acquire the lock multiple times. 
///
/// The mutex is implemented as an recursive mutex since on Windows platforms its implementation
/// is based critical sections. Win32 critical sections are always recursive, and therefore we also
/// make the other platform implementations recursive for consistency.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mutex::Mutex()
:   m_pimpl(new MutexImpl)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mutex::~Mutex()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Mutex::lock()
{
    m_pimpl->lock();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Mutex::unlock()
{
    m_pimpl->unlock();
}


//==================================================================================================
///
/// \class cvf::Mutex::ScopedLock
/// \ingroup Core
///
/// 
///
//==================================================================================================
Mutex::ScopedLock::ScopedLock(Mutex& mutex)
:   m_theMutex(mutex)
{
    m_theMutex.lock();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Mutex::ScopedLock::~ScopedLock()
{
    m_theMutex.unlock();
}


} // namespace cvf

