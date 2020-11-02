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

#include "cvfCollection.h"
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
    struct StreamlineSegment
    {
        /*
        x_ij(t) = a_ij + b_ij * t + c_ij * t^2 + d_ij * t^3
        */

        StreamlineSegment(){};
        StreamlineSegment( cvf::Vec3d startPoint, cvf::Vec3d endPoint, cvf::Vec3d startDirection, cvf::Vec3d endDirection )
            : startPoint( startPoint )
            , endPoint( endPoint )
            , startDirection( startDirection )
            , endDirection( endDirection )
        {
            computeSegments();
        };

        void computeSegments();

        cvf::Vec3f getPointAt( double t ) const;
        cvf::Vec3f getDirectionAt( double t ) const;
        double     getChordLength() const;

        cvf::Vec3f startPoint;
        cvf::Vec3f endPoint;
        cvf::Vec3f startDirection;
        cvf::Vec3f endDirection;
        double     t;

    private:
        cvf::Vec3f a;
        cvf::Vec3f b;
        cvf::Vec3f c;
        cvf::Vec3f d;
    };
    struct StreamlineVisualization
    {
        StreamlineVisualization() { areTValuesComputed = false; };

        void                           computeTValues();
        void                           appendSegment( StreamlineSegment segment );
        size_t                         segmentsSize() const;
        std::vector<StreamlineSegment> getSegments() const;

        cvf::Vec3f getPointAt( double t ) const;
        cvf::Vec3f getDirectionAt( double t ) const;

    private:
        bool                           areTValuesComputed;
        std::vector<StreamlineSegment> segments;
    };

private:
    cvf::ref<cvf::Part> createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                    const StreamlineVisualization&       streamlineVisualization,
                                    const double                         t ) const;

    std::array<cvf::Vec3f, 7> createArrowVertices( const cvf::Vec3f anchorPoint, const cvf::Vec3f direction ) const;
    std::array<uint, 2>       createArrowShaftIndices( uint startIndex ) const;
    std::array<uint, 6>       createArrowHeadIndices( uint startIndex ) const;
    std::array<uint, 2>       createLineIndices( uint startIndex ) const;
    void                      createExampleStreamline( std::vector<StreamlineVisualization>& streamlineVisualizations );

private:
    caf::PdmPointer<RimEclipseView> m_rimReservoirView;
    cvf::Collection<cvf::Part>      m_parts;
    uint                            m_count;
    size_t                          m_numSegments;
    size_t                          m_currentT;
};
