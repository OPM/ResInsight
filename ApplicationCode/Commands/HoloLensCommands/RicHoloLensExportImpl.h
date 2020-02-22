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

#include "RivCellSetEnum.h"

#include "VdeExportPart.h"

#include "cvfCollection.h"

class QString;
class RimGridView;

namespace cvf
{
class Part;
} // namespace cvf

//==================================================================================================
///
//==================================================================================================
class RicHoloLensExportImpl
{
public:
    static std::vector<VdeExportPart>                      partsForExport( const RimGridView& view );
    static std::vector<std::pair<cvf::Vec3f, cvf::String>> labelsForExport( const RimGridView& view );

private:
    static void    appendTextureImage( VdeExportPart& exportPart, cvf::Part* part );
    static QString gridCellSetTypeText( RivCellSetEnum cellSetType );

    static bool isGrid( const cvf::Part* part );
    static bool isPipe( const cvf::Part* part );
    static bool isMeshLines( const cvf::Part* part );
};
