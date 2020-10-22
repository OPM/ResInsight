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

#include "cvfBase.h"
#include "cvfSystem.h"

#include <set>

#include "cvfAtomicCounter.h"

#if !defined(CVF_ATOMIC_COUNTER_CLASS_EXISTS) && !defined(CVF_USE_NON_THREADSAFE_REFERENCE_COUNT)
#error No support for atomics. Define CVF_USE_NON_THREADSAFE_REFERENCE_COUNT to be able to compile
#endif

namespace cvf {


//==================================================================================================
//
// Base class for all reference counted objects
//
//==================================================================================================
class Object
{
public:
    inline Object();
    inline virtual ~Object();

    inline int addRef() const;
    inline int release() const;
    inline int refCount() const;

    // Helpers for debugging, see the CVF_TRACK_ACTIVE_OBJECT_INSTANCES define 
    static std::set<Object*>* activeObjectInstances();
    static void               dumpActiveObjectInstances();

private:

#if defined(CVF_USE_NON_THREADSAFE_REFERENCE_COUNT)
    mutable int m_refCount;
#elif defined(CVF_ATOMIC_COUNTER_CLASS_EXISTS)
    mutable AtomicCounter m_refCount;
#else
    #error No support for atomics. Define CVF_USE_NON_THREADSAFE_REFERENCE_COUNT to be able to compile
#endif


    CVF_DISABLE_COPY_AND_ASSIGN(Object);
};



//==================================================================================================
//
// Smart pointer class
//
//==================================================================================================
template <typename T>
class ref
{
public:
    ref(T* object = NULL);
    ref(const ref& other);
    template<typename T2> ref(const ref<T2>& other);
    ~ref();

    ref&                        operator=(T* rhs);
    ref&                        operator=(ref rhs);
    template<typename T2> ref&  operator=(const ref<T2>& rhs);

    inline T*       operator->();
    inline const T* operator->() const;
    inline T&       operator*();
    inline const T& operator*() const;
    inline T*       p();
    inline const T* p() const;
    inline bool     isNull() const;
    inline bool     notNull() const;

    bool            operator<(const ref& rhs) const; 

    void            swap(ref& other);

private:
    T* m_object;
};

/// \relates cvf::ref
/// @{

template<typename T1, typename T2> inline bool operator==(const ref<T1>& a, const ref<T2>& b)   { return a.p() == b.p(); }  ///< Returns true if the internal pointers of refs \a a and \a b are equal.
template<typename T1, typename T2> inline bool operator!=(const ref<T1>& a, const ref<T2>& b)   { return a.p() != b.p(); }  ///< Returns true if the internal pointers of refs \a a and \a b are different.
template<typename T1, typename T2> inline bool operator==(const ref<T1>& a, T2* b)              { return a.p() == b; }      ///< Returns true if the internal pointer of ref \a a is equal to the naked pointer \a b.
template<typename T1, typename T2> inline bool operator!=(const ref<T1>& a, T2* b)              { return a.p() != b; }      ///< Returns true if the internal pointer of ref \a a is different from the naked pointer \a b.
template<typename T1, typename T2> inline bool operator==(T1* a, const ref<T2>& b)              { return a == b.p(); }      ///< Returns true if the naked pointer \a a is equal to the internal pointer of ref \a b.
template<typename T1, typename T2> inline bool operator!=(T1* a, const ref<T2>& b)              { return a != b.p(); }      ///< Returns true if the naked pointer \a a is different from the internal pointer of ref \a b.

/// Swap contents of \a a and \a b. Matches signature of std::swap().
/// \todo Need to investigate which STL algorithms actually utilize the swap() function.
template<typename T> inline void swap(ref<T>& a, ref<T>& b)   { a.swap(b); }              

/// @}

//==================================================================================================
//
// Smart pointer class for const pointers
//
//==================================================================================================
template <typename T>
class cref
{
public:
    cref(const T* object = NULL);
    cref(const cref& other);
    template<typename T2> cref(const cref<T2>& other);
    ~cref();

    cref&                       operator=(const T* rhs);
    cref&                       operator=(cref rhs);
    template<typename T2> cref& operator=(const cref<T2>& rhs);

    inline const T* operator->() const;
    inline const T& operator*() const;
    inline const T* p() const;
    inline bool     isNull() const;
    inline bool     notNull() const;

    bool            operator<(const cref& rhs) const; 

    void            swap(cref& other);

private:
    const T* m_object;
};

/// \relates cvf::cref
/// @{

template<typename T1, typename T2> inline bool operator==(const cref<T1>& a, const cref<T2>& b)  { return a.p() == b.p(); }  ///< Returns true if the internal pointers of refs \a a and \a b are equal.
template<typename T1, typename T2> inline bool operator!=(const cref<T1>& a, const cref<T2>& b)  { return a.p() != b.p(); }  ///< Returns true if the internal pointers of refs \a a and \a b are different.
template<typename T1, typename T2> inline bool operator==(const cref<T1>& a, T2* b)              { return a.p() == b; }      ///< Returns true if the internal pointer of ref \a a is equal to the naked pointer \a b.
template<typename T1, typename T2> inline bool operator!=(const cref<T1>& a, T2* b)              { return a.p() != b; }      ///< Returns true if the internal pointer of ref \a a is different from the naked pointer \a b.
template<typename T1, typename T2> inline bool operator==(T1* a, const cref<T2>& b)              { return a == b.p(); }      ///< Returns true if the naked pointer \a a is equal to the internal pointer of ref \a b.
template<typename T1, typename T2> inline bool operator!=(T1* a, const cref<T2>& b)              { return a != b.p(); }      ///< Returns true if the naked pointer \a a is different from the internal pointer of ref \a b.

//==================================================================================================
//
// Creation of smart pointers.
//
//==================================================================================================
template<typename T, class... Args>
ref<T> make_ref(Args&&... args);

template<typename T, class... Args>
cref<T> make_cref(Args&&... args);

/// @}

}

#include "cvfObject.inl"
