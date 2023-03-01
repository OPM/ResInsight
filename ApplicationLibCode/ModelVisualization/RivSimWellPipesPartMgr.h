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

#include "RigSimWellData.h"

#include "cafPdmPointer.h"

#include "cvfCollection.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <list>

namespace cvf
{
class Part;
class ModelBasicList;
class DrawableGeo;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class RivPipeGeometryGenerator;
class Rim3dView;
class RimSimWellInView;
class RimEclipseView;
class RivWellConnectionFactorGeometryGenerator;
struct RigWellResultPoint;

class RivSimWellPipesPartMgr : public cvf::Object
{
public:
    RivSimWellPipesPartMgr( RimSimWellInView* well );

    ~RivSimWellPipesPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList* model, size_t frameIndex, const caf::DisplayCoordTransform* displayXf );

    void appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                     size_t                            frameIndex,
                                                     const caf::DisplayCoordTransform* displayXf,
                                                     double                            flattenedIntersectionExtentLength,
                                                     int                               branchIndex );

    void updatePipeResultColor( size_t frameIndex );

    std::vector<double> flattenedBranchWellHeadOffsets();

private:
    struct RivPipeBranchData
    {
        std::vector<RigWellResultPoint>    m_cellIds;
        cvf::ref<RivPipeGeometryGenerator> m_pipeGeomGenerator;

        cvf::ref<cvf::Part>        m_surfacePart;
        cvf::ref<cvf::DrawableGeo> m_surfaceDrawable;
        cvf::ref<cvf::DrawableGeo> m_largeSurfaceDrawable;
        cvf::ref<cvf::Part>        m_centerLinePart;
        cvf::ref<cvf::DrawableGeo> m_centerLineDrawable;

        cvf::ref<RivWellConnectionFactorGeometryGenerator> m_connectionFactorGeometryGenerator;
        cvf::ref<cvf::Part>                                m_connectionFactorsPart;

        cvf::Collection<cvf::Part> m_valveParts;
    };

    Rim3dView* viewWithSettings();
    void       buildWellPipeParts( const caf::DisplayCoordTransform* displayXf,
                                   bool                              doFlatten,
                                   double                            flattenedIntersectionExtentLength,
                                   int                               branchIndex,
                                   size_t                            frameIndex );

    void appendVirtualConnectionFactorGeo( const RimEclipseView*             eclipseView,
                                           size_t                            frameIndex,
                                           size_t                            brIdx,
                                           const caf::DisplayCoordTransform* displayXf,
                                           double                            pipeRadius,
                                           RivPipeBranchData&                pbd );

    void appendValvesGeo( const RimEclipseView*             eclipseView,
                          size_t                            frameIndex,
                          size_t                            brIdx,
                          const caf::DisplayCoordTransform* displayXf,
                          double                            pipeRadius,
                          RivPipeBranchData&                pbd );

private:
    caf::PdmPointer<RimSimWellInView> m_simWellInView;

    std::list<RivPipeBranchData> m_wellBranches;

    std::vector<double> m_flattenedBranchWellHeadOffsets;
};
