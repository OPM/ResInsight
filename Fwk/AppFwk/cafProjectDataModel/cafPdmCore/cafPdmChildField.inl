#include "cafPdmObjectHandle.h"

#include <iostream>
#include <vector>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PdmChildField<DataType*>::childObjects( std::vector<PdmObjectHandle*>* objects )
{
    CAF_ASSERT( objects );
    PdmObjectHandle* obj = m_fieldValue.rawPtr();
    if ( obj )
    {
        objects->push_back( obj );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PdmChildField<DataType*>::setChildObject( PdmObjectHandle* object )
{
    if ( m_fieldValue.rawPtr() != nullptr )
    {
        PdmObjectHandle* oldObject = m_fieldValue.rawPtr();
        this->removeChildObject( oldObject );
        delete oldObject;
    }
    m_fieldValue.setRawPtr( object );
    object->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PdmChildField<DataType*>::removeChildObject( PdmObjectHandle* object )
{
    if ( m_fieldValue.rawPtr() != nullptr && m_fieldValue.rawPtr() == object )
    {
        m_fieldValue.rawPtr()->removeAsParentField( this );
        m_fieldValue.setRawPtr( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PdmChildField<DataType*>::PdmChildField( const DataTypePtr& fieldValue )
{
    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PdmChildField<DataType*>::PdmChildField( DataTypeUniquePtr fieldValue )
{
    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue.release();
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PdmChildField<DataType*>::~PdmChildField()
{
    delete m_fieldValue.rawPtr();
}

//--------------------------------------------------------------------------------------------------
/// Assign a regular raw pointer. This method should be considered private.
/// External use should be considered deprecated.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PdmChildField<DataType*>& PdmChildField<DataType*>::operator=( const DataTypePtr& fieldValue )
{
    CAF_ASSERT( isInitializedByInitFieldMacro() );

    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
    return *this;
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
/// This should be preferred over the method taking a raw pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
caf::PdmChildField<DataType*>& PdmChildField<DataType*>::operator=( DataTypeUniquePtr fieldValue )
{
    return this->operator=( fieldValue.release() );
}

//--------------------------------------------------------------------------------------------------
/// Assign a regular raw pointer. This method should be considered private.
/// External use should be considered deprecated.
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PdmChildField<DataType*>::setValue( const DataTypePtr& fieldValue )
{
    if ( m_fieldValue ) m_fieldValue->removeAsParentField( this );
    m_fieldValue = fieldValue;
    if ( m_fieldValue != nullptr ) m_fieldValue->setAsParentField( this );
}

//--------------------------------------------------------------------------------------------------
/// Assign a unique pointer and take ownership.
/// This should be preferred over the method taking a raw pointer
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void caf::PdmChildField<DataType*>::setValue( DataTypeUniquePtr fieldValue )
{
    return this->setValue( fieldValue.release() );
}

} // End of namespace caf
