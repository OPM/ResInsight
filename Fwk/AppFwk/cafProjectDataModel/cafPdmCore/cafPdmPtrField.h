#pragma once

#include "cafAssert.h"
#include "cafPdmPointer.h"

#include "cafPdmValueField.h"

namespace caf
{
template <typename T>
class PdmFieldXmlCap;

//==================================================================================================
/// A field that contains a pointer to a PdmObjectHandle derived object.
/// The referenced object will not be printed in the XML-output yet, but
/// it is intended to be written as a reference (by path from common root)
/// This field has nothing to do with ownership at all, and is not a part of the
/// parent-child relations induced by the other PdmChildField<PdmPtrType*> PdmChildArrayField<PdmPtrType*>
/// The pointer is guarded, meaning that it will be set to NULL if the object pointed to
/// is deleted.
//==================================================================================================

template <typename DataType>
class PdmPtrField : public PdmValueField
{
public:
    PdmPtrField()
    {
        bool doNotUsePdmPtrFieldForAnythingButPointersToPdmObject = false;
        CAF_ASSERT( doNotUsePdmPtrFieldForAnythingButPointersToPdmObject );
    }
};

template <typename DataType>
class PdmPtrField<DataType*> : public PdmValueField
{
    typedef DataType* DataTypePtr;

public:
    typedef PdmPointer<DataType> FieldDataType;

    PdmPtrField() {}
    explicit PdmPtrField( const DataTypePtr& fieldValue );
    ~PdmPtrField() override;

    PdmPtrField& operator=( const DataTypePtr& fieldValue );
    PdmPtrField& operator=( const FieldDataType& fieldValue );

    DataType* value() const { return m_fieldValue; }
    void      setValue( const DataTypePtr& fieldValue );

    QVariant toQVariant() const override;
    void     setFromQVariant( const QVariant& variant ) override;
    bool     isReadOnly() const override { return false; }

    operator DataType*() const { return m_fieldValue; }
    DataType* operator->() const { return m_fieldValue; }
    DataType* operator()() const { return m_fieldValue; }
    bool      operator==( const DataTypePtr& fieldValue ) { return m_fieldValue == fieldValue; }

    std::vector<PdmObjectHandle*> ptrReferencedObjects() const override;

private:
    PDM_DISABLE_COPY_AND_ASSIGN( PdmPtrField );

    friend class PdmFieldXmlCap<PdmPtrField<DataType*>>;
    void setRawPtr( PdmObjectHandle* obj );

    PdmPointer<DataType> m_fieldValue;
};

} // End of namespace caf

#include "cafPdmPtrField.inl"
