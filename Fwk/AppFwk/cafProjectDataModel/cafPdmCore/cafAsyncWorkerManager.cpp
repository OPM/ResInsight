//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafAsyncWorkerManager.h"

//--------------------------------------------------------------------------------------------------
/// Create a static thread manager that is cleared when the program ends.
//--------------------------------------------------------------------------------------------------
caf::AsyncWorkerManager& caf::AsyncWorkerManager::instance()
{
    static AsyncWorkerManager manager;
    return manager;
}

//--------------------------------------------------------------------------------------------------
/// Destructor waiting for threads to join.
//--------------------------------------------------------------------------------------------------
caf::AsyncWorkerManager::~AsyncWorkerManager()
{
    for ( ThreadAndJoinAtExitPair& workerThreadAndJoinFlag : m_threads )
    {
        if ( workerThreadAndJoinFlag.second && workerThreadAndJoinFlag.first.joinable() )
        {
            workerThreadAndJoinFlag.first.join();
        }
        else
        {
            workerThreadAndJoinFlag.first.detach();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Takes over ownership of the thread and the caller no longer has access to it.
/// A flag can be provided to force the Worker manager to join the threads on exit.
/// Set this to true if it is important that the thread finishes before the program exits.
//--------------------------------------------------------------------------------------------------
void caf::AsyncWorkerManager::takeThreadOwnership( std::thread& thread, bool joinAtExit )
{
    m_threads.push_back( std::make_pair( std::move( thread ), joinAtExit ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AsyncWorkerManager::AsyncWorkerManager()
{
}