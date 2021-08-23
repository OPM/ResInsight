/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
#include "cvfColor4.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmPointer.h"
#include "cvfCollection.h"

#include <QString>

#include <list>
#include <vector>

namespace cvf
{
class ModelBasicList;
class Transform;
class Part;
class ScalarMapper;
class DrawableGeo;
} // namespace cvf

class RigFemPart;
class RigFemResultAddress;
class RigGeoMechCaseData;
class RigMainGrid;
class RigResultAccessor;
class Rim3dView;
class RimCellEdgeColors;
class RimEclipseCellColors;
class RimExtrudedCurveIntersection;
class RivTernaryScalarMapper;
class RivExtrudedCurveIntersectionGeometryGenerator;
class RivIntersectionHexGridInterface;
class RivIntersectionVertexWeights;
class RivPipeGeometryGenerator;
class RivIntersectionGeometryGeneratorIF;

//==================================================================================================
///
///
//==================================================================================================

class RivExtrudedCurveIntersectionPartMgr : public cvf::Object
{
public:
    explicit RivExtrudedCurveIntersectionPartMgr( RimExtrudedCurveIntersection* rimIntersection, bool isFlattened = false );

    void applySingleColorEffect();
    void updateCellResultColor( size_t                        timeStepIndex,
                                const cvf::ScalarMapper*      explicitScalarColorMapper,
                                const RivTernaryScalarMapper* explicitTernaryColorMapper );

    void appendIntersectionFacesToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendMeshLinePartsToModel( cvf::ModelBasicList* model, cvf::Transform* scaleTransform );
    void appendPolylinePartsToModel( Rim3dView& view, cvf::ModelBasicList* model, cvf::Transform* scaleTransform );

    cvf::Mat4d unflattenTransformMatrix( const cvf::Vec3d& intersectionPointFlat ) const;

    const RivIntersectionGeometryGeneratorIF* intersectionGeometryGenerator() const;

private:
    void generatePartGeometry();
    void createFaultLabelParts( const std::vector<std::pair<QString, cvf::Vec3d>>& labelAndAnchors );
    void createPolyLineParts( bool useBufferObjects );
    void createExtrusionDirParts( bool useBufferObjects );
    void createAnnotationSurfaceParts( bool useBufferObjects );

private:
    caf::PdmPointer<RimExtrudedCurveIntersection> m_rimIntersection;

    cvf::ref<RivExtrudedCurveIntersectionGeometryGenerator> m_intersectionGenerator;

    cvf::ref<cvf::Part> m_intersectionFaces;
    cvf::ref<cvf::Part> m_intersectionGridLines;
    cvf::ref<cvf::Part> m_intersectionFaultGridLines;
    cvf::ref<cvf::Part> m_faultMeshLabels;
    cvf::ref<cvf::Part> m_faultMeshLabelLines;
    cvf::ref<cvf::Part> m_highlightLineAlongPolyline;
    cvf::ref<cvf::Part> m_highlightPointsForPolyline;
    cvf::ref<cvf::Part> m_highlightLineAlongExtrusionDir;
    cvf::ref<cvf::Part> m_highlightPointsForExtrusionDir;

    cvf::Collection<cvf::Part> m_annotatedIntersectionSurfaces;

    cvf::ref<cvf::Vec2fArray> m_intersectionFacesTextureCoords;

    struct RivPipeBranchData
    {
        cvf::ref<RivPipeGeometryGenerator> m_pipeGeomGenerator;
        cvf::ref<cvf::Part>                m_surfacePart;
        cvf::ref<cvf::Part>                m_centerLinePart;
    };
    std::list<RivPipeBranchData> m_wellBranches;

    bool m_isFlattened;
};
