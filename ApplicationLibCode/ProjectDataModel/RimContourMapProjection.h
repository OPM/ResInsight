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
#include "RimIntersectionEnums.h"

#include "RigContourMapCalculator.h"
#include "RigContourPolygonsTools.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"
#include "cvfVector2.h"

class RigContourMapGrid;
class RimGridView;
class RimRegularLegendConfig;
class RigContourMapProjection;

//==================================================================================================
///
///
//==================================================================================================
class RimContourMapProjection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    using ResultAggregation = caf::AppEnum<RigContourMapCalculator::ResultAggregationType>;
    using ContourPolygons   = std::vector<RigContourPolygonsTools::ContourPolygon>;

    RimContourMapProjection();
    ~RimContourMapProjection() override;

    void generateResultsIfNecessary( int timeStep );
    void generateGeometryIfNecessary();
    void clearGeometry();

    const std::vector<ContourPolygons>& contourPolygons() const;
    const std::vector<cvf::Vec4d>&      trianglesWithVertexValues();

    virtual double sampleSpacing() const = 0;

    double sampleSpacingFactor() const;
    void   setSampleSpacingFactor( double spacingFactor );
    bool   showContourLines() const;
    bool   showContourLabels() const;

    virtual QString resultAggregationText() const;

    QString caseName() const;
    QString currentTimeStepName() const;

    virtual bool isColumnResult() const;
    bool         isMeanResult() const;
    bool         isStraightSummationResult() const;

    void       setPickPoint( cvf::Vec2d globalPickPoint );
    cvf::Vec2d pickPoint() const;

    cvf::Vec3d origin3d() const;

    // Pure-virtual public methods which should be overridden by Eclipse and Geo-mechanical contour map implementations
    virtual QString                 resultVariableName() const    = 0;
    virtual QString                 resultDescriptionText() const = 0;
    virtual RimRegularLegendConfig* legendConfig() const          = 0;
    virtual void                    updateLegend()                = 0;

    // Use this function to get the result index into grid cell results. The index will differ if we have active cells
    virtual size_t gridResultIndex( size_t globalCellIdx ) const;

    virtual std::vector<double> retrieveParameterWeights() = 0;

    virtual cvf::ref<cvf::UByteArray> getCellVisibility() const;

    const RigContourMapProjection* mapProjection() const;
    const RigContourMapGrid*       mapGrid() const;

protected:
    // Protected virtual methods to be overridden by Eclipse and Geo-mechanical contour map implementations
    virtual void                updateGridInformation()                = 0;
    virtual std::vector<double> generateResults( int timeStep ) const  = 0;
    virtual void                generateAndSaveResults( int timeStep ) = 0;
    virtual bool                resultVariableChanged() const          = 0;
    virtual void                clearResultVariable()                  = 0;
    virtual RimGridView*        baseView() const                       = 0;

    double calculateValueInMapCell( uint i, uint j, const std::vector<double>& gridCellValues ) const;

    void clearGridMapping();

    virtual std::pair<double, double> computeMinMaxValuesAllTimeSteps() = 0;
    std::pair<double, double>         minmaxValuesAllTimeSteps();

    bool mapCellVisibilityNeedsUpdating( int timeStep );

    void generateVertexResults();

    double       gridEdgeOffset() const;
    virtual void updateAfterResultGeneration( int timeStep ) = 0;

protected:
    // Framework overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void initAfterRead() override;

private:
    bool                                     resultsNeedsUpdating( int timeStep ) const;
    bool                                     gridMappingNeedsUpdating() const;
    bool                                     geometryNeedsUpdating() const;
    void                                     clearResults();
    void                                     clearMinMaxValueRange();
    bool                                     resultRangeIsValid() const;
    std::optional<std::pair<double, double>> valueFilterMinMax() const;

protected:
    caf::PdmField<double> m_relativeSampleSpacing;

    caf::PdmField<ResultAggregation> m_resultAggregation;
    caf::PdmField<bool>              m_showContourLines;
    caf::PdmField<bool>              m_showContourLabels;
    caf::PdmField<bool>              m_smoothContourLines;

    cvf::Vec2d                   m_pickPoint;
    std::vector<ContourPolygons> m_contourPolygons;
    std::vector<double>          m_contourLevelCumulativeAreas;
    std::vector<cvf::Vec4d>      m_trianglesWithVertexValues;
    int                          m_currentResultTimestep;
    std::vector<bool>            m_mapCellVisibility;

    caf::PdmField<caf::AppEnum<RimIntersectionFilterEnum>> m_valueFilterType;
    caf::PdmField<double>                                  m_upperThreshold;
    caf::PdmField<double>                                  m_lowerThreshold;

    std::unique_ptr<RigContourMapGrid>       m_contourMapGrid;
    std::unique_ptr<RigContourMapProjection> m_contourMapProjection;

private:
    double m_minResultAllTimeSteps;
    double m_maxResultAllTimeSteps;
};
