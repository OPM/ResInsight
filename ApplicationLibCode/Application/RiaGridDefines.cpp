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

#include "RiaGridDefines.h"

#include "cafAppEnum.h"

namespace caf
{
template <>
void caf::AppEnum<RiaGridDefines::RadialGridMode>::setUp()
{
    addItem( RiaGridDefines::RadialGridMode::CARTESIAN, "CARTESIAN", "Show Cells as Cartesian Hex Elements" );
    addItem( RiaGridDefines::RadialGridMode::CYLINDRICAL, "CYLINDRICAL", "Show Cells as Cylinder Segments" );
    setDefault( RiaGridDefines::RadialGridMode::CYLINDRICAL );
}

} // namespace caf
