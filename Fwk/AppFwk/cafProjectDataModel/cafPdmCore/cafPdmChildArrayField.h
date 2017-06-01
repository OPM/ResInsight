#pragma once

#include "cafPdmPtrArrayFieldHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmPointer.h"


namespace caf
{

template <typename T> class PdmFieldXmlCap;

//==================================================================================================
/// 
/// 
/// 
//==================================================================================================
class PdmChildArrayFieldHandle : public PdmPtrArrayFieldHandle
{
public:
    PdmChildArrayFieldHandle()          {}
    virtual ~PdmChildArrayFieldHandle() {}

    virtual void        deleteAllChildObjects() = 0;
    
    bool                hasSameFieldCountForAllObjects();
};

//==================================================================================================
/// PdmFieldClass to handle a collection of PdmObject derived pointers
/// The reasons for this class is to add itself as parentField into the objects being pointed to.
/// The interface is made similar to std::vector<>, and the complexity of the methods is similar too.
//==================================================================================================

template<typename DataType>
class PdmChildArrayField : public PdmFieldHandle
{
public:
    PdmChildArrayField()
    {
        bool doNotUsePdmPointersFieldForAnythingButPointersToPdmObject = false; CAF_ASSERT(doNotUsePdmPointersFieldForAnythingButPointersToPdmObject);
    }
};


template<typename DataType>
class PdmChildArrayField<DataType*> : public PdmChildArrayFieldHandle
{
    typedef DataType* DataTypePtr;
public:
    PdmChildArrayField()          { }
    virtual ~PdmChildArrayField();

    PdmChildArrayField&   operator() () { return *this; }
    const PdmChildArrayField& operator() () const { return *this; }

    // Reimplementation of PdmPointersFieldHandle methods
  
    virtual size_t      size() const                              { return m_pointers.size(); }
    virtual bool        empty() const                             { return m_pointers.empty(); }
    virtual void        clear();
    virtual void        deleteAllChildObjects(); 
    virtual void        insertAt(int indexAfter, PdmObjectHandle* obj);
    virtual PdmObjectHandle* at(size_t index);

    // std::vector-like access

    DataType*           operator[] (size_t index) const;

    void                push_back(DataType* pointer);
    void                set(size_t index, DataType* pointer);
    void                insert(size_t indexAfter, DataType* pointer);
    void                insert(size_t indexAfter, const std::vector<PdmPointer<DataType> >& objects);
    size_t              count(const DataType* pointer) const;

    void                erase(size_t index);
    size_t              index(DataType* pointer);

    typename std::vector< PdmPointer<DataType> >::iterator begin()        { return m_pointers.begin(); };
    typename std::vector< PdmPointer<DataType> >::iterator end()          { return m_pointers.end(); };

    typename std::vector< PdmPointer<DataType> >::const_iterator begin() const        { return m_pointers.begin();    };
    typename std::vector< PdmPointer<DataType> >::const_iterator end()   const        { return m_pointers.end();      };

    // Child objects

    virtual void        childObjects(std::vector<PdmObjectHandle*>* objects);
    virtual void        removeChildObject(PdmObjectHandle* object);

private: //To be disabled
    PDM_DISABLE_COPY_AND_ASSIGN(PdmChildArrayField);

private:
    void                removeThisAsParentField();
    void                addThisAsParentField();

private:
    friend class PdmFieldXmlCap< PdmChildArrayField<DataType*> >;
    std::vector< PdmPointer<DataType> > m_pointers;
};

} // End of namespace caf

#include "cafPdmChildArrayField.inl"
