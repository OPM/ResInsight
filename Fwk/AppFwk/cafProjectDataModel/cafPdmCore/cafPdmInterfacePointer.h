//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2018 Ceetron Solutions AS
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

#include "cafPdmPointer.h"

namespace caf 
{

template<class T>
class PdmInterfacePointer
{
public:
    inline PdmInterfacePointer () : m_iface(nullptr)                    { }
    inline PdmInterfacePointer ( T * iface )                            { *this = iface; }
    inline PdmInterfacePointer ( const PdmInterfacePointer<T> & other ) { *this = other.p();}

    T* p() const 
    {
        if ( m_implementingPdmObject.notNull() ) 
        {
            return m_iface;
        }
        else 
        {
            return nullptr;
        }
    }

    PdmInterfacePointer<T> & operator= ( T* iface )
    {
        m_implementingPdmObject = nullptr;
        m_iface = nullptr;
        if ( iface != nullptr )
        {
            m_implementingPdmObject = iface->implementingPdmObject();
            m_iface = iface;
        }
        return *this;
    }

    bool                     isNull()     const                                { return m_implementingPdmObject.isNull(); }
    bool                     notNull()    const                                { return m_implementingPdmObject.notNull(); }
    operator                 T* ()        const                                { return p(); }
    T*                       operator->() const                                { return p();  }
    PdmInterfacePointer<T> & operator= ( const PdmInterfacePointer<T>& other ) {  return *this = other.p();}

private:
    T* m_iface; 
    PdmPointer<caf::PdmObjectHandle> m_implementingPdmObject;
};

}