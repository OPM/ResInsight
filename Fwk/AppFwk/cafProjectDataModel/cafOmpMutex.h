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

#include <omp.h>

namespace caf
{
//==================================================================================================
/// OpenMP mutex definition
/// Taken from http://bisqwit.iki.fi/story/howto/openmp/
//==================================================================================================
class MutexType
{
public:
    MutexType() { omp_init_lock( &m_lock ); }
    ~MutexType() { omp_destroy_lock( &m_lock ); }
    void lock() { omp_set_lock( &m_lock ); }
    void unlock() { omp_unset_lock( &m_lock ); }

    MutexType( const MutexType& ) { omp_init_lock( &m_lock ); }
    MutexType& operator=( const MutexType& ) { return *this; }

private:
    omp_lock_t m_lock;
};

//==================================================================================================
/// OpenMP scoped lock on a mutex
/// Taken from http://bisqwit.iki.fi/story/howto/openmp/
//==================================================================================================
class ScopedLock
{
public:
    explicit ScopedLock( MutexType& m )
        : m_mutex( m )
        , m_locked( true )
    {
        m_mutex.lock();
    }

    ~ScopedLock() { unlock(); }

    void unlock()
    {
        if ( !m_locked ) return;
        m_locked = false;
        m_mutex.unlock();
    }

    void lockAgain()
    {
        if ( m_locked ) return;
        m_mutex.lock();
        m_locked = true;
    }

private:
    MutexType& m_mutex;
    bool       m_locked;

private: // prevent copying the scoped lock.
    void operator=( const ScopedLock& );
    ScopedLock( const ScopedLock& );
};

} // namespace caf
