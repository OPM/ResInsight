/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Equinor ASA
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

#include "cafContourLines.h"
#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

#include "cvfArray.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfString.h"
#include "cvfVector2.h"

class RimGridView;
class RimRegularLegendConfig;

//==================================================================================================
///
///
//==================================================================================================
class RimContourMapProjection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    using CellIndexAndResult = std::pair<size_t, double>;

    struct ContourPolygon
    {
        std::vector<cvf::Vec3d> vertices;
        double                  value;
        double                  area;
        cvf::BoundingBox        bbox;
    };

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
    using ResultAggregation = caf::AppEnum<ResultAggregationEnum>;
    using ContourPolygons   = std::vector<ContourPolygon>;

    RimContourMapProjection();
    ~RimContourMapProjection() override;

    void generateResultsIfNecessary( int timeStep );
    void generateGeometryIfNecessary();
    void clearGeometry();

    std::vector<cvf::Vec3d> generatePickPointPolygon();

    const std::vector<ContourPolygons>& contourPolygons() const;
    const std::vector<cvf::Vec4d>&      trianglesWithVertexValues();

    virtual double sampleSpacing() const = 0;

    double sampleSpacingFactor() const;
    void   setSampleSpacingFactor( double spacingFactor );
    bool   showContourLines() const;
    bool   showContourLabels() const;

    QString resultAggregationText() const;

    QString caseName() const;
    QString currentTimeStepName() const;

    double maxValue() const;
    double minValue() const;

    double meanValue() const;
    double sumAllValues() const;

    cvf::Vec2ui numberOfElementsIJ() const;
    cvf::Vec2ui numberOfVerticesIJ() const;

    bool isColumnResult() const;

    double valueAtVertex( uint i, uint j ) const;

    uint   numberOfCells() const;
    uint   numberOfValidCells() const;
    size_t numberOfVertices() const;

    bool       checkForMapIntersection( const cvf::Vec3d& domainPoint3d, cvf::Vec2d* contourMapPoint, double* valueAtPoint ) const;
    void       setPickPoint( cvf::Vec2d globalPickPoint );
    cvf::Vec3d origin3d() const;

    std::vector<double> xVertexPositions() const;
    std::vector<double> yVertexPositions() const;

    // Pure-virtual public methods which should be overridden by Eclipse and Geo-mechanical contour map implementations
    virtual QString                 resultDescriptionText() const = 0;
    virtual RimRegularLegendConfig* legendConfig() const          = 0;
    virtual void                    updateLegend()                = 0;

protected:
    // Protected virtual methods to be overridden by Eclipse and Geo-mechanical contour map implementations
    virtual void                updateGridInformation()                                                            = 0;
    virtual std::vector<double> retrieveParameterWeights()                                                         = 0;
    virtual std::vector<double> generateResults( int timeStep )                                                    = 0;
    virtual bool                resultVariableChanged() const                                                      = 0;
    virtual void                clearResultVariable()                                                              = 0;
    virtual RimGridView*        baseView() const                                                                   = 0;
    virtual size_t              kLayer( size_t globalCellIdx ) const                                               = 0;
    virtual size_t              kLayers() const                                                                    = 0;
    virtual std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& bbox ) const                        = 0;
    virtual double              calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const = 0;
    virtual double calculateRayLengthInCell( size_t globalCellIdx, const cvf::Vec3d& highestPoint, const cvf::Vec3d& lowestPoint ) const = 0;
    virtual double getParameterWeightForCell( size_t globalCellIdx, const std::vector<double>& parameterWeights ) const = 0;

    // Use this function to get the result index into grid cell results. The index will differ if we have active cells
    virtual size_t gridResultIndex( size_t globalCellIdx ) const;

    double calculateValueInMapCell( uint i, uint j, const std::vector<double>& gridCellValues ) const;

