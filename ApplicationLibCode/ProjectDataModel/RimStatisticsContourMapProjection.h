/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RimContourMapProjection.h"
#include "RimStatisticsContourMap.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RigActiveCellInfo;
class RigMainGrid;
class RigContourMapGrid;
class RigResultAccessor;
class RimStatisticsContourMapView;
class RimEclipseCase;
class RimEclipseResultDefinition;

//==================================================================================================
///
///
//==================================================================================================
class RimStatisticsContourMapProjection : public RimContourMapProjection
{
    CAF_PDM_HEADER_INIT;

public:
    RimStatisticsContourMapProjection();
    ~RimStatisticsContourMapProjection() override;

    void clearGridMappingAndRedraw();

    QString                 statisticsType() const;
    QString                 resultVariableName() const override;
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

    double sampleSpacing() const override;

    bool    isColumnResult() const override;
    QString resultAggregationText() const override;

protected:
    void                updateGridInformation() override;
    std::vector<double> retrieveParameterWeights() override;
    std::vector<double> generateResults( int timeStep ) const override;
    void                generateAndSaveResults( int timeStep ) override;
    bool                resultVariableChanged() const override;
    void                clearResultVariable() override;
    RimGridView*        baseView() const override;

    RimEclipseCase*              eclipseCase() const;
    RimStatisticsContourMap*     statisticsContourMap() const;
    RimStatisticsContourMapView* view() const;

    std::pair<double, double> computeMinMaxValuesAllTimeSteps() override;

    void updateAfterResultGeneration( int timeStep ) override;

protected:
    // Framework overrides
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    caf::PdmField<caf::AppEnum<RimStatisticsContourMap::StatisticsType>> m_statisticsType;

    QString m_currentResultName;
};
