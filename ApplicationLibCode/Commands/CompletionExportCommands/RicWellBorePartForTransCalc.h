/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "cvfVector3.h"

#include "cafPdmPointer.h"

#include <QString>

namespace caf
{
class PdmObject;
}

//==================================================================================================
///
//==================================================================================================
struct WellBorePartForTransCalc
{
    WellBorePartForTransCalc( cvf::Vec3d lengthsInCell, double wellRadius, double skinFactor, bool isMainBore, const QString& metaData )
        : lengthsInCell( lengthsInCell )
        , wellRadius( wellRadius )
        , skinFactor( skinFactor )
        , isMainBore( isMainBore )
        , metaData( metaData )
        , intersectionWithWellMeasuredDepth( HUGE_VAL )
        , lateralIndex( cvf::UNDEFINED_SIZE_T )
    {
    }

    cvf::Vec3d lengthsInCell;
    double     wellRadius;
    double     skinFactor;
    QString    metaData;
    bool       isMainBore;

    double intersectionWithWellMeasuredDepth;
    size_t lateralIndex;

    void setSourcePdmObject( const caf::PdmObject* sourcePdmObj ) { sourcePdmObject = const_cast<caf::PdmObject*>( sourcePdmObj ); }
    caf::PdmPointer<caf::PdmObject> sourcePdmObject;
};
