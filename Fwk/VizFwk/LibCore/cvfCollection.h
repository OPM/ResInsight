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

#include "cvfObject.h"
#include "cvfMath.h"
#include <vector>
#include <algorithm>

namespace cvf {


//==================================================================================================
//
// A collection class for reference counted objects (that derive from Object)
//
//==================================================================================================
template<typename T>
class Collection
{
public:
    Collection();
    Collection(const Collection& other);
    explicit Collection(const std::vector< ref<T> >& vector);

    Collection&     operator=(Collection rhs);
    Collection&     operator=(const std::vector< ref<T> >& vector);
    const ref<T>&   operator[](size_t index) const;
    ref<T>&         operator[](size_t index);

    void            push_back(T* data);
    const T*        at(size_t index) const;
    T*              at(size_t index);

    void            resize(size_t size);
    size_t          size() const;
    void            reserve(size_t capacity);
    size_t          capacity() const;
    void            clear();

    bool            empty() const;
    bool            contains(const T* data) const;
    size_t          indexOf(const T* data) const;

    void            erase(const T* data);
    void            eraseAt(size_t index);

    void            swap(Collection& other);

// Iterator support
public:
    typedef typename std::vector<ref<T> >::iterator       iterator;
    typedef typename std::vector<ref<T> >::const_iterator const_iterator;

    iterator        begin();
    iterator        end();
    const_iterator  begin() const;
    const_iterator  end() const;

private:
    std::vector<ref<T> > m_vector;
};

}

#include "cvfCollection.inl"