protected:
    // Keep track of whether cached data needs updating
    bool gridMappingNeedsUpdating() const;
    bool resultsNeedsUpdating( int timeStep ) const;
    bool geometryNeedsUpdating() const;
    bool resultRangeIsValid() const;
    void clearGridMapping();
    void clearResults();
    void clearTimeStepRange();

    double maxValue( const std::vector<double>& aggregatedResults ) const;
    double minValue( const std::vector<double>& aggregatedResults ) const;

    virtual std::pair<double, double> minmaxValuesAllTimeSteps();

    virtual cvf::ref<cvf::UByteArray>                   getCellVisibility() const;
    virtual std::vector<bool>                           getMapCellVisibility();
    bool                                                mapCellVisibilityNeedsUpdating();
    std::vector<std::vector<std::pair<size_t, double>>> generateGridMapping();

    void                    generateVertexResults();
    void                    generateTrianglesWithVertexValues();
    std::vector<cvf::Vec3d> generateVertices() const;
    void                    generateContourPolygons();
    ContourPolygons createContourPolygonsFromLineSegments( caf::ContourLines::ListOfLineSegments& unorderedLineSegments, double contourValue );
    void          smoothContourPolygons( ContourPolygons* contourPolygons, bool favourExpansion );
    void          clipContourPolygons( ContourPolygons* contourPolygons, const ContourPolygons* clipBy );
    static double sumPolygonArea( const ContourPolygons& contourPolygons );
    static double sumTriangleAreas( const std::vector<cvf::Vec4d>& triangles );

    std::vector<CellIndexAndResult> cellOverlapVolumesAndResults( const cvf::Vec2d&          globalPos2d,
                                                                  const std::vector<double>& weightingResultValues ) const;
    std::vector<CellIndexAndResult> cellRayIntersectionAndResults( const cvf::Vec2d&          globalPos2d,
                                                                   const std::vector<double>& weightingResultValues ) const;

    bool        isMeanResult() const;
    bool        isStraightSummationResult() const;
    static bool isStraightSummationResult( ResultAggregationEnum aggregationType );

    double interpolateValue( const cvf::Vec2d& gridPosition2d ) const;
    double valueInCell( uint i, uint j ) const;
    bool   hasResultInCell( uint i, uint j ) const;
    double calculateValueAtVertex( uint i, uint j ) const;

    // Cell index and position conversion
    std::vector<CellIndexAndResult> cellsAtIJ( uint i, uint j ) const;
    size_t                          cellIndexFromIJ( uint i, uint j ) const;
    size_t                          vertexIndexFromIJ( uint i, uint j ) const;
    cvf::Vec2ui                     ijFromVertexIndex( size_t gridIndex ) const;
    cvf::Vec2ui                     ijFromCellIndex( size_t mapIndex ) const;
    cvf::Vec2ui                     ijFromLocalPos( const cvf::Vec2d& localPos2d ) const;
    cvf::Vec2d                      cellCenterPosition( uint i, uint j ) const;
    cvf::Vec2d                      origin2d() const;

    cvf::Vec2ui  calculateMapSize() const;
    double       gridEdgeOffset() const;
    virtual void updateAfterResultGeneration( int timeStep ) = 0;

protected:
    // Framework overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void initAfterRead() override;

protected:
    caf::PdmField<double> m_relativeSampleSpacing;

    caf::PdmField<ResultAggregation> m_resultAggregation;
    caf::PdmField<bool>              m_showContourLines;
    caf::PdmField<bool>              m_showContourLabels;
    caf::PdmField<bool>              m_smoothContourLines;

    cvf::ref<cvf::UByteArray>                           m_cellGridIdxVisibility;
    std::vector<double>                                 m_aggregatedResults;
    std::vector<double>                                 m_aggregatedVertexResults;
    std::vector<std::vector<std::pair<size_t, double>>> m_projected3dGridIndices;

    cvf::Vec2d                   m_pickPoint;
    cvf::Vec2ui                  m_mapSize;
    cvf::BoundingBox             m_expandedBoundingBox;
    cvf::BoundingBox             m_gridBoundingBox;
    std::vector<ContourPolygons> m_contourPolygons;
    std::vector<double>          m_contourLevelCumulativeAreas;
    std::vector<cvf::Vec4d>      m_trianglesWithVertexValues;
    int                          m_currentResultTimestep;
    std::vector<bool>            m_mapCellVisibility;

    double m_minResultAllTimeSteps;
    double m_maxResultAllTimeSteps;
};
