#pragma once

#include "cafPdmFieldHandle.h"

#include "cafAssert.h"
#include "cafPdmPointer.h"

namespace caf
{
template <typename T>
class PdmFieldXmlCap;
//==================================================================================================
/// Specialization for pointers, but only applicable to PdmObject derived objects.
/// The pointer is guarded, meaning that it will be set to NULL if the object pointed to
/// is deleted. The referenced object will be printed in place in the xml-file
/// This is supposed to be renamed to PdmChildField
//==================================================================================================

class PdmChildFieldHandle : public PdmFieldHandle
{
public:
    virtual void childObjects( std::vector<PdmObjectHandle*>* objects ) = 0;
    virtual void setChildObject( PdmObjectHandle* object )              = 0;
};

template <typename DataType>
class PdmChildField : public PdmChildFieldHandle
{
public:
    PdmChildField()
    {
        bool doNotUsePdmPtrFieldForAnythingButPointersToPdmObject = false;
        CAF_ASSERT( doNotUsePdmPtrFieldForAnythingButPointersToPdmObject );
    }
};

template <typename DataType>
class PdmChildField<DataType*> : public PdmChildFieldHandle
{
    typedef DataType* DataTypePtr;

public:
    PdmChildField() {}
    explicit PdmChildField( const DataTypePtr& fieldValue );
    virtual ~PdmChildField();

    // Assignment

    PdmChildField& operator=( const DataTypePtr& fieldValue );

    // Basic access

    DataType* value() const { return m_fieldValue; }
    void      setValue( const DataTypePtr& fieldValue );

    // Access operators

    /*Conversion*/ operator DataType*() const { return m_fieldValue; }
    DataType*      operator->() const { return m_fieldValue; }

    const PdmPointer<DataType>& operator()() const { return m_fieldValue; }
    const PdmPointer<DataType>& v() const { return m_fieldValue; }

    // Child objects
    virtual void childObjects( std::vector<PdmObjectHandle*>* objects ) override;
    void         setChildObject( PdmObjectHandle* object ) override;
    virtual void removeChildObject( PdmObjectHandle* object ) override;

private:
    PDM_DISABLE_COPY_AND_ASSIGN( PdmChildField );

    friend class PdmFieldXmlCap<PdmChildField<DataType*>>;
    PdmPointer<DataType> m_fieldValue;
};

} // End of namespace caf

#include "cafPdmChildField.inl"
