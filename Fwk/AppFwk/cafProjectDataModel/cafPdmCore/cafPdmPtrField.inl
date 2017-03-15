namespace caf
{

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

