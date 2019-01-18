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

#include "RigFemPart.h"
#include "RigFemResultAddress.h"

#include "RimCheckableNamedObject.h"
#include "RimContourMapProjection.h"
#include "RimGeoMechCase.h"
#include "RimRegularLegendConfig.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfString.h"
#include "cvfVector2.h"

class RimGeoMechContourMapView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechContourMapProjection : public RimContourMapProjection
{
    CAF_PDM_HEADER_INIT;
public:

    RimGeoMechContourMapProjection();
    ~RimGeoMechContourMapProjection() override;

    // GeoMech case overrides for contour map methods
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

protected:
    typedef RimContourMapProjection::CellIndexAndResult CellIndexAndResult;

    cvf::ref<cvf::UByteArray>       getCellVisibility() const override;
    void                            ensureOnlyValidPorBarVisible(cvf::UByteArray* visibility, int timeStep) const;
    void                            updateGridInformation() override;
    std::vector<double>             retrieveParameterWeights() override;
    std::vector<double>             generateResults(int timeStep, int everyNCells = 1) override;
    bool                            resultVariableChanged() const override;
    void                            clearResultVariable() override;
    RimGridView*                    baseView() const override;
    std::vector<size_t>             findIntersectingCells(const cvf::BoundingBox& bbox) const override;
    double                          calculateOverlapVolume(size_t globalCellIdx, const cvf::BoundingBox& bbox, size_t* cellKLayerOut) const override;
    double                          calculateRayLengthInCell(size_t globalCellIdx, const cvf::Vec3d& highestPoint, const cvf::Vec3d& lowestPoint, size_t* cellKLayerOut) const override;
    double                          getParameterWeightForCell(size_t globalCellIdx, const std::vector<double>& parameterWeights) const override;

    // GeoMech implementation specific data generation methods
    double                          gridCellValue(size_t globalCellIdx) const override;

    RimGeoMechCase*                 geoMechCase() const;
    RimGeoMechContourMapView*       view() const;


protected:
    // Framework overrides
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

protected:
    cvf::ref<RigFemPart>                          m_femPart;
    cvf::cref<RigFemPartGrid>                     m_femPartGrid;
    RigFemResultAddress                           m_currentResultAddr;

    std::vector<float>                            m_resultValues;
};


