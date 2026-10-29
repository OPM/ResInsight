/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "cafUpdateEditorsScheduler.h"
#include "cafPdmUiItem.h"

namespace caf
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UpdateEditorsScheduler::sm_isAlive = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UpdateEditorsScheduler::UpdateEditorsScheduler()
{
    sm_isAlive = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UpdateEditorsScheduler::~UpdateEditorsScheduler()
{
    sm_isAlive = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UpdateEditorsScheduler* UpdateEditorsScheduler::instance()
{
    static UpdateEditorsScheduler theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UpdateEditorsScheduler::scheduleUpdateConnectedEditors( const PdmUiItem* uiItem )
{
    if ( !uiItem ) return;

    {
        std::lock_guard<std::mutex> lock( m_mutex );
        m_itemsToUpdate.insert( uiItem );
    }

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
/// Remove a deleted item, to avoid updating editors of a deleted item
//--------------------------------------------------------------------------------------------------
void UpdateEditorsScheduler::unscheduleDeletedItem( const PdmUiItem* uiItem )
{
    if ( !sm_isAlive ) return;

    auto scheduler = instance();

    std::lock_guard<std::mutex> lock( scheduler->m_mutex );
    scheduler->m_itemsToUpdate.erase( uiItem );
    scheduler->m_itemsBeingUpdated.erase( uiItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UpdateEditorsScheduler::performScheduledUpdates()
{
    // Updating editors can delete other scheduled items or schedule new updates. Process a snapshot one item at a time,
    // so deleted items are removed by unscheduleDeletedItem() before they are visited.
    // The lock is not held while updating editors, as the update can delete items or schedule new updates.
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        m_itemsBeingUpdated.insert( m_itemsToUpdate.begin(), m_itemsToUpdate.end() );
        m_itemsToUpdate.clear();
    }

    while ( true )
    {
        const PdmUiItem* uiItem = nullptr;
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            if ( m_itemsBeingUpdated.empty() ) break;

            uiItem = *m_itemsBeingUpdated.begin();
            m_itemsBeingUpdated.erase( m_itemsBeingUpdated.begin() );
        }

        uiItem->updateConnectedEditors();
    }
}

} //namespace caf
