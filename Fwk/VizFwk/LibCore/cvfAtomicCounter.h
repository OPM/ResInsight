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


#pragma once

#include "cvfBase.h"

#ifdef WIN32
  #define CVF_ATOMIC_COUNTER_CLASS_EXISTS
#elif defined(CVF_IOS) || defined(CVF_OSX)
  #include <libkern/OSAtomic.h>
  #define CVF_ATOMIC_COUNTER_CLASS_EXISTS
#elif defined __GNUC__
    #define CVF_GCC_DEFINED
    #define CVF_ATOMIC_COUNTER_CLASS_EXISTS
#endif

#if defined(CVF_ATOMIC_COUNTER_CLASS_EXISTS)

namespace cvf {

// Inspired by Poco


class AtomicCounter
{
public:
    explicit AtomicCounter(int initialValue);
    ~AtomicCounter();

    operator int () const;

    int operator ++ ();     // prefix
    int operator ++ (int);  // postfix

    int operator -- ();     // prefix
    int operator -- (int);  // postfix

private:
    
    CVF_DISABLE_COPY_AND_ASSIGN(AtomicCounter);

#ifdef WIN32
    typedef volatile long ImplType;
#elif defined(CVF_IOS) || defined(CVF_OSX)
    typedef int32_t ImplType;
#else
    typedef int ImplType;
#endif

    ImplType m_counter;
};


} // namespace cvf

#endif
