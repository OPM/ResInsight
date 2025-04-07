#include "cafAsyncObjectDeleter.h"
#include "cafClassTypeName.h"
#include "cafPdmObjectHandle.h"

namespace caf
{
//==================================================================================================
/// Implementation of PdmChildArrayField<>
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
PdmChildArrayField<DataType*>::~PdmChildArrayField()
{
    deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
DataType* PdmChildArrayField<DataType*>::operator[]( size_t index ) const
{
    return m_pointers[index];
}

//--------------------------------------------------------------------------------------------------
/// Assign a regular raw pointer. This method should be considered private.
/// External use should be considered deprecated.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::push_back( DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.push_back( pointer );
    if ( m_pointers.back() ) m_pointers.back()->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
/// This should be preferred over the method taking a raw pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::push_back( DataTypeUniquePtr pointer )
{
    this->push_back( pointer.release() );
}

//--------------------------------------------------------------------------------------------------
/// Set the value at position index to pointer, overwriting any pointer already present at that
/// position without deleting the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::set( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index] ) m_pointers[index]->removeAsParentField( this );
    m_pointers[index] = pointer;
    if ( m_pointers[index] ) m_pointers[index]->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Set the value at position index to pointer, overwriting any pointer already present at that
/// position without deleting the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::set( size_t index, DataTypeUniquePtr pointer )
{
    this->set( index, pointer.release() );
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::insert( size_t index, DataType* pointer )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, pointer );

    if ( m_pointers[index] ) m_pointers[index]->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Insert pointer at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::insert( size_t index, DataTypeUniquePtr pointer )
{
    this->insert( index, pointer.release() );
}

//--------------------------------------------------------------------------------------------------
/// Insert the pointers at position index, pushing the value previously at that position and all
/// the preceding values backwards
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::insert( size_t index, const std::vector<PdmPointer<DataType>>& objects )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    m_pointers.insert( m_pointers.begin() + index, objects.begin(), objects.end() );

    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin() + index; it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->setAsParentField( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the number of times pointer is referenced from the container.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t PdmChildArrayField<DataType*>::count( const DataType* pointer ) const
{
    size_t itemCount = 0;

    typename std::vector<PdmPointer<DataType>>::const_iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( *it == pointer )
        {
            itemCount++;
        }
    }

    return itemCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
size_t PdmChildArrayField<DataType*>::indexOf( const PdmObjectHandle* obj ) const
{
    for ( size_t i = 0; i < m_pointers.size(); ++i )
    {
        if ( obj == m_pointers[i].rawPtr() )
        {
            return i;
        }
    }

    return (size_t)( -1 ); // Undefined size_t > m_pointers.size();
}

//--------------------------------------------------------------------------------------------------
/// Empty the container without deleting the objects pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::clearWithoutDelete()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    this->removeThisAsParentField();
    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// Deletes all the objects pointed to by the field, then clears the container.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::deleteChildren()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    size_t index;
    for ( index = 0; index < m_pointers.size(); ++index )
    {
        delete ( m_pointers[index].rawPtr() );
    }

    m_pointers.clear();
}

//--------------------------------------------------------------------------------------------------
/// Transfers ownership of the objects pointed to a separate thread.
/// Then clears the container and lets the thread delete the objects.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::deleteChildrenAsync()
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    auto objectsToDelete = m_pointers;

    // Disconnect the connection to observers before deleting the objects
    // https://github.com/OPM/ResInsight/issues/12262
    //
    // See test in \Fwk\AppFwk\cafProjectDataModel\cafPdmCore\cafPdmCore_UnitTests\cafPdmChildArrayFieldHandleTest.cpp
    clearWithoutDelete();

    AsyncPdmObjectVectorDeleter<DataType> pointerDeleter( objectsToDelete );
}

//--------------------------------------------------------------------------------------------------
/// Removes the pointer at index from the container. Does not delete the object pointed to.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::erase( size_t index )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_pointers[index].rawPtr() )
    {
        m_pointers[index].rawPtr()->removeAsParentField( this );
    }

    m_pointers.erase( m_pointers.begin() + index );
}

//--------------------------------------------------------------------------------------------------
/// Assign objects to the field, replacing the current child objects
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::setValue( const std::vector<DataType*>& objects )
{
    clearWithoutDelete();
    for ( auto object : objects )
    {
        push_back( object );
    }
}

//--------------------------------------------------------------------------------------------------
/// Removes all instances of object pointer from the container without deleting the object.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::removeChild( PdmObjectHandle* object )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    std::vector<PdmPointer<DataType>> tempPointers;

    tempPointers = m_pointers;
    m_pointers.clear();

    for ( size_t index = 0; index < tempPointers.size(); ++index )
    {
        if ( tempPointers[index].rawPtr() != object )
        {
            m_pointers.push_back( tempPointers[index] );
        }
        else
        {
            if ( tempPointers[index].rawPtr() )
            {
                tempPointers[index].rawPtr()->removeAsParentField( this );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
std::vector<DataType*> caf::PdmChildArrayField<DataType*>::childrenByType() const
{
    std::vector<DataType*> objects;

    for ( DataType* p : m_pointers )
    {
        if ( p != nullptr )
        {
            objects.push_back( p );
        }
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
[[nodiscard]] std::vector<PdmObjectHandle*> PdmChildArrayField<DataType*>::children() const
{
    std::vector<PdmObjectHandle*> objects;
    for ( size_t i = 0; i < m_pointers.size(); ++i )
    {
        objects.push_back( m_pointers[i].rawPtr() );
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::insertAt( int indexAfter, PdmObjectHandle* obj )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    // This method should assert if obj to insert is not castable to the container type, but since this
    // is a virtual method, its implementation is always created and that makes a dyn_cast add the need for
    // #include of the header file "everywhere"
    typename std::vector<PdmPointer<DataType>>::iterator it;

    if ( indexAfter == -1 )
    {
        m_pointers.push_back( PdmPointer<DataType>() );
        it = m_pointers.end() - 1;
    }
    else
    {
        m_pointers.insert( m_pointers.begin() + indexAfter, PdmPointer<DataType>() );
        it = m_pointers.begin() + indexAfter;
    }

    it->setRawPtr( obj );
    obj->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::insertAt( int indexAfter, std::unique_ptr<PdmObjectHandle> obj )
{
    this->insertAt( indexAfter, obj.release() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
PdmObjectHandle* PdmChildArrayField<DataType*>::at( size_t index )
{
    return m_pointers[index].rawPtr();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::removeThisAsParentField()
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            it->rawPtr()->removeAsParentField( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmChildArrayField<DataType*>::addThisAsParentField()
{
    typename std::vector<PdmPointer<DataType>>::iterator it;
    for ( it = m_pointers.begin(); it != m_pointers.end(); ++it )
    {
        if ( !it->isNull() )
        {
            ( *it )->setAsParentField( this );
        }
    }
}

} // End of namespace caf
