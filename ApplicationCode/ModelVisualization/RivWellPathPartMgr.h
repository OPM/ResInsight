/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafPdmPointer.h"
#include "cvfBoundingBox.h"
#include "cvfCollection.h"

namespace cvf
{
class Part;
class ModelBasicList;
class Transform;
class Effect;
class DrawableGeo;
class ScalarMapper;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class RivPipeGeometryGenerator;
class RimProject;
class RimWellPath;
class RivFishbonesSubsPartMgr;
class RimPerforationInterval;
class RimWellPathCollection;
class Rim3dView;
class Riv3dWellLogPlanePartMgr;
class RivWellConnectionFactorPartMgr;
class RimWellMeasurementInView;

class QDateTime;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RivWellPathPartMgr : public cvf::Object
{
public:
    explicit RivWellPathPartMgr( RimWellPath* wellPath, Rim3dView* view );
    ~RivWellPathPartMgr() override;

    void appendStaticGeometryPartsToModel( cvf::ModelBasicList*              model,
                                           const caf::DisplayCoordTransform* displayCoordTransform,
                                           double                            characteristicCellSize,
                                           const cvf::BoundingBox&           wellPathClipBoundingBox );

    void appendFlattenedStaticGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                                    double                            characteristicCellSize,
                                                    const cvf::BoundingBox&           wellPathClipBoundingBox );

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            size_t                            timeStepIndex,
                                            const caf::DisplayCoordTransform* displayCoordTransform,
                                            double                            characteristicCellSize,
                                            const cvf::BoundingBox&           wellPathClipBoundingBox );

    void appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                     size_t                            timeStepIndex,
                                                     const caf::DisplayCoordTransform* displayCoordTransform,
                                                     double                            characteristicCellSize,
                                                     const cvf::BoundingBox&           wellPathClipBoundingBox );

    void appendStaticFracturePartsToModel( cvf::ModelBasicList* model, const cvf::BoundingBox& wellPathClipBoundingBox );

private:
    void appendFishboneSubsPartsToModel( cvf::ModelBasicList*              model,
                                         const caf::DisplayCoordTransform* displayCoordTransform,
                                         double                            characteristicCellSize );

    void appendWellPathAttributesToModel( cvf::ModelBasicList*              model,
                                          const caf::DisplayCoordTransform* displayCoordTransform,
                                          double                            characteristicCellSize );

    void appendWellMeasurementsToModel( cvf::ModelBasicList*              model,
                                        const caf::DisplayCoordTransform* displayCoordTransform,
                                        double                            characteristicCellSize );

    void appendImportedFishbonesToModel( cvf::ModelBasicList*              model,
                                         const caf::DisplayCoordTransform* displayCoordTransform,
                                         double                            characteristicCellSize );

    void appendPerforationsToModel( cvf::ModelBasicList*              model,
                                    size_t                            timeStepIndex,
                                    const caf::DisplayCoordTransform* displayCoordTransform,
                                    double                            characteristicCellSize,
                                    bool                              doFlatten );

    void appendPerforationValvesToModel( cvf::ModelBasicList*              model,
                                         RimPerforationInterval*           perforation,
                                         double                            wellPathRadius,
                                         const caf::DisplayCoordTransform* displayCoordTransform,
                                         RivPipeGeometryGenerator&         geoGenerator );

    void appendVirtualTransmissibilitiesToModel( cvf::ModelBasicList*              model,
                                                 size_t                            timeStepIndex,
                                                 const caf::DisplayCoordTransform* displayCoordTransform,
                                                 double                            characteristicCellSize );

    void buildWellPathParts( const caf::DisplayCoordTransform* displayCoordTransform,
                             double                            characteristicCellSize,
                             const cvf::BoundingBox&           wellPathClipBoundingBox,
                             bool                              doFlatten );

    void                          clearAllBranchData();
    inline RimWellPathCollection* wellPathCollection() const;
    inline double wellPathRadius( double characteristicCellSize, RimWellPathCollection* wellPathCollection );
    double        wellMeasurementRadius( double                          characteristicCellSize,
                                         const RimWellPathCollection*    wellPathCollection,
                                         const RimWellMeasurementInView* wellMeasurementInView );

    bool isWellPathWithinBoundingBox( const cvf::BoundingBox& wellPathClipBoundingBox ) const;

    static cvf::Color3f mapWellMeasurementToColor( const QString& measurementKind, double value );

private:
    caf::PdmPointer<RimWellPath> m_rimWellPath;
    caf::PdmPointer<Rim3dView>   m_rimView;

    cvf::ref<RivPipeGeometryGenerator> m_pipeGeomGenerator;
    cvf::ref<cvf::Part>                m_surfacePart;
    cvf::ref<cvf::DrawableGeo>         m_surfaceDrawable;
    cvf::ref<cvf::Part>                m_centerLinePart;
    cvf::ref<cvf::DrawableGeo>         m_centerLineDrawable;
    cvf::ref<cvf::Part>                m_wellLabelPart;

    cvf::ref<Riv3dWellLogPlanePartMgr>       m_3dWellLogPlanePartMgr;
    cvf::ref<RivWellConnectionFactorPartMgr> m_wellConnectionFactorPartMgr;
};
