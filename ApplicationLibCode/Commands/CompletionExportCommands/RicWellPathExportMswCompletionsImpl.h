/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

class RicExportCompletionDataSettingsUi;
class RimWellPath;

#include <vector>

//--------------------------------------------------------------------------------------------------
/// Most of the functionality in this class has been moved to RicWellPathExportMswTableData
/// This class is kept for compatibility with existing code that calls exportWellSegmentsForAllCompletions
//--------------------------------------------------------------------------------------------------
class RicWellPathExportMswCompletionsImpl
{
public:
    static void exportWellSegmentsForAllCompletions( const RicExportCompletionDataSettingsUi& exportSettings,
                                                     const std::vector<RimWellPath*>&         wellPaths );
};
