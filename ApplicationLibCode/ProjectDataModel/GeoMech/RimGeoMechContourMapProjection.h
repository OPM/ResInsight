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

#include "ContourMap/RimContourMapProjection.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"
#include "cvfBoundingBox.h"

class RimGeoMechContourMapView;
class RimGeoMechCase;

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
    QString                 resultVariableName() const override;
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

    double sampleSpacing() const override;

protected:
    // GeoMech implementation specific data generation methods
    cvf::ref<cvf::UByteArray> getCellVisibility() const override;
    cvf::BoundingBox          calculateExpandedPorBarBBox( int timeStep, int frameIndex ) const;
    void                      updateGridInformation() override;

    std::vector<double> retrieveParameterWeights() override;
    std::vector<double> generateResults( int timeStep ) const override;
    void                generateAndSaveResults( int timeStep ) override;
    std::vector<double>
        generateResultsFromAddress( RigFemResultAddress resultAddress, const std::vector<bool>& mapCellVisibility, int viewerStepIndex ) const;
    bool                      resultVariableChanged() const override;
    void                      clearResultVariable() override;
    RimGridView*              baseView() const override;
    RimGeoMechCase*           geoMechCase() const;
    RimGeoMechContourMapView* view() const;

    std::pair<double, double> computeMinMaxValuesAllTimeSteps() override;

    void updateAfterResultGeneration( int timeStep ) override;

protected:
    // Framework overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

protected:
    caf::PdmField<bool>       m_limitToPorePressureRegions;
    caf::PdmField<bool>       m_applyPPRegionLimitVertically;
    caf::PdmField<double>     m_paddingAroundPorePressureRegion;
    cvf::ref<RigFemPart>      m_femPart;
    cvf::cref<RigFemPartGrid> m_femPartGrid;
    RigFemResultAddress       m_currentResultAddr;
};
