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
class RimContourMapView;
class RimEclipseResultCase;
class RimEclipseResultDefinition;

//==================================================================================================
///  
///  
//==================================================================================================
class RimContourMapProjection : public RimCheckableNamedObject
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
        RESULTS_VOLUME_SUM,
        RESULTS_SUM,
        RESULTS_OIL_COLUMN,
        RESULTS_GAS_COLUMN,
        RESULTS_HC_COLUMN
    };
    typedef caf::AppEnum<ResultAggregationEnum> ResultAggregation;
    typedef std::vector<cvf::ref<cvf::Vec3fArray>> ContourPolygons;

    RimContourMapProjection();
    ~RimContourMapProjection() override;

    void                        generateVertices(cvf::Vec3fArray* vertices, const caf::DisplayCoordTransform* displayCoordTransform);    
    ContourPolygons             generateContourPolygons(const caf::DisplayCoordTransform* displayCoordTransform);
    cvf::ref<cvf::Vec3fArray>   generatePickPointPolygon(const caf::DisplayCoordTransform* displayCoordTransform);
    void                        generateResults();

    ResultAggregation           resultAggregation() const;
    double                      sampleSpacing() const;
    double                      sampleSpacingFactor() const;
    bool                        showContourLines() const;

    QString                     resultAggregationText() const;
    QString                     resultDescriptionText() const;
    QString                     weightingParameter() const;

    double                      maxValue() const;
    double                      minValue() const;
    double                      meanValue() const;
    double                      sumAllValues() const;

    cvf::Vec2ui                 numberOfElementsIJ() const;
    cvf::Vec2ui                 numberOfVerticesIJ() const;

    bool                        isColumnResult() const;

    double                      valueAtVertex(uint i, uint j) const;
    bool                        hasResultAtVertex(uint i, uint j) const;

    RimRegularLegendConfig*     legendConfig() const;
    void                        updateLegend();

    uint                        numberOfCells() const;
    uint                        numberOfValidCells() const;
    size_t                      numberOfVertices() const;

    void                        updatedWeightingResult();

    bool                        checkForMapIntersection(const cvf::Vec3d& localPoint3d, cvf::Vec2d* contourMapPoint, cvf::Vec2ui* contourMapCell, double* valueAtPoint) const;
    void                        setPickPoint(cvf::Vec2d pickedPoint);

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    void initAfterRead() override;

private:
    typedef std::pair<size_t, double> CellIndexAndResult;

private:
    void                            generateGridMapping();

    double                          valueInCell(uint i, uint j) const;
    bool                            hasResultInCell(uint i, uint j) const;

    double                          calculateValueInCell(uint i, uint j) const;
    double                          calculateValueAtVertex(uint i, uint j) const;


    std::vector<CellIndexAndResult> cellsAtIJ(uint i, uint j) const;

    std::vector<CellIndexAndResult> visibleCellsAndOverlapVolumeFrom2dPoint(const cvf::Vec2d& globalPos2d, const std::vector<double>* weightingResultValues = nullptr) const;
    std::vector<CellIndexAndResult> visibleCellsAndLengthInCellFrom2dPoint(const cvf::Vec2d& globalPos2d, const std::vector<double>* weightingResultValues = nullptr) const;
    double                          findColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const;

    bool                            isMeanResult() const;
    bool                            isSummationResult() const;
    bool                            isStraightSummationResult() const;
    static bool                     isStraightSummationResult(ResultAggregationEnum aggregationType);

    size_t                          cellIndexFromIJ(uint i, uint j) const;
    size_t                          vertexIndexFromIJ(uint i, uint j) const;

    cvf::Vec2ui                     ijFromVertexIndex(size_t gridIndex) const;
    cvf::Vec2ui                     ijFromCellIndex(size_t mapIndex) const;
    cvf::Vec2ui                     ijFromLocalPos(const cvf::Vec2d& localPos2d) const;
    cvf::Vec2d                      globalCellCenterPosition(uint i, uint j) const;

    std::vector<double>             xVertexPositions() const;
    std::vector<double>             yVertexPositions() const;

    bool                            getLegendRangeFrom3dGrid() const;
    void                            updateGridInformation();
    cvf::Vec2ui                     calculateMapSize() const;

    RimEclipseResultCase*           eclipseCase() const;
    RimContourMapView*              view() const;

protected:
    caf::PdmField<double>                               m_relativeSampleSpacing;
    caf::PdmField<ResultAggregation>                    m_resultAggregation;
    caf::PdmField<bool>                                 m_showContourLines;
    caf::PdmField<bool>                                 m_weightByParameter;
    caf::PdmChildField<RimEclipseResultDefinition*>     m_weightingResult;
    cvf::ref<cvf::UByteArray>                           m_cellGridIdxVisibility;

    std::vector<double>                                 m_aggregatedResults;
    std::vector<double>                                 m_aggregatedVertexResults;

    std::vector<std::vector<std::pair<size_t, double>>> m_projected3dGridIndices;

    cvf::ref<RigResultAccessor>                         m_resultAccessor;

    cvf::Vec2d                                          m_pickPoint;

    caf::PdmPointer<RimEclipseResultCase>               m_eclipseCase;
    cvf::ref<RigMainGrid>                               m_mainGrid;
    cvf::Vec2ui                                         m_mapSize;
    cvf::BoundingBox                                    m_fullBoundingBox;
    double                                              m_sampleSpacing;
};
