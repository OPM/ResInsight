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

#include "RimCheckableNamedObject.h"
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfVector2.h"

class RigMainGrid;
class RigResultAccessor;
class RimEclipseResultCase;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim2dGridProjection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;
public:
    enum ResultAggregationEnum
    {
        RESULTS_TOP_VALUE,
        RESULTS_MEAN_VALUE,
        RESULTS_GEOM_VALUE,
        RESULTS_HARM_VALUE,
        RESULTS_MIN_VALUE,
        RESULTS_MAX_VALUE,
        RESULTS_SUM,
        RESULTS_OIL_COLUMN,
        RESULTS_GAS_COLUMN,
        RESULTS_HC_COLUMN
    };
    typedef caf::AppEnum<ResultAggregationEnum> ResultAggregation;
    typedef std::vector<cvf::ref<cvf::Vec3fArray>> ContourPolygons;

    Rim2dGridProjection();
    ~Rim2dGridProjection() override;

    void                        generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform);
    
    ContourPolygons             generateContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform);
    void                        generateResults();
    double                      maxValue() const;
    double                      minValue() const;
    double                      sampleSpacing() const;
    void                        updateDefaultSampleSpacingFromGrid();
    const std::vector<double>&  aggregatedResults() const;
    bool                        isSummationResult() const;

    double                      value(uint i, uint j) const;

    bool                        hasResultAt(uint i, uint j) const;

    cvf::Vec2ui                 surfaceGridSize() const;
    uint                        vertexCount() const;
    RimRegularLegendConfig*     legendConfig() const;

    size_t                      gridIndex(uint i, uint j) const;
    cvf::Vec2ui                 ijFromGridIndex(size_t gridIndex) const;
    void                        updateLegend();

protected:
    double                                       calculateValue(uint i, uint j) const;

    cvf::BoundingBox                             expandedBoundingBox() const;
    void                                         generateGridMapping();
    void                                         calculateCellRangeVisibility();
    void                                         calculatePropertyFilterVisibility();
    cvf::Vec2d                                   globalPos2d(uint i, uint j) const;
    const std::vector<std::pair<size_t, float>>& cellsAtPos2d(uint i, uint j) const;
    std::vector<double>                          xPositions() const;
    std::vector<double>                          yPositions() const;

    std::vector<std::pair<size_t, float>>        visibleCellsAndWeightMatching2dPoint(const cvf::Vec2d& globalPos2d) const;
    double                                       findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const;

    const RimEclipseResultCase* eclipseCase() const;
    RigMainGrid*                mainGrid() const;
    
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void initAfterRead() override;

protected:
    caf::PdmField<double>                              m_sampleSpacing;
    caf::PdmField<ResultAggregation>                   m_resultAggregation;
    std::map<size_t, cvf::ref<cvf::UByteArray>>        m_cellGridIdxVisibilityMap;

    std::vector<double>                                m_aggregatedResults;
    std::vector<std::vector<std::pair<size_t, float>>> m_projected3dGridIndices;

    cvf::ref<RigResultAccessor>                        m_resultAccessor;
};
