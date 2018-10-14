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


namespace caf 
{

class PdmObjectHandle;

//==================================================================================================
/// Helper class for the PdmPointer class
/// The add and removing of references is put into a pure static class to 
/// resolve circular include problems.
//
/// Overall idea of the referencing system:
/// The addressToObjectPointer is added to a std::set in the object pointed to.
/// when the object pointed to is deleted, its destructor sets the object pointers 
/// it has addresses to to NULL
//==================================================================================================

class PdmPointerImpl
{
private:
    template < class T > friend class PdmPointer;
    static void addReference(PdmObjectHandle ** addressToObjectPointer);
    static void removeReference(PdmObjectHandle ** addressToObjectPointer);
};

//==================================================================================================
/// Guarded pointer class to point at PdmObjects
/// Use a PdmPointer<SomePdmObject> in the same way as a normal pointer. 
/// The guarding sets the pointer to NULL if the object pointed to dies
///
/// NOTE: This is not reference counting. The user is responsible to delete the objects pointed to.
///       It _can_ be used together with the cvf::ref system if neccesary (this is no recomendation)
//==================================================================================================

template < class T >
class PdmPointer
{
    PdmObjectHandle* m_object;
public :
    inline PdmPointer () : m_object(nullptr)                { }
    inline PdmPointer ( T * p ) : m_object(p)            { PdmPointerImpl::addReference(&m_object); }
    inline PdmPointer ( const PdmPointer<T> & p ) : m_object ( p.m_object ) 
                                                         { PdmPointerImpl::addReference(&m_object); }
    inline ~PdmPointer ()                                { PdmPointerImpl::removeReference(&m_object); }

    T*                 p() const                            { return  static_cast<T*>(const_cast<PdmObjectHandle*>(m_object)); }
    bool             isNull() const                       { return !m_object; }
    bool             notNull() const                      { return !isNull(); }
                    operator T* () const                 { return  static_cast<T*>(const_cast<PdmObjectHandle*>(m_object)); }
    T&                 operator* () const                   { return *static_cast<T*>(const_cast<PdmObjectHandle*>(m_object)); }
    T*                 operator->() const                   { return  static_cast<T*>(const_cast<PdmObjectHandle*>(m_object)); }
    PdmPointer<T> & operator= ( const PdmPointer<T>& p ) { if (this != &p)    PdmPointerImpl::removeReference(&m_object); m_object = p.m_object; PdmPointerImpl::addReference(&m_object); return *this; }
    PdmPointer<T> & operator= ( T* p )                   { if (m_object != p) PdmPointerImpl::removeReference(&m_object); m_object = p;          PdmPointerImpl::addReference(&m_object); return *this; }
    template <class S>
    bool            operator==(const PdmPointer<S>& rhs) const { return m_object == rhs.rawPtr(); }
    // Private methods used by PdmField<T*> and PdmPointersField<T*>. Do not use unless you mean it !
    PdmObjectHandle*      rawPtr() const                       { return m_object; }
    void            setRawPtr( PdmObjectHandle* p)             { if (m_object != p) PdmPointerImpl::removeReference(&m_object); m_object = p;          PdmPointerImpl::addReference(&m_object);  }    
};

} // End of namespace caf

#include <QMetaType>
Q_DECLARE_METATYPE(caf::PdmPointer<caf::PdmObjectHandle>);

