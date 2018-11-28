/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cvfObject.h"

#include "cafPdmPointer.h"
#include "cvfVector3.h"
#include "cvfColor4.h"

#include <list>
#include <vector>
#include <utility>

namespace cvf
{
    class ModelBasicList;
    class DrawableGeo;
    class Part;
}

class RigWellResultFrame;
class RimEclipseView;
class RimSimWellInView;
class RivPipeGeometryGenerator;

struct RigWellResultPoint;

class RivWellConnectionsPartMgr : public cvf::Object
{
public:
    RivWellConnectionsPartMgr(RimEclipseView* reservoirView, RimSimWellInView* well);
    ~RivWellConnectionsPartMgr() override;

    void appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex);



private:
    cvf::ref<cvf::Part>  createArrowPart(const cvf::Vec3f& startPoint, 
                                         const cvf::Vec3f& endPoint, 
                                         float width, 
                                         bool isProducer, 
                                         const cvf::Color4f& arrowColor, 
                                         bool enableLighting);
    cvf::ref< cvf::DrawableGeo> createArrowGeometry(const cvf::Vec3f& startPoint, 
                                            const cvf::Vec3f& endPoint, 
                                            double width, 
                                            bool useArrowEnd);

private:
    caf::PdmPointer<RimEclipseView>   m_rimReservoirView;
    caf::PdmPointer<RimSimWellInView>   m_rimWell;

    bool                              m_useCurvedArrows;
};
