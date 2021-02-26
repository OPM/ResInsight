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
#include "cvfPart.h"
#include "cvfVector3.h"

#include <vector>

namespace cvf
{
class ModelBasicList;
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
        StreamlineSegment( cvf::Vec3d startPoint,
                           cvf::Vec3d endPoint,
                           cvf::Vec3d startDirection,
                           cvf::Vec3d endDirection,
                           double     startVelocity,
                           double     endVelocity )
            : startPoint( startPoint )
            , endPoint( endPoint )
            , startDirection( startDirection )
            , endDirection( endDirection )
            , startVelocity( startVelocity )
            , endVelocity( endVelocity )
        {
            computeSegments();
        };

        void computeSegments();

        cvf::Vec3d getPointAt( double localT ) const;
        cvf::Vec3d getDirectionAt( double localT ) const;
        double     getVelocityAt( double localT ) const;
        double     getChordLength() const;

        cvf::Vec3d startPoint;
        cvf::Vec3d endPoint;
        cvf::Vec3d startDirection;
        cvf::Vec3d endDirection;
        double     globalTStart;
        double     globalTEnd;
        double     startVelocity;
        double     endVelocity;

    private:
        cvf::Vec3d a;
        cvf::Vec3d b;
        cvf::Vec3d c;
        cvf::Vec3d d;
    };
    struct StreamlineVisualization
    {
        StreamlineVisualization()
        {
            areTValuesComputed      = false;
            currentAnimationGlobalT = 0.0;
        };

        void                                                computeTValues();
        void                                                appendSegment( StreamlineSegment segment );
        void                                                prependSegment( StreamlineSegment segment );
        void                                                appendPart( cvf::ref<cvf::Part> part, double globalT );
        size_t                                              segmentsSize() const;
        std::list<RivStreamlinesPartMgr::StreamlineSegment> getSegments();
        void                                                clear();
        void                                                updateAnimationGlobalT( double timeMs );
        double                                              getApproximatedTotalLength();

        cvf::Vec3d                 getPointAt( double globalT ) const;
        cvf::Vec3d                 getDirectionAt( double globalT ) const;
        double                     getVelocityAt( double globalT ) const;
        cvf::Collection<cvf::Part> getParts();
        cvf::ref<cvf::Part>        getPartAtGlobalT( double globalT ) const;

        double currentAnimationGlobalT;

    private:
        bool                         areTValuesComputed;
        double                       approximatedTotalLength;
        std::list<StreamlineSegment> segments;
        cvf::Collection<cvf::Part>   parts;
        std::vector<double>          partTValues;
    };

    struct Streamline
    {
        Streamline() { animIndex = 0; };

        void                  appendTracerPoint( cvf::Vec3d point );
        void                  appendAbsVelocity( double velocity );
        void                  appendDirection( cvf::Vec3d direction );
        void                  appendPhase( RiaDefines::PhaseType phase );
        void                  clear();
        cvf::ref<cvf::Part>   getPart();
        cvf::Vec3d            getTracerPoint( size_t index ) const;
        double                getAbsVelocity( size_t index ) const;
        cvf::Vec3d            getDirection( size_t index ) const;
        RiaDefines::PhaseType getPhase( size_t index ) const;

        size_t countTracerPoints() const;
        void   setPart( cvf::ref<cvf::Part> part );
        size_t getAnimationIndex() const;
        void   incrementAnimationIndex( size_t increment = 1.0 );
        void   setAnimationIndex( size_t index );

    private:
        std::vector<cvf::Vec3d>            tracerPoints;
        std::vector<double>                absVelocities;
        std::vector<cvf::Vec3d>            directions;
        std::vector<RiaDefines::PhaseType> dominantPhases;

        cvf::ref<cvf::Part> part;
        size_t              animIndex;
    };

private:
    cvf::ref<cvf::Part> createPart( const RimStreamlineInViewCollection& streamlineCollection,
                                    Streamline&                          streamlineVisualization );

    void createResultColorTextureCoords( cvf::Vec2fArray*         textureCoords,
                                         const Streamline&        streamline,
                                         const cvf::ScalarMapper* mapper );

    cvf::ref<cvf::Part> createVectorPart( const RimStreamlineInViewCollection& streamlineCollection, Streamline& segment );

    std::array<cvf::Vec3f, 7> createArrowVertices( const cvf::Vec3f anchorPoint, const cvf::Vec3f direction ) const;
    std::array<uint, 2>       createArrowShaftIndices( uint startIndex ) const;
    std::array<uint, 6>       createArrowHeadIndices( uint startIndex ) const;
    void                      setAlpha( cvf::ref<cvf::Part> part, float alpha );

private:
    std::list<Streamline>           m_streamlines;
    caf::PdmPointer<RimEclipseView> m_rimReservoirView;
    uint                            m_count;
    size_t                          m_currentT;
    bool                            m_showAsVectors;
};
