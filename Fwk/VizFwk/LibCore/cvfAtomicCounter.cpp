//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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


#include "cvfAtomicCounter.h"

// Some older GCC version do not support atomics, we have seen this for RHEL5
#if defined(CVF_ATOMIC_COUNTER_CLASS_EXISTS)

namespace cvf {

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)


AtomicCounter::AtomicCounter(int initialValue)
    : m_counter(initialValue)
{
}


AtomicCounter::~AtomicCounter()
{
}


AtomicCounter::operator int () const
{
    return m_counter;
}

int AtomicCounter::operator ++ () // prefix
{
    return InterlockedIncrement(&m_counter);
}


int AtomicCounter::operator ++ (int) // postfix
{
    int result = InterlockedIncrement(&m_counter);
    return --result;
}


int AtomicCounter::operator -- () // prefix
{
    return InterlockedDecrement(&m_counter);
}


int AtomicCounter::operator -- (int) // postfix
{
    int result = InterlockedDecrement(&m_counter);
    return ++result;
}


#elif defined(CVF_IOS) || defined(CVF_OSX)

AtomicCounter::AtomicCounter(int initialValue)
    : m_counter(initialValue)
{
}


AtomicCounter::~AtomicCounter()
{
}

AtomicCounter::operator int () const
{
    return m_counter;
}


int AtomicCounter::operator ++ () // prefix
{
    return OSAtomicIncrement32(&m_counter);
}


int AtomicCounter::operator ++ (int) // postfix
{
    int result = OSAtomicIncrement32(&m_counter);
    return --result;
}


int AtomicCounter::operator -- () // prefix
{
    return OSAtomicDecrement32(&m_counter);
}


int AtomicCounter::operator -- (int) // postfix
{
    int result = OSAtomicDecrement32(&m_counter);
    return ++result;
}


#elif defined(CVF_GCC_DEFINED)


AtomicCounter::AtomicCounter(int initialValue)
    : m_counter(initialValue)
{
}

AtomicCounter::~AtomicCounter()
{
}

AtomicCounter::operator int () const
{
    return m_counter;
}


int AtomicCounter::operator ++ () // prefix
{
    return __sync_add_and_fetch(&m_counter, 1);
}


int AtomicCounter::operator ++ (int) // postfix
{
    return __sync_fetch_and_add(&m_counter, 1);
}


int AtomicCounter::operator -- () // prefix
{
    return __sync_sub_and_fetch(&m_counter, 1);
}


int AtomicCounter::operator -- (int) // postfix
{
    return __sync_fetch_and_sub(&m_counter, 1);
}


#endif


} // namespace cvf



#endif // CVF_ATOMICS_COMPILED
