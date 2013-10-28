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

#pragma once

#include <QString>

class RimDefines
{

public:
    enum ResultCatType
    {
        DYNAMIC_NATIVE,
        STATIC_NATIVE,
        GENERATED,
        INPUT_PROPERTY,
        REMOVED
    };

    enum PorosityModelType
    {
        MATRIX_MODEL,
        FRACTURE_MODEL
    };

    static QString undefinedResultName() { return "None"; }
    static QString undefinedGridFaultName() { return "Undefined grid faults"; }
    static QString combinedTransmissibilityResultName() { return "TRANSXYZ"; }


    // Mock model text identifiers
    static QString mockModelBasic()                 { return "Result Mock Debug Model Simple"; }
    static QString mockModelBasicWithResults()      { return "Result Mock Debug Model With Results"; }
    static QString mockModelLargeWithResults()      { return "Result Mock Debug Model Large With Results"; }
    static QString mockModelBasicInputCase()        { return "Input Mock Debug Model Simple"; }

};

