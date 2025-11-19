/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RiaModelExportDefines.h"

#include "cafVecIjk.h"
#include "cvfArray.h"

#include <utility>
#include <vector>

class RimEclipseView;
class RigEclipseCaseData;
class RigSimWellData;

namespace RimEclipseViewTools
{

std::vector<const RigSimWellData*> getVisibleSimulationWells( RimEclipseView* view );
std::pair<caf::VecIjk0, caf::VecIjk0> computeVisibleWellCells( RimEclipseView* view, RigEclipseCaseData* caseData, int visibleWellsPadding );
std::pair<caf::VecIjk0, caf::VecIjk0> getVisibleCellRange( RimEclipseView* view, const cvf::UByteArray& cellVisibillity );
cvf::ref<cvf::UByteArray>             createVisibilityBasedOnBoxSelection( RimEclipseView*                         view,
                                                                           RiaModelExportDefines::GridBoxSelection gridBoxType,
                                                                           caf::VecIjk0                            minIjk,
                                                                           caf::VecIjk0                            maxIjk,
                                                                           int                                     wellPadding );

} // namespace RimEclipseViewTools
