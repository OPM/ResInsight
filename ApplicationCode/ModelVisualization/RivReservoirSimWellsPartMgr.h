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

#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"

namespace cvf
{
class Transform;
class ModelBasicList;
} // namespace cvf

class RimEclipseView;
class RivSimWellPipesPartMgr;
class RivWellHeadPartMgr;
class RivWellSpheresPartMgr;
class RivWellConnectionsPartMgr;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RivReservoirSimWellsPartMgr : public cvf::Object
{
public:
    explicit RivReservoirSimWellsPartMgr(RimEclipseView* reservoirView);
    ~RivReservoirSimWellsPartMgr() override;

    void clearGeometryCache();
    void scheduleGeometryRegen();

    void setScaleTransform(cvf::Transform* scaleTransform);

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);
    void updatePipeResultColor(size_t frameIndex);

private:
    caf::PdmPointer<RimEclipseView> m_reservoirView;
    cvf::ref<cvf::Transform>        m_scaleTransform;

    cvf::Collection<RivSimWellPipesPartMgr>    m_wellPipesPartMgrs;
    cvf::Collection<RivWellHeadPartMgr>        m_wellHeadPartMgrs;
    cvf::Collection<RivWellSpheresPartMgr>     m_wellSpheresPartMgrs;
    cvf::Collection<RivWellConnectionsPartMgr> m_wellConnPartMgrs;
};
