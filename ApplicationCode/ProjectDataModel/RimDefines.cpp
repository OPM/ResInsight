/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

    template<>
    void caf::AppEnum< RimDefines::DepthUnitType >::setUp()
    {
        addItem(RimDefines::UNIT_METER,  "UNIT_METER",   "Meter");
        addItem(RimDefines::UNIT_FEET,   "UNIT_FEET",    "Feet");

        setDefault(RimDefines::UNIT_METER);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimDefines::isPerCellFaceResult(const QString& resultName)
{
    if (resultName.compare(RimDefines::combinedTransmissibilityResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RimDefines::combinedMultResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RimDefines::ternarySaturationResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RimDefines::combinedRiTranResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RimDefines::combinedRiMultResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }
    else if (resultName.compare(RimDefines::combinedRiAreaNormTranResultName(), Qt::CaseInsensitive) == 0)
    {
        return true;
    }

    return false;
}
