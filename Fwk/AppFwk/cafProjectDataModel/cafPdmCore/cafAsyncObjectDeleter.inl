#include <thread>

namespace caf
{
    //--------------------------------------------------------------------------------------------------
    /// Constructor that takes ownership of the data in the provided vector
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    AsyncRawPointerVectorDeleter<T>::AsyncRawPointerVectorDeleter(std::vector<T*>&& pointerVector)
        : pointersToDelete_(std::move(pointerVector))
    {
    }

    //--------------------------------------------------------------------------------------------------
    /// Destructor will launch the asynchronous deletion if start() hasn't already been run.
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    AsyncRawPointerVectorDeleter<T>::~AsyncRawPointerVectorDeleter()
    {
        if (!pointersToDelete_.empty())
        {
            start();
        }
    }

    //--------------------------------------------------------------------------------------------------
    /// Perform deletion of the pointers in a separate thread.    
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    void AsyncRawPointerVectorDeleter<T>::start()
    {
        std::thread([](std::vector<T*>&& pointerVector)
        {
            for (T* pointerToDelete : pointerVector)
            {
                delete pointerToDelete;
            }
        }, std::move(pointersToDelete_)).detach();
    }


    //--------------------------------------------------------------------------------------------------
    /// Constructor that takes ownership of the data in the provided vector
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    AsyncPdmPointerVectorDeleter<T>::AsyncPdmPointerVectorDeleter(std::vector<PdmPointer<T>>&& pdmPointerVector)
        : pointersToDelete_(std::move(pdmPointerVector))
    {
    }

    //--------------------------------------------------------------------------------------------------
    /// Destructor will launch the asynchronous deletion if start() hasn't already been run.
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    AsyncPdmPointerVectorDeleter<T>::~AsyncPdmPointerVectorDeleter()
    {
        if (!pointersToDelete_.empty())
        {
            start();
        }
    }
  
    //--------------------------------------------------------------------------------------------------
    /// Perform deletion of the pointers in a separate thread.    
    //--------------------------------------------------------------------------------------------------
    template<typename T>
    void AsyncPdmPointerVectorDeleter<T>::start()
    {
        std::thread([](std::vector<PdmPointer<T>>&& pointerVector)
        {
            for (PdmPointer<T>& pointerToDelete : pointerVector)
            {
                delete pointerToDelete.rawPtr();
            }
        }, std::move(pointersToDelete_)).detach();
    }

}