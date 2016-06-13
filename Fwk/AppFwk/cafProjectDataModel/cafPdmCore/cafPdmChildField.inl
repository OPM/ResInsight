
#include <vector>
#include <iostream>
#include <assert.h>
namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmChildField<DataType*>::childObjects(std::vector<PdmObjectHandle*>* objects)
{
    assert(objects);
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
#ifdef _DEBUG
    assert(m_fieldValue.isNull());
#endif

    if (!m_fieldValue.isNull()) m_fieldValue.rawPtr()->removeAsParentField(this);
    m_fieldValue.setRawPtr(NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmChildField<DataType*>& PdmChildField<DataType*>::operator=(const DataTypePtr & fieldValue)
{
    assert(isInitializedByInitFieldMacro());

    if (m_fieldValue) m_fieldValue->removeAsParentField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->setAsParentField(this);
    return *this;
}

} //End of namespace caf

