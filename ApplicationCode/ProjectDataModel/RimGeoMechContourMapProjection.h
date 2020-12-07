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

    double sampleSpacing() const override;

protected:
    typedef RimContourMapProjection::CellIndexAndResult CellIndexAndResult;

    // GeoMech implementation specific data generation methods
    cvf::ref<cvf::UByteArray> getCellVisibility() const override;
    cvf::BoundingBox          calculateExpandedPorBarBBox( int timeStep ) const;
    void                      updateGridInformation() override;
    std::vector<bool>         getMapCellVisibility() override;
    std::vector<double>       retrieveParameterWeights() override;
    std::vector<double>       generateResults( int timeStep ) override;
    std::vector<double>       generateResultsFromAddress( RigFemResultAddress      resultAddress,
                                                          const std::vector<bool>& mapCellVisibility,
                                                          int                      timeStep );
    bool                      resultVariableChanged() const override;
    void                      clearResultVariable() override;
    RimGridView*              baseView() const override;
    std::vector<size_t>       findIntersectingCells( const cvf::BoundingBox& bbox ) const override;
    size_t                    kLayer( size_t globalCellIdx ) const override;
    double calculateOverlapVolume( size_t globalCellIdx, const cvf::BoundingBox& bbox ) const override;
    double calculateRayLengthInCell( size_t            globalCellIdx,
                                     const cvf::Vec3d& highestPoint,
                                     const cvf::Vec3d& lowestPoint ) const override;
    double getParameterWeightForCell( size_t globalCellIdx, const std::vector<double>& parameterWeights ) const override;
    std::vector<double>       gridCellValues( RigFemResultAddress resAddr, std::vector<float>& resultValues ) const;
    RimGeoMechCase*           geoMechCase() const;
    RimGeoMechContourMapView* view() const;

    void updateAfterResultGeneration( int timeStep ) override;

protected:
    // Framework overrides
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

protected:
    caf::PdmField<bool>       m_limitToPorePressureRegions;
    caf::PdmField<bool>       m_applyPPRegionLimitVertically;
    caf::PdmField<double>     m_paddingAroundPorePressureRegion;
    cvf::ref<RigFemPart>      m_femPart;
    cvf::cref<RigFemPartGrid> m_femPartGrid;
    RigFemResultAddress       m_currentResultAddr;
};
