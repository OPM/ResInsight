/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RivAnnotationLabelPositionStrategy.h"

#include "cafAppEnum.h"

namespace caf
{

template <>
void caf::AppEnum<RivAnnotationLabelPositionStrategy>::setUp()
{
    addItem( RivAnnotationLabelPositionStrategy::LEFT, "LEFT", "Left" );
    addItem( RivAnnotationLabelPositionStrategy::RIGHT, "RIGHT", "Right" );
    addItem( RivAnnotationLabelPositionStrategy::LEFT_AND_RIGHT, "LEFT_AND_RIGHT", "Left and Right" );
    addItem( RivAnnotationLabelPositionStrategy::COUNT_HINT, "COUNT_HINT", "Count Hint" );
    addItem( RivAnnotationLabelPositionStrategy::ALL, "All", "All" );
    addItem( RivAnnotationLabelPositionStrategy::NONE, "None", "Disabled" );
    // RivAnnotationLabelPositionStrategy::UNDEFINED is not included, as this enum value is not supposed to be displayed in GUI
    setDefault( RivAnnotationLabelPositionStrategy::RIGHT );
}

} // End namespace caf
