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


    
//==================================================================================================
///
/// \class cvf::Collection
/// \ingroup Core
/// 
/// A collection class for reference counted objects (that derive from Object). 
///    
/// The class add a reference to all objects when added to the array, and releases them when removed 
/// from the array.
/// 
/// The class exposes the same public interface as a std::vector, so see the STL documentation
/// for documentation on most of the methods.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor. Create an empty collection
//--------------------------------------------------------------------------------------------------
template<typename T>
Collection<T>::Collection()
{
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
template<typename T>
Collection<T>::Collection(const Collection& other)
:   m_vector(other.m_vector)
{
}


//--------------------------------------------------------------------------------------------------
/// Create a collection and copy the contents from the passed collection
//--------------------------------------------------------------------------------------------------
template<typename T>
Collection<T>::Collection(const std::vector< ref<T> >& vector)
:   m_vector(vector)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
Collection<T>& Collection<T>::operator=(Collection rhs)
{
    rhs.swap(*this);
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Set the collection to contain the same elements as the passed collection
//--------------------------------------------------------------------------------------------------
template<typename T>
Collection<T>& Collection<T>::operator=(const std::vector< ref<T> >& vector)
{
    m_vector = vector;

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
const ref<T>& Collection<T>::operator[](size_t index) const    
{ 
    return m_vector[index]; 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
ref<T>& Collection<T>::operator[](size_t index)                
{ 
    return m_vector[index]; 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::push_back(T* data)      	
{ 
    m_vector.push_back(data); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
const T* Collection<T>::at(size_t index) const	
{ 
    return m_vector[index].p(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
T* Collection<T>::at(size_t index)				
{ 
    return m_vector[index].p(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::resize(size_t size)     	
{ 
    m_vector.resize(size); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
size_t Collection<T>::size() const             	
{ 
    return m_vector.size(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::reserve(size_t capacity)	
{ 
    m_vector.reserve(capacity); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
size_t Collection<T>::capacity() const      	
{ 
    return m_vector.capacity(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::clear()                 	
{ 
    m_vector.clear(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
bool Collection<T>::empty() const           	
{ 
    return m_vector.empty(); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
bool Collection<T>::contains(const T* data) const
{
    if (std::find(m_vector.begin(), m_vector.end(), data) != m_vector.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// Find index of the first occurence of the specified element
///
/// Returns UNDEFINED_SIZE_T if element could not be found
//--------------------------------------------------------------------------------------------------
template<typename T>
size_t Collection<T>::indexOf(const T* data) const
{
    typename std::vector<ref<T> >::const_iterator it = std::find(m_vector.begin(), m_vector.end(), data);
    if (it != m_vector.end())
    {
        return static_cast<size_t>(it - m_vector.begin());
    }
    else
    {
        return UNDEFINED_SIZE_T;
    }
}


//--------------------------------------------------------------------------------------------------
/// Erase the specified element
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::erase(const T* data) 
{
    typename std::vector<ref<T> >::iterator it = std::find(m_vector.begin(), m_vector.end(), data);
    if (it != m_vector.end())
    {
        m_vector.erase(it);
    }
}


//--------------------------------------------------------------------------------------------------
/// Erase the element at the specified index
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::eraseAt(size_t index) 
{ 
    CVF_ASSERT(index < m_vector.size());

    // Cast may have to be to std::vector<T>::difference_type
    m_vector.erase(m_vector.begin() + static_cast<ptrdiff_t>(index)); 
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
void Collection<T>::swap(Collection& other)		
{ 
    m_vector.swap(other.m_vector); 
}


//--------------------------------------------------------------------------------------------------
/// Return iterator to beginning
//--------------------------------------------------------------------------------------------------
template<typename T>
typename Collection<T>::iterator Collection<T>::begin()
{
    return m_vector.begin();
}


//--------------------------------------------------------------------------------------------------
/// Return iterator to end
//--------------------------------------------------------------------------------------------------
template<typename T>
typename Collection<T>::iterator Collection<T>::end()
{
    return m_vector.end();
}


//--------------------------------------------------------------------------------------------------
/// Return const iterator to beginning
//--------------------------------------------------------------------------------------------------
template<typename T>
typename Collection<T>::const_iterator Collection<T>::begin() const
{
    return m_vector.begin();
}


//--------------------------------------------------------------------------------------------------
/// Return const iterator to end
//--------------------------------------------------------------------------------------------------
template<typename T>
typename Collection<T>::const_iterator Collection<T>::end() const
{
    return m_vector.end();
}

}  // namespace cvf
