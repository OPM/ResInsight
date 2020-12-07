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
#include "RimContourMapProjection.h"
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfString.h"
#include "cvfVector2.h"

class RigActiveCellInfo;
class RigMainGrid;
class RigResultAccessor;
class RimEclipseContourMapView;
class RimEclipseCase;
class RimEclipseResultDefinition;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseContourMapProjection : public RimContourMapProjection
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseContourMapProjection();
    ~RimEclipseContourMapProjection() override;

    QString weightingParameter() const;
    void    updatedWeightingResult();

    // Eclipse case overrides for contour map methods
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

    double sampleSpacing() const override;

protected:
    typedef RimContourMapProjection::CellIndexAndResult CellIndexAndResult;

    void                updateGridInformation() override;
    std::vector<double> retrieveParameterWeights() override;
    std::vector<double> generateResults( int timeStep ) override;
    bool                resultVariableChanged() const override;
    void                clearResultVariable() override;
    RimGridView*        baseView() const override;
    std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& bbox ) const override;
    size_t              kLayer( size_t globalCellIdx ) const override;
    size_t              kLayers() const override;
    double              calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const override;
    double              calculateRayLengthInCell( size_t            globalCellIdx,
                                                  const cvf::Vec3d& highestPoint,
                                                  const cvf::Vec3d& lowestPoint ) const override;
    double getParameterWeightForCell( size_t cellResultIdx, const std::vector<double>& parameterWeights ) const override;
    size_t gridResultIndex( size_t globalCellIdx ) const override;

    // Eclipse implementation specific data generation methods
    std::vector<double> calculateColumnResult( ResultAggregation resultAggregation ) const;

    RimEclipseCase*           eclipseCase() const;
    RimEclipseContourMapView* view() const;

    void updateAfterResultGeneration( int timeStep ) override;

protected:
    // Framework overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

protected:
    caf::PdmField<bool>                             m_weightByParameter;
    caf::PdmChildField<RimEclipseResultDefinition*> m_weightingResult;

    cvf::ref<RigMainGrid>       m_mainGrid;
    cvf::ref<RigActiveCellInfo> m_activeCellInfo;
    size_t                      m_kLayers;

    QString m_currentResultName;
};
