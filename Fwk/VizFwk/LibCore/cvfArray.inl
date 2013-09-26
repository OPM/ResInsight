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
/// \class cvf::Array
/// \ingroup Core
///
/// Templated version of a simple array designed for high performance and no overhead over C arrays 
/// in release (non-check builds).
///
/// Will only work on simple types and classes that do not rely on a constructor or 
/// destructor (as they are not called).
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// Create an empty array
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array()
{
    ground();
}


//--------------------------------------------------------------------------------------------------
/// Create an array with the given number of elements
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array(size_t size)
{
    CVF_ASSERT(size > 0);
    ground();
    resize(size);
}


//--------------------------------------------------------------------------------------------------
/// Create the array and assign (copy) the given data
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array(const T* data, size_t size)
{
    CVF_ASSERT(data);
    CVF_ASSERT(size > 0);

    ground();
    assign(data, size);
}


//--------------------------------------------------------------------------------------------------
/// Create the array and assign (copy) the given data
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array(const Array& other)
: Object(), ValueArray<T>()
{
    ground();

    if (other.size() > 0)
    {
        assign(other.m_data, other.size());
    }
}


//--------------------------------------------------------------------------------------------------
/// Explicit constructor to create the array and assign (copy) the passed value array
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array(const ValueArray<T>& other)
{
    ground();

    if (other.size() > 0)
    {
        resize(other.size());

        size_t i;
        for (i = 0; i < other.size(); i++)
        {
            set(i, other.val(i));
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Explicit constructor to create the array and assign (copy) the passed std::vector
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::Array(const std::vector<T>& other)
{
    ground();

    if (other.size() > 0)
    {
        assign(other);
    }
}


//--------------------------------------------------------------------------------------------------
/// Init all members
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::ground()
{
    m_size = 0;
    m_data = NULL;
    m_capacity = 0;   
    m_sharedData = false;
}


//--------------------------------------------------------------------------------------------------
/// Destructor. Deletes the data.
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>::~Array()
{
    clear();
}


//--------------------------------------------------------------------------------------------------
/// Returns a const reference to the element at the given index.
//--------------------------------------------------------------------------------------------------
template <typename T>
inline const T& Array<T>::operator[] (size_t index) const
{
    CVF_TIGHT_ASSERT(this && m_data);
    CVF_TIGHT_ASSERT(index < m_size);

    return m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// Returns a modifiable reference to the element at the given index.
//--------------------------------------------------------------------------------------------------
template <typename T>
inline T& Array<T>::operator[] (size_t index)
{
    CVF_TIGHT_ASSERT(this && m_data);
    CVF_TIGHT_ASSERT(index < m_size);

    return m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// Assign (copy) the data in the given array
//--------------------------------------------------------------------------------------------------
template <typename T>
Array<T>& Array<T>::operator=(Array rhs)
{
    CVF_TIGHT_ASSERT(!m_sharedData);

    rhs.swap(*this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Resize the array to the given size.
/// The current contents (data) will be kept (as much as possible). Any new elements will be unassigned.
/// Calling resize(0) is the same as clear().
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::resize(size_t size)
{
    CVF_ASSERT(!m_sharedData);

    if (size > 0)
    {
        if (m_data)
        {
            // Copy old data
            T* pTmp = m_data;
            m_data = new T[size];
            
            size_t numToCopy = CVF_MIN(size, m_size);
            cvf::System::memcpy(m_data, size*sizeof(T), pTmp, numToCopy*sizeof(T));

            delete[] pTmp;
        }
        else
        {
            m_data = new T[size];
        }

        m_size = size;
        m_capacity = size;
    }
    else
    {
        clear();
    }
}


//--------------------------------------------------------------------------------------------------
/// Clear the array, freeing all memory (if not shared) and setting capacity and size to zero.
/// If the array was in shared mode, it will not be in shared mode anymore.
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::clear()
{
    if (!m_sharedData) 
    {
        delete[] m_data;
    }

    ground();
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of elements in the array
//--------------------------------------------------------------------------------------------------
template <typename T>
inline size_t Array<T>::size() const
{
    return m_size;
}


//--------------------------------------------------------------------------------------------------
/// Set the element at the given index. Index must be < size()
//--------------------------------------------------------------------------------------------------
template <typename T>
inline void Array<T>::set(size_t index, const T& val)
{
    CVF_TIGHT_ASSERT(index < m_size);

    m_data[index] = val;
}


//--------------------------------------------------------------------------------------------------
/// Set all elements in the array to the given value
//--------------------------------------------------------------------------------------------------
template <typename T>
inline void Array<T>::setAll(const T& val)
{
    size_t i;
    for (i = 0; i < m_size; i++)
    {
        set(i, val);
    }
}


//--------------------------------------------------------------------------------------------------
/// Assign consecutive values to the elements in the array
/// 
/// Example: setConsecutive(2) on an array with size=3 gives: {2,3,4}
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::setConsecutive(const T& startValue)
{
    T val = startValue;
    size_t i;
    for (i = 0; i < m_size; i++)
    {
        set(i, val++);
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns a const reference to the element at the given index
//--------------------------------------------------------------------------------------------------
template <typename T>
const T& cvf::Array<T>::get(size_t index) const
{
    CVF_TIGHT_ASSERT(index < m_size);
    return m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
T cvf::Array<T>::val(size_t index) const
{
    CVF_TIGHT_ASSERT(index < m_size);
    return m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// Returns a const native pointer to the array storing the data
//--------------------------------------------------------------------------------------------------
template <typename T>
inline const T* Array<T>::ptr() const
{
    CVF_TIGHT_ASSERT(this);
    return m_data;
}


//--------------------------------------------------------------------------------------------------
/// Returns the native pointer to the array storing the data
//--------------------------------------------------------------------------------------------------
template <typename T>
inline T* Array<T>::ptr()
{
    CVF_TIGHT_ASSERT(this);
    return m_data;
}



//--------------------------------------------------------------------------------------------------
/// Returns a const native pointer to the array storing the data
//--------------------------------------------------------------------------------------------------
template <typename T>
inline const T* Array<T>::ptr(size_t index) const
{
    CVF_TIGHT_ASSERT(this);
    CVF_TIGHT_ASSERT(index < m_size);

    return &m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// Returns the native pointer to the array storing the data
//--------------------------------------------------------------------------------------------------
template <typename T>
inline T* Array<T>::ptr(size_t index)
{
    CVF_TIGHT_ASSERT(this);
    CVF_TIGHT_ASSERT(index < m_size);

    return &m_data[index];
}


//--------------------------------------------------------------------------------------------------
/// Data shared with another class. Will not delete on destruction. 
///
/// Any method that reallocates data (resize, assign & setPtr) is not allowed and will assert
/// The only way to break the shared 'connection' is via clear()
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::setSharedPtr(T* data, size_t size)
{
    CVF_ASSERT((data && size > 0) || (!data && size == 0));

    clear();

    m_data = data;
    m_size = size;
    m_sharedData = true;
}


//--------------------------------------------------------------------------------------------------
/// Set the data in the array to use the passed array. 
/// 
/// This class takes ownership of the passed data.
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::setPtr(T* data, size_t size)
{
    CVF_ASSERT((data && size > 0) || (!data && size == 0));

    // Not allowed
    // The only way to break the shared 'connection' is via clear()
    CVF_ASSERT(!m_sharedData);
    
    clear();

    m_data = data;
    m_size = size;
}


//--------------------------------------------------------------------------------------------------
/// Delete any current data and set from the given data pointer
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::assign(const T* data, size_t size)
{
    CVF_ASSERT(data);
    CVF_ASSERT(size > 0);
    CVF_ASSERT(!m_sharedData);

    clear();
    resize(size);
    
    // copy
    cvf::System::memcpy(m_data, m_size*sizeof(T), data, size*sizeof(T));
}


//--------------------------------------------------------------------------------------------------
/// Delete any current data and set from the given data
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::assign(const std::vector<T>& data)
{
    CVF_ASSERT(!m_sharedData);

    clear();

    size_t newSize = data.size();
    if (newSize > 0)
    {
        resize(newSize);

        cvf::System::memcpy(m_data, m_size*sizeof(T), &data[0], newSize*sizeof(T));
    }
}


//--------------------------------------------------------------------------------------------------
/// Copy data into the array
/// 
/// \warning Must have enough room for the new data, meaning the array must be resize()'ed before use
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::copyData(const T* pSource, size_t numElementsToCopy, size_t destIndex)
{
    CVF_ASSERT(pSource);
    CVF_ASSERT(numElementsToCopy > 0);
    CVF_ASSERT(destIndex < m_size);
    CVF_ASSERT(destIndex + numElementsToCopy <= m_size);

    cvf::System::memcpy(m_data + destIndex, (m_size - destIndex)*sizeof(T), pSource, numElementsToCopy*sizeof(T));
}


//--------------------------------------------------------------------------------------------------
/// Copy data into the array
/// 
/// \warning Must have enough room for the new data, meaning the array must be resize()'ed before use
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::copyData(const Array<T>& source, size_t numElementsToCopy, size_t destIndex, size_t sourceIndex)
{
    CVF_ASSERT(numElementsToCopy > 0);
    CVF_ASSERT(destIndex < m_size);
    CVF_ASSERT(numElementsToCopy + destIndex <= m_size);

    cvf::System::memcpy(m_data + destIndex, (m_size - destIndex)*sizeof(T), source.ptr() + sourceIndex, numElementsToCopy*sizeof(T));    
}


//--------------------------------------------------------------------------------------------------
/// Copy data into this array with conversion. 
/// Data in source array will be converted using static_cast
//--------------------------------------------------------------------------------------------------
template <typename T>
template <typename U>
void Array<T>::copyConvertedData(const Array<U>& source, size_t numElementsToCopy, size_t destIndex, size_t sourceIndex)
{
    CVF_ASSERT(numElementsToCopy > 0);
    CVF_ASSERT(destIndex < m_size);
    CVF_ASSERT(numElementsToCopy + destIndex <= m_size);

    size_t dst = destIndex;

    size_t i;
    for (i = sourceIndex; i < sourceIndex + numElementsToCopy; i++)
    {
        m_data[dst++] = static_cast<T>(source[i]);
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns an array containing the specified elements
/// 
/// Example:
/// <PRE>
///   this           = {2.0, 5.5, 100.0}
///   elementIndices = {  0,     2,   1,   0,     2}
///   -> output      = {2.0, 100.0, 5.5, 2.0, 100.0}
/// </PRE>
//--------------------------------------------------------------------------------------------------
template <typename T>
template <typename U>
ref<Array<T> > Array<T>::extractElements(const Array<U>& elementIndices) const
{
    ref<Array<T> > arr = new Array<T>;
    size_t numItems = elementIndices.size();
    
    if (numItems > 0)
    {
        arr->resize(numItems);
        
        size_t i;
        for (i = 0; i < numItems; i++)
        {
            U idx = elementIndices[i];
            arr->set(i, get(idx));
        }
    }

    return arr;
}


//--------------------------------------------------------------------------------------------------
/// Copy the contents of the array to the given std::vector
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::toStdVector(std::vector<T>* vec) const
{
    CVF_ASSERT(vec);

    size_t numItems = size();
    vec->resize(numItems);

    size_t i;
    for (i = 0; i < numItems; i++)
    {
        (*vec)[i] = (*this)[i];
    }
}


//--------------------------------------------------------------------------------------------------
/// Get the current capacity of the array (the size of the buffer). This will return a number >= size(), 
/// depending on if reserve() has been used or not.
/// If capacity() > size(), then add() is allowed on the array.
///
/// \sa
///  - reserve()
///  - add()
///  - squeeze()
///  - setSizeZero()
//--------------------------------------------------------------------------------------------------
template <typename T>
size_t Array<T>::capacity() const
{
    return m_capacity;
}


//--------------------------------------------------------------------------------------------------
/// Reserve (allocate) at least the given number of items.
/// If capacity is less than the current buffer, nothing is done.
/// size() of the array is not changed.
///
/// \sa
///  - capacity()
///  - add()
///  - squeeze()
///  - setSizeZero()
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::reserve(size_t capacity)
{
    CVF_ASSERT(!m_sharedData);

    if ((capacity <= m_size) || (capacity <= m_capacity))
    {
        // Required capacity is less than current capacity, done!
        return;
    }

    CVF_ASSERT(capacity > 0);
    CVF_ASSERT(capacity > m_size);

    if (m_data)
    {
        // Copy old data
        T* pTmp = m_data;
        m_data = new T[capacity];
        m_capacity = capacity;

        cvf::System::memcpy(m_data, capacity*sizeof(T), pTmp, m_size*sizeof(T));

        delete[] pTmp;
    }
    else
    {
        // alloc
        m_data = new T[capacity];
        m_capacity = capacity;
    }

    CVF_ASSERT(m_size <= m_capacity);
}


//--------------------------------------------------------------------------------------------------
/// Realloc the array to the current size(). Removing any reserved memory created by reserve().
///
/// \sa
///  - reserve()
///  - capacity()
///  - add()
///  - setSizeZero()
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::squeeze()
{
    CVF_ASSERT(!m_sharedData);

    if (m_data && (m_size < m_capacity))
    {
        if (m_size > 0)
        {
            // Copy old data
            T* pTmp = m_data;
            m_data = new T[m_size];

            cvf::System::memcpy(m_data, m_capacity*sizeof(T), pTmp, m_size*sizeof(T));

            delete[] pTmp;

            m_capacity = m_size;
        }
        else
        {
            clear();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Set the size (number of items) in the array to zero, but keep the buffer. Items in the buffer 
/// are not modified.
///
/// \sa
///  - reserve()
///  - capacity()
///  - add()
///  - squeeze()
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::setSizeZero()
{
    CVF_TIGHT_ASSERT(!m_sharedData);

    m_size = 0;
}


//--------------------------------------------------------------------------------------------------
/// Add an item to the array. 
///
/// Note that this will not grow the array, so the array needs to be pre-allocated with reserve() 
/// before calling this method. The method will assert if not enough space.
///
/// \sa
///  - reserve()
///  - capacity()
///  - squeeze()
///  - setSizeZero()
//--------------------------------------------------------------------------------------------------
template <typename T>
inline void Array<T>::add(const T& val)
{
    CVF_TIGHT_ASSERT(!m_sharedData);
    CVF_TIGHT_ASSERT(m_size < m_capacity);
    
    m_data[m_size] = val;
    m_size++;
}


//--------------------------------------------------------------------------------------------------
/// Get the min value and optionally the index of the (first) min value
/// Works only on arrays with types that implement comparison operators
/// Empty arrays will return std::numeric_limits<T>::max() and not modify index
//--------------------------------------------------------------------------------------------------
template <typename T>
T Array<T>::min(size_t* index) const
{
    T minVal = std::numeric_limits<T>::max();

    size_t i;
    for (i = 0; i < m_size; i++)
    {
        if (m_data[i] < minVal)
        {
            minVal = m_data[i];
            
            if (index) 
            {
                *index = i;
            }
        }
    }

    return minVal;
}


//--------------------------------------------------------------------------------------------------
/// Get the min value and optionally the index of the (first) min value
/// Works only on arrays with types that implement comparison operators
/// Empty arrays will return std::numeric_limits<T>::max() and not modify index
//--------------------------------------------------------------------------------------------------
template <typename T>
T Array<T>::max(size_t* index) const
{
    T maxVal = std::numeric_limits<T>::min();

    size_t i;
    for (i = 0; i < m_size; i++)
    {
        if (m_data[i] > maxVal)
        {
            maxVal = m_data[i];
            
            if (index) 
            {
                *index = i;
            }
        }
    }

    return maxVal;
}


//--------------------------------------------------------------------------------------------------
/// Exchanges the contents of the two arrays.
/// 
/// \param other  Modifiable reference to the array that should have its contents swapped.
/// 
/// \warning Note that signature differs from normal practice. This is done to be 
///          consistent with the signature of std::swap()
//--------------------------------------------------------------------------------------------------
template <typename T>
void Array<T>::swap(Array& other)
{
    using std::swap;

    swap(m_size, other.m_size);    
    swap(m_capacity, other.m_capacity);    
    swap(m_data, other.m_data);    
    swap(m_sharedData, other.m_sharedData);    
}

}  // namespace cvf
