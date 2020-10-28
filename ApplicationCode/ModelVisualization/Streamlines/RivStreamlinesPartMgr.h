/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cvfObject.h"

#include "cafPdmPointer.h"
#include "cafSignal.h"

#include "cvfVector3.h"

#include <vector>

namespace cvf
{
class ModelBasicList;
class Part;
class ScalarMapper;
class ScalarMapperDiscreteLinear;
} // namespace cvf

class RimEclipseView;
class RimStreamlineInViewCollection;
class Rim3dView;

class RivStreamlinesPartMgr : public cvf::Object, public caf::SignalObserver
{
public:
    RivStreamlinesPartMgr( RimEclipseView* reservoirView );
    ~RivStreamlinesPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t timeStepIndex );
    void updateAnimation();

private:
    struct StreamlineVisualization
    {
        StreamlineVisualization(){};

        std::vector<cvf::Vec3d> tracerPoints;
    };

private:
    cvf::ref<cvf::Part> createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                    const StreamlineVisualization&       streamlineVisualizations ) const;

private:
    caf::PdmPointer<RimEclipseView>  m_rimReservoirView;
    std::vector<cvf::ref<cvf::Part>> m_parts;
    uint                             m_count;
};
