#pragma once

#include "cafPdmPointer.h"

#include "cafPdmFieldHandle.h"
#include <assert.h>

namespace caf
{

template< typename T> class PdmFieldXmlCap;

//==================================================================================================
/// A field that contains a pointer to a PdmObjectHandle derived object.
/// The referenced object will not be printed in the XML-output yet, but
/// it is intended to be written as a reference (by path from common root)
/// This field has nothing to do with ownership at all, and is not a part of the 
/// parent-child relations induced by the other PdmChildField<PdmPtrType*> PdmChildArrayField<PdmPtrType*> 
/// The pointer is guarded, meaning that it will be set to NULL if the object pointed to 
/// is deleted. 
//==================================================================================================

template<typename DataType>
class PdmPtrField : public PdmFieldHandle
{
public:
    PdmPtrField()
    {
        bool doNotUsePdmPtrFieldForAnythingButPointersToPdmObject = false; assert(doNotUsePdmPtrFieldForAnythingButPointersToPdmObject);
    }
};

template<typename DataType >
class PdmPtrField <DataType*> : public PdmFieldHandle
{
    typedef DataType* DataTypePtr;
public:
    typedef PdmPointer<DataType> FieldDataType;

    PdmPtrField()                                                          { m_referenceString = ""; m_isResolved = false; }
    explicit PdmPtrField(const DataTypePtr& fieldValue);                
    virtual ~PdmPtrField();

    //  Assignment 

    PdmPtrField&                operator= (const DataTypePtr & fieldValue);
    PdmPtrField&                operator= (const FieldDataType & fieldValue);

    // Basic access 

    DataType*                   value() const                               { return m_fieldValue;  }
    void                        setValue(const DataTypePtr& fieldValue);    

    // Access operators

    /*Conversion*/              operator DataType* () const                 { return m_fieldValue; }
    DataType*                   operator->() const                          { return m_fieldValue; }

    const PdmPointer<DataType>& operator()() const                          { return m_fieldValue; }
    const PdmPointer<DataType>& v() const                                   { return m_fieldValue; }

    bool                        operator==(const DataTypePtr& fieldValue)   { return m_fieldValue == fieldValue; }

    // Child objects
    
    virtual void childObjects(std::vector<PdmObjectHandle*>*);

    // Ptr referenced objects

    virtual void ptrReferencedObjects(std::vector<PdmObjectHandle*>* objectsToFill);


private:
    PDM_DISABLE_COPY_AND_ASSIGN(PdmPtrField);

    friend class PdmFieldXmlCap< PdmPtrField <DataType*> >;
    void setRawPtr(PdmObjectHandle* obj);

    PdmPointer<DataType>        m_fieldValue;

    // Resolving
    QString                     m_referenceString;
    bool                        m_isResolved;
};

} // End of namespace caf

#include "cafPdmPtrField.inl"

#include <QMetaType>
Q_DECLARE_METATYPE(caf::PdmPointer<caf::PdmObjectHandle>);
