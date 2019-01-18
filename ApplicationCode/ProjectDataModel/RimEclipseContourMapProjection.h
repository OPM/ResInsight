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

class RigMainGrid;
class RigResultAccessor;
class RimEclipseContourMapView;
class RimEclipseResultCase;
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

    QString                 weightingParameter() const;
    void                    updatedWeightingResult();

    // Eclipse case overrides for contour map methods
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

protected:
    typedef RimContourMapProjection::CellIndexAndResult CellIndexAndResult;

    void                            updateGridInformation() override;
    std::vector<double>             retrieveParameterWeights() override;
    std::vector<double>             generateResults(int timeStep) override;
    bool                            resultVariableChanged() const override;
    void                            clearResultVariable() override;
    RimGridView*                    baseView() const override;
    std::vector<size_t>             findIntersectingCells(const cvf::BoundingBox& bbox) const override;
    double                          calculateOverlapVolume(size_t globalCellIdx, const cvf::BoundingBox& bbox, size_t* cellKLayerOut) const override;
    double                          calculateRayLengthInCell(size_t globalCellIdx, const cvf::Vec3d& highestPoint, const cvf::Vec3d& lowestPoint, size_t* cellKLayerOut) const override;
    double                          getParameterWeightForCell(size_t globalCellIdx, const std::vector<double>& parameterWeights) const override;
    double                          gridCellValue(size_t globalCellIdx) const override;

    // Eclipse implementation specific data generation methods
    double                          calculateValueInMapCell(uint i, uint j) const;
    double                          calculateColumnResult(ResultAggregation resultAggregation, size_t cellGlobalIdx) const;

    RimEclipseResultCase*           eclipseCase() const;
    RimEclipseContourMapView*       view() const;


protected:
    // Framework overrides
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void initAfterRead() override;

protected:
    caf::PdmField<bool>                                 m_weightByParameter;
    caf::PdmChildField<RimEclipseResultDefinition*>     m_weightingResult;

    cvf::ref<RigResultAccessor>                         m_resultAccessor;

    cvf::ref<RigMainGrid>                               m_mainGrid;
    QString                                             m_currentResultName;
};
