/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RimDefines.h"
#include "cafAppEnum.h"


namespace caf
{
    template<>
    void caf::AppEnum< RimDefines::ResultCatType >::setUp()
    {
        addItem(RimDefines::DYNAMIC_NATIVE, "DYNAMIC_NATIVE",   "Dynamic");
        addItem(RimDefines::STATIC_NATIVE,  "STATIC_NATIVE",    "Static");
        addItem(RimDefines::GENERATED,      "GENERATED",        "Generated");
        addItem(RimDefines::INPUT_PROPERTY, "INPUT_PROPERTY",   "Input Property");

        setDefault(RimDefines::DYNAMIC_NATIVE);
    }

    template<>
    void caf::AppEnum< RimDefines::PorosityModelType >::setUp()
    {
        addItem(RimDefines::MATRIX_MODEL,   "MATRIX_MODEL",     "Matrix");
        addItem(RimDefines::FRACTURE_MODEL, "FRACTURE_MODEL",   "Fracture");

        setDefault(RimDefines::MATRIX_MODEL);
    }

}

