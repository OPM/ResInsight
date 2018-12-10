#include <QVariant>

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename DataType >
QVariant caf::PdmPtrField<DataType*>::toQVariant() const
{
    caf::PdmObjectHandle* objectHandle = m_fieldValue.rawPtr();
    caf::PdmPointer<caf::PdmObjectHandle> ptrHandle(objectHandle);
    return QVariant::fromValue(ptrHandle);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmPtrField<DataType*>::setFromQVariant(const QVariant& variant)
{
    caf::PdmPointer<caf::PdmObjectHandle> variantHandle = variant.value<caf::PdmPointer<caf::PdmObjectHandle>>();
    m_fieldValue.setRawPtr(variantHandle.rawPtr());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>::PdmPtrField(const DataTypePtr& fieldValue)
{
    m_isResolved = true; 
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>::~PdmPtrField()
{
    if (!m_fieldValue.isNull()) m_fieldValue.rawPtr()->removeReferencingPtrField(this);
    m_fieldValue.setRawPtr(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::setValue(const DataTypePtr& fieldValue)
{
    CAF_ASSERT(isInitializedByInitFieldMacro());

    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::setRawPtr(PdmObjectHandle* obj)
{
    CAF_ASSERT(isInitializedByInitFieldMacro());

    if (m_fieldValue.notNull()) m_fieldValue.rawPtr()->removeReferencingPtrField(this);
    m_fieldValue.setRawPtr(obj);
    if (m_fieldValue.notNull()) m_fieldValue.rawPtr()->addReferencingPtrField(this);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>& PdmPtrField<DataType*>::operator=(const DataTypePtr & fieldValue)
{
    CAF_ASSERT(isInitializedByInitFieldMacro());

    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
caf::PdmPtrField<DataType*>& PdmPtrField<DataType*>::operator=(const FieldDataType & fieldValue)
{
	CAF_ASSERT(isInitializedByInitFieldMacro());

    if (m_fieldValue) m_fieldValue->removeReferencingPtrField(this);
    m_fieldValue = fieldValue;
    if (m_fieldValue != NULL) m_fieldValue->addReferencingPtrField(this);

    return *this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmPtrField<DataType*>::ptrReferencedObjects(std::vector<PdmObjectHandle*>* objectsToFill)
{
    if (m_fieldValue.rawPtr())
    {
        objectsToFill->push_back(m_fieldValue.rawPtr());
    }
}

} // End of namespace caf

