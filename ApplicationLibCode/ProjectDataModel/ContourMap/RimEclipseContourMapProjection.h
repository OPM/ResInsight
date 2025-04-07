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

#include "RimContourMapProjection.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RigActiveCellInfo;
class RigMainGrid;
class RigContourMapGrid;
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
    void    clearGridMappingAndRedraw();

    // Eclipse case overrides for contour map methods
    QString                 resultVariableName() const override;
    QString                 resultDescriptionText() const override;
    RimRegularLegendConfig* legendConfig() const override;
    void                    updateLegend() override;

    double sampleSpacing() const override;

protected:
    void                updateGridInformation() override;
    std::vector<double> retrieveParameterWeights() override;
    std::vector<double> generateResults( int timeStep ) const override;
    void                generateAndSaveResults( int timeStep ) override;
    bool                resultVariableChanged() const override;
    void                clearResultVariable() override;
    RimGridView*        baseView() const override;

    RimEclipseCase*           eclipseCase() const;
    RimEclipseContourMapView* view() const;

    std::pair<double, double> computeMinMaxValuesAllTimeSteps() override;

    void updateAfterResultGeneration( int timeStep ) override;

protected:
    // Framework overrides
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

protected:
    caf::PdmField<bool>                             m_weightByParameter;
    caf::PdmChildField<RimEclipseResultDefinition*> m_weightingResult;

    QString m_currentResultName;
};
