#pragma once

#include "cafPdmFieldHandle.h"

namespace caf
{

//==================================================================================================
/// 
/// 
/// 
//==================================================================================================
class PdmPtrArrayFieldHandle : public PdmFieldHandle
{
public:
    PdmPtrArrayFieldHandle()          {}
    ~PdmPtrArrayFieldHandle() override {}

    virtual size_t      size() const = 0;
    virtual bool        empty() const = 0;
    virtual void        clear() = 0;
    virtual void        insertAt(int indexAfter, PdmObjectHandle* obj) = 0;
    virtual void        erase(size_t index) = 0;
    
    virtual PdmObjectHandle* at(size_t index) = 0;

};

}