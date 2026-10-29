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

#pragma once

#include "cafScheduler.h"

#include <mutex>
#include <set>

namespace caf
{

class PdmUiItem;

class UpdateEditorsScheduler : public Scheduler
{
public:
    UpdateEditorsScheduler();

    static UpdateEditorsScheduler* instance();

    void scheduleUpdateConnectedEditors( const PdmUiItem* uiItem );
    void performScheduledUpdates() override;

    // Called from the PdmUiItem destructor. Safe to call also after the scheduler instance is destroyed, and from
    // worker threads deleting PDM objects.
    static void unscheduleDeletedItem( const PdmUiItem* uiItem );

private:
    ~UpdateEditorsScheduler() override;

    static bool sm_isAlive;

    std::mutex                 m_mutex;
    std::set<const PdmUiItem*> m_itemsToUpdate;
    std::set<const PdmUiItem*> m_itemsBeingUpdated;
};

} // namespace caf
