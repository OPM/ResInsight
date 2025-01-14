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
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

#include "RimContourMapProjection.h"

#include <map>

class RimEclipseCase;
class RimEclipseResultDefinition;
class RimEclipseCaseEnsemble;
class RimEclipseContourMapView;
class RimStatisticsContourMapView;

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
    std::vector<double> result( size_t timeStep, StatisticsType statisticsType ) const;

    void addView( RimStatisticsContourMapView* view );

    void ensureResultsComputed();

    QString resultAggregationText() const;
    QString resultVariable() const;
    double  sampleSpacingFactor() const;
    bool    isColumnResult() const;

    std::vector<int> selectedTimeSteps() const;
    QString          timeStepName( int timeStep ) const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    RimEclipseCase* switchToSelectedSourceCase();

private:
    void computeStatistics();
    void doStatisticsCalculation( std::map<size_t, std::vector<std::vector<double>>>& timestep_results );

    caf::PdmField<double>                                     m_boundingBoxExpPercent;
    caf::PdmField<double>                                     m_relativeSampleSpacing;
    caf::PdmField<RimContourMapProjection::ResultAggregation> m_resultAggregation;
    caf::PdmField<std::vector<int>>                           m_selectedTimeSteps;
    caf::PdmChildField<RimEclipseResultDefinition*>           m_resultDefinition;
    caf::PdmField<bool>                                       m_computeStatisticsButton;

    caf::PdmField<QString> m_primaryCase;

    std::unique_ptr<RigContourMapGrid>                              m_contourMapGrid;
    std::map<size_t, std::map<StatisticsType, std::vector<double>>> m_timeResults;

    std::vector<std::vector<std::pair<size_t, double>>> m_gridMapping;

    caf::PdmChildArrayField<RimStatisticsContourMapView*> m_views;
};
