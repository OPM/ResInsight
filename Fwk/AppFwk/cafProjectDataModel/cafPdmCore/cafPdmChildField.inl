#include "cafPdmObjectHandle.h"

#include <vector>
#include <iostream>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmChildField<DataType*>::childObjects(std::vector<PdmObjectHandle*>* objects)
{
    CAF_ASSERT(objects);
    PdmObjectHandle* obj = m_fieldValue.rawPtr();
    if (obj)
    {
        objects->push_back(obj);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmChildField<DataType*>::setChildObject(PdmObjectHandle* object)
{
    if (m_fieldValue.rawPtr() != NULL)
    {
        PdmObjectHandle* oldObject = m_fieldValue.rawPtr();
        this->removeChildObject(oldObject);
        delete oldObject;
    }
    m_fieldValue.setRawPtr(object);
    object->setAsParentField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmChildField<DataType*>::removeChildObject(PdmObjectHandle* object)
{
    if (m_fieldValue.rawPtr() != NULL  && m_fieldValue.rawPtr() == object)
    {
        m_fieldValue.rawPtr()->removeAsParentField(this);
        m_fieldValue.setRawPtr(NULL);
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmChildField<DataType*>::PdmChildField(const DataTypePtr& fieldValue)
{
    if (m_fieldValue) m_fieldValue->removeAsParentField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->setAsParentField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmChildField<DataType*>::~PdmChildField()
{
    delete m_fieldValue.rawPtr();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmChildField<DataType*>& PdmChildField<DataType*>::operator=(const DataTypePtr & fieldValue)
{
    CAF_ASSERT(isInitializedByInitFieldMacro());

    if (m_fieldValue) m_fieldValue->removeAsParentField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->setAsParentField(this);
    return *this;
}

} //End of namespace caf

