/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfArray.h"
#include "cvfCollection.h"

#include "RivFemPartPartMgr.h"

namespace cvf
{
    class ModelBasicList;
    class Transform;
}

class RimGeoMechCellColors;
class RigGeoMechCaseData;
class RimGeoMechView;

//==================================================================================================
///
/// Class to handle visualization structures that embodies a complete Geo-mech reservoir at a specific 
/// time step. 
///
//==================================================================================================

class RivGeoMechPartMgr: public cvf::Object
{
public:
    RivGeoMechPartMgr();
    ~RivGeoMechPartMgr();

    int    initializedFemPartCount() { return static_cast<int>(m_femPartPartMgrs.size());}
    void   clearAndSetReservoir(const RigGeoMechCaseData* geoMechCase);
    void   setTransform(cvf::Transform* scaleTransform);
    void   setCellVisibility(size_t partIndex, cvf::UByteArray* cellVisibilities );

    cvf::ref<cvf::UByteArray>  
           cellVisibility(size_t partIndex);

    void   updateCellColor(cvf::Color4f color);
    void   updateCellResultColor(size_t timeStepIndex, RimGeoMechCellColors* cellResultColors);

    void   appendGridPartsToModel(cvf::ModelBasicList* model, const std::vector<size_t>& partIndices);
    void   appendGridPartsToModel(cvf::ModelBasicList* model);

    const cvf::Collection<RivFemPartPartMgr> femPartMgrs() const;
private:

    cvf::Collection<RivFemPartPartMgr>  m_femPartPartMgrs;
};
