#include <thread>

#include "cafAsyncWorkerManager.h"
#include "cafPdmObjectHandle.h"

namespace caf
{
    //--------------------------------------------------------------------------------------------------
    /// Constructor that takes ownership of the data in the provided vector
    //--------------------------------------------------------------------------------------------------
    template<typename PdmObjectType>
    AsyncPdmObjectVectorDeleter<PdmObjectType>::AsyncPdmObjectVectorDeleter(std::vector<PdmObjectType*>& pointerVector)        
    {
        m_pointersToDelete.reserve(pointerVector.size());
        for (PdmObjectType* rawPointer : pointerVector)
        {
            if (rawPointer)
            {
                PdmObjectHandle* objectHandle = static_cast<PdmObjectHandle*>(rawPointer);
                objectHandle->prepareForDelete();
                m_pointersToDelete.push_back(objectHandle);
            }
        }
        pointerVector.clear();
    }

    //--------------------------------------------------------------------------------------------------
    /// Constructor that takes ownership of the data in the provided vector
    //--------------------------------------------------------------------------------------------------
    template<typename PdmObjectType>
    AsyncPdmObjectVectorDeleter<PdmObjectType>::AsyncPdmObjectVectorDeleter(std::vector<PdmPointer<PdmObjectType>>& pdmPointerVector)
    {
        m_pointersToDelete.reserve(pdmPointerVector.size());
        for (PdmPointer<PdmObjectType>& pdmPointer : pdmPointerVector)
        {
            if (pdmPointer.notNull())
            {
                PdmObjectHandle* objectHandle = pdmPointer.rawPtr();
                objectHandle->prepareForDelete();
                m_pointersToDelete.push_back(objectHandle);
            }
        }
        pdmPointerVector.clear();
    }

    //--------------------------------------------------------------------------------------------------
    /// Destructor will launch the asynchronous deletion if start() hasn't already been run.
    //--------------------------------------------------------------------------------------------------
    template<typename PdmObjectType>
    AsyncPdmObjectVectorDeleter<PdmObjectType>::~AsyncPdmObjectVectorDeleter()
    {
        if (!m_pointersToDelete.empty())
        {
            start();
        }
    }
  
    //--------------------------------------------------------------------------------------------------
    /// Perform deletion of the pointers in a separate thread.    
    //--------------------------------------------------------------------------------------------------
    template<typename PdmObjectType>
    void AsyncPdmObjectVectorDeleter<PdmObjectType>::start()
    {
        std::thread thread([](std::vector<PdmObjectHandle*>&& pointerVector)
        {
            for (PdmObjectHandle* pointerToDelete : pointerVector)
            {
                delete pointerToDelete;
            }
        }, std::move(m_pointersToDelete));
        AsyncWorkerManager::instance().takeThreadOwnership(thread, false);
    }

}