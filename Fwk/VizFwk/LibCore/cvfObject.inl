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


namespace cvf {


//--------------------------------------------------------------------------------------------------
/// Constructor. Set the ref count to zero
//--------------------------------------------------------------------------------------------------
inline Object::Object()
:   m_refCount(0)
{
#if ((CVF_TRACK_ACTIVE_OBJECT_INSTANCES >= 1 && defined(_DEBUG)) || CVF_TRACK_ACTIVE_OBJECT_INSTANCES == 2)
    activeObjectInstances()->insert(this);
#endif
}


//--------------------------------------------------------------------------------------------------
/// Destructor. Asserts that the ref count is zero.
//--------------------------------------------------------------------------------------------------
inline Object::~Object()
{
    CVF_ASSERT(m_refCount == 0);

#if ((CVF_TRACK_ACTIVE_OBJECT_INSTANCES >= 1 && defined(_DEBUG)) || CVF_TRACK_ACTIVE_OBJECT_INSTANCES == 2)
    activeObjectInstances()->erase(this);
#endif
}


//--------------------------------------------------------------------------------------------------
/// Increments reference count.
/// 
/// \return  The new reference count.
//--------------------------------------------------------------------------------------------------
inline int Object::addRef() const
{
    CVF_TIGHT_ASSERT(this);
    return ++m_refCount;
}


//--------------------------------------------------------------------------------------------------
/// Decrements the reference count. Returns the new reference count.
/// 
/// \return  The new reference count.
/// 
/// If the new reference count is zero, the object (this) is deleted.\n
//--------------------------------------------------------------------------------------------------
inline int Object::release() const
{
    CVF_TIGHT_ASSERT(m_refCount > 0);

    if (--m_refCount == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refCount;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the current reference count
//--------------------------------------------------------------------------------------------------
inline int Object::refCount() const
{
    CVF_TIGHT_ASSERT(this);
    return m_refCount;
}



//==================================================================================================
///
/// \class cvf::ref
/// \ingroup Core
///
/// Smart pointer class used for handling reference counted objects (that derive from Object). 
///
/// The ref<T> class encapsulates reference counting by calling Object::addRef() and Object::release() 
/// on the internally stored object pointer. 
///
/// \internal Inspired by boost's intrusive_ptr class.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor from naked pointer 
/// 
/// Will call addRef() on the passed object.
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>::ref(T* object)
{
    if (object)
    {
        object->addRef();
        m_object = object;
    }
    else
    {
        m_object = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/// Copy constructor
///  
/// Copies the internal pointer from the passed ref object and calls addRef() on the object.
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>::ref(const ref& other)
{
    if (other.m_object)
    {
        other.m_object->addRef();
        m_object = other.m_object;
    }
    else
    {
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Construct from related. 
/// 
/// Copies the internal pointer from the passed ref object and calls addRef() on the object.
//--------------------------------------------------------------------------------------------------
template<typename T>
template<typename T2>
ref<T>::ref(const ref<T2>& other)
{
    if (other.notNull())
    {
        other->addRef();

        // Escape const
        m_object = const_cast<T2*>(other.p());
    }
    else
    {
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Destructor. 
/// 
/// Will call release() on the internal pointer.
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>::~ref()
{
    if (m_object)
    {
        m_object->release();
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Assignment from raw pointer. 
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>& ref<T>::operator=(T* rhs)
{
    // Copy-and-swap (on temporary obj)
    ref<T>(rhs).swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
/// 
/// Copies the internal pointer from \a rhs and calls addRef().
/// If we're already storing an internal pointer, this pointer will be release()'ed. 
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>& ref<T>::operator=(ref rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Assignment from related.
/// 
/// Copies the internal pointer from \a rhs and calls addRef().
/// If we're already storing an internal pointer, this pointer will be release()'ed. 
//--------------------------------------------------------------------------------------------------
template<typename T>
template<typename T2>
ref<T>& ref<T>::operator=(const ref<T2>& rhs)
{
    // Copy-and-swap (on temporary obj)
    ref<T>(rhs).swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Returns the naked pointer this object is associated with. 
/// 
/// Added to be able to write the same code for smart pointers as for normal naked pointers.
/// 
/// \code
/// ref<MyObject> myObject = new MyObject;
/// myObject->doSomething();                // Easier than myObject.p()->doSomething()
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename T>
inline T* ref<T>::operator->()
{
    CVF_TIGHT_ASSERT(m_object); 
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// Returns naked const pointer this object is associated with. 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T* ref<T>::operator->() const
{
    CVF_TIGHT_ASSERT(m_object); 
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// Dereference operator returning a modifiable reference to the associated object. 
/// 
/// Added to be able to write the same code for smart pointers as for normal naked pointers.
/// \code
/// ref<MyObject> myObject = new MyObject;
/// *myObject.doSomething();
/// \endcode
//--------------------------------------------------------------------------------------------------
template<typename T>
inline T& ref<T>::operator*()
{
    CVF_TIGHT_ASSERT(m_object); 
    return *m_object;
}


//--------------------------------------------------------------------------------------------------
/// Dereference operator returning a const reference to the associated object. 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T& ref<T>::operator*() const
{
    CVF_TIGHT_ASSERT(m_object); 
    return *m_object;
}


//--------------------------------------------------------------------------------------------------
/// Returns the naked pointer
//--------------------------------------------------------------------------------------------------
template<typename T>
inline T* ref<T>::p()
{
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// Returns naked const pointer
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T* ref<T>::p() const
{
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the internal pointer is NULL
//--------------------------------------------------------------------------------------------------
template<typename T>
inline bool ref<T>::isNull() const
{
    return m_object == NULL;
}


//--------------------------------------------------------------------------------------------------
/// Returns true if the internal pointer is different from NULL 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline bool ref<T>::notNull() const
{
    return m_object != NULL;
}


//--------------------------------------------------------------------------------------------------
/// Does less than comparison of two ref objects.
/// 
/// \return Returns true if this object's internal pointer is less than the internal pointer of \a rhs.
/// 
/// The comparison is done by comparing the internal pointer values ( this.p() < rhs.p() )
/// This operator is used in several STL collections (e.g. std::set) as well as in many STL algorithms
/// (e.g. std::sort() ).
//--------------------------------------------------------------------------------------------------
template<typename T>
bool ref<T>::operator<(const ref& rhs) const
{
    CVF_TIGHT_ASSERT((m_object != NULL) && (rhs.p() != NULL));
    return std::less<T*>()(m_object, rhs.m_object);
}


//--------------------------------------------------------------------------------------------------
/// Exchanges the contents of the two smart pointers.
/// 
/// \param other  Modifiable reference to the ref object that should have its contents swapped.
/// 
/// Swap the associated pointer in this and the passed ref object. Will not modify the reference 
/// count of any of the associated objects.
/// 
/// \warning Note that signature differs from normal practice. This is done to be 
///          consistent with the signature of std::swap()
//--------------------------------------------------------------------------------------------------
template<typename T>
void ref<T>::swap(ref& other)
{
    T* tmp = other.m_object;
    other.m_object = m_object; 
    m_object = tmp;
}


//==================================================================================================
///
/// \class cvf::cref
/// \ingroup Core
///
/// Smart pointer class used for handling const reference counted objects
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Construct from naked pointer
//--------------------------------------------------------------------------------------------------
template<typename T>
cref<T>::cref(const T* object)
{
    if (object)
    {
        object->addRef();
        m_object = object;
    }
    else
    {
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
template<typename T>
cref<T>::cref(const cref& other)
{
    if (other.m_object)
    {
        other.m_object->addRef();
        m_object = other.m_object;
    }
    else
    {
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Construct from related
//--------------------------------------------------------------------------------------------------
template<typename T>
template<typename T2>
cref<T>::cref(const cref<T2>& other)
{
    if (other.notNull())
    {
        other->addRef();
        m_object = other.p();
    }
    else
    {
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
cref<T>::~cref()
{
    if (m_object)
    {
        m_object->release();
        m_object = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
cref<T>& cref<T>::operator=(const T* rhs)
{
    // Copy-and-swap (on temporary obj)
    cref<T>(rhs).swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
cref<T>& cref<T>::operator=(cref rhs)
{
    // Copy-and-swap (copy already done since parameter is passed by value)
    rhs.swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
template<typename T2>
cref<T>& cref<T>::operator=(const cref<T2>& rhs)
{
    // Copy-and-swap (on temporary obj)
    cref<T>(rhs).swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T* cref<T>::operator->() const
{
    CVF_TIGHT_ASSERT(m_object); 
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T& cref<T>::operator*() const
{
    CVF_TIGHT_ASSERT(m_object); 
    return *m_object;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline const T* cref<T>::p() const
{
    return m_object;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline bool cref<T>::isNull() const
{
    return m_object == NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
inline bool cref<T>::notNull() const
{
    return m_object != NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
bool cref<T>::operator<(const cref& rhs) const
{
    CVF_TIGHT_ASSERT((m_object != NULL) && (rhs.p() != NULL));
    return std::less<const T*>()(m_object, rhs.m_object);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void cref<T>::swap(cref& other)
{
    const T* tmp = other.m_object;
    other.m_object = m_object; 
    m_object = tmp;
}



//--------------------------------------------------------------------------------------------------
/// Creation of smart pointers.
//--------------------------------------------------------------------------------------------------
template<typename T, class... Args>
ref<T> make_ref(Args&&... args)
{
    return cvf::ref(new T(args...));
}

//--------------------------------------------------------------------------------------------------
/// Creation of const smart pointers.
//--------------------------------------------------------------------------------------------------
template<typename T, class... Args>
cref<T> make_cref(Args&&... args)
{
    return cvf::cref(new T(args...));
}



}  // namespace cvf

