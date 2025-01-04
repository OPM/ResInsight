/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

class RimPlotCollection;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPlotCollectionScheduler : public caf::Scheduler
{
public:
    RiaPlotCollectionScheduler();
    ~RiaPlotCollectionScheduler() override;

    static RiaPlotCollectionScheduler* instance();

    void schedulePlotCollectionUpdate( const std::vector<RimPlotCollection*> plotCollections );

    void performScheduledUpdates() override;

private:
    std::vector<RimPlotCollection*> m_plotCollectionsToUpdate;
};
