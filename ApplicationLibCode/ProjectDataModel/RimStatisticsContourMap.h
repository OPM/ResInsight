/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimNamedObject.h"
#include "cafPdmField.h"

#include "RimContourMapProjection.h"

#include <map>

class RimEclipseCase;
class RimEclipseResultDefinition;
class RimEclipseCaseEnsemble;

//==================================================================================================
//
//
//
//==================================================================================================
class RimStatisticsContourMap : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class StatisticsType
    {
        P10,
        P50,
        P90,
        MEAN,
        MIN,
        MAX
    };

    RimStatisticsContourMap();

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    RimEclipseCaseEnsemble* ensemble() const;

    RigContourMapGrid*  contourMapGrid() const;
    std::vector<double> result( StatisticsType statisticsType ) const;

    std::vector<std::vector<std::pair<size_t, double>>> gridMapping() const;

    void ensureResultsComputed();

    QString resultAggregationText() const;
    QString resultVariable() const;
    double  sampleSpacingFactor() const;
    bool    isColumnResult() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    void computeStatistics();

    caf::PdmField<double>                                     m_relativeSampleSpacing;
    caf::PdmField<RimContourMapProjection::ResultAggregation> m_resultAggregation;
    caf::PdmField<int>                                        m_timeStep;
    caf::PdmField<double>                                     m_boundingBoxExpPercent;

    caf::PdmChildField<RimEclipseResultDefinition*> m_resultDefinition;
    caf::PdmField<bool>                             m_computeStatisticsButton;

    std::unique_ptr<RigContourMapGrid>                  m_contourMapGrid;
    std::map<StatisticsType, std::vector<double>>       m_result;
    std::vector<std::vector<std::pair<size_t, double>>> m_gridMapping;
};
