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

#include "RiaDefines.h"

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
    struct Streamline
    {
        Streamline() { animIndex = 0; };

        void                  appendTracerPoint( cvf::Vec3d point );
        void                  appendAbsVelocity( double velocity );
        void                  appendPhase( RiaDefines::PhaseType phase );
        void                  clear();
        cvf::ref<cvf::Part>   getPart();
        cvf::Vec3d            getTracerPoint( size_t index ) const;
        double                getAbsVelocity( size_t index ) const;
        RiaDefines::PhaseType getPhase( size_t index ) const;

        size_t countTracerPoints() const;
        void   setPart( cvf::ref<cvf::Part> part );
        size_t getAnimationIndex() const;
        void   incrementAnimationIndex( size_t increment = 1.0 );
        void   setAnimationIndex( size_t index );

    private:
        std::vector<cvf::Vec3d>            tracerPoints;
        std::vector<double>                absVelocities;
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

    void setAlpha( cvf::ref<cvf::Part> part, float alpha );

private:
    std::list<Streamline>           m_streamlines;
    caf::PdmPointer<RimEclipseView> m_rimReservoirView;
    uint                            m_count;
    size_t                          m_currentT;
    bool                            m_showAsVectors;
};
