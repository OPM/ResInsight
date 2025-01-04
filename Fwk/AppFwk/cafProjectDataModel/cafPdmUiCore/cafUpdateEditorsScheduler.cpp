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
UpdateEditorsScheduler::UpdateEditorsScheduler()
{
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
    m_itemsToUpdate.insert( uiItem );

    startTimer( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UpdateEditorsScheduler::performScheduledUpdates()
{
    for ( auto uiItem : m_itemsToUpdate )
    {
        if ( uiItem )
        {
            uiItem->updateConnectedEditors();
        }
    }

    m_itemsToUpdate.clear();
}

} //namespace caf
