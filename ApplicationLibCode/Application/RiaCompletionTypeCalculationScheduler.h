/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmPointer.h"
#include "cafScheduler.h"

#include <vector>

class RimEclipseCase;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaCompletionTypeCalculationScheduler : public caf::Scheduler
{
    Q_OBJECT;

public:
    static RiaCompletionTypeCalculationScheduler* instance();
    void                                          scheduleRecalculateCompletionTypeAndRedrawAllViews();
    void                                          clearCompletionTypeResultsInAllCases();

    void scheduleRecalculateCompletionTypeAndRedrawAllViews( const std::vector<RimEclipseCase*>& eclipseCases );
    void clearCompletionTypeResults( const std::vector<RimEclipseCase*>& eclipseCases );

    void performScheduledUpdates() override;

private:
    RiaCompletionTypeCalculationScheduler();
    ~RiaCompletionTypeCalculationScheduler() override;

    RiaCompletionTypeCalculationScheduler( const RiaCompletionTypeCalculationScheduler& o ) = delete;
    void operator=( const RiaCompletionTypeCalculationScheduler& o )                        = delete;

private:
    std::vector<caf::PdmPointer<RimEclipseCase>> m_eclipseCasesToRecalculate;
};
