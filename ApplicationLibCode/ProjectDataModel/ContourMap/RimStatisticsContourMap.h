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

#include "ContourMap/RimContourMapResolutionTools.h"
#include "RimContourMapProjection.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include "cvfVector3.h"

#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace caf
{
class ProgressInfo;
}

class RigContourMapGrid;
class RimEclipseCase;
class RimEclipseResultDefinition;
class RimEclipseCaseEnsemble;
class RimFormationNames;
class RimReservoirGridEnsemble;
class RimEclipseContourMapView;
class RimStatisticsContourMapView;
class RimPolygon;
class RigPolyLinesData;

//==================================================================================================
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

    enum class GridImportMode
    {
        SHARED_GRID,
        INDIVIDUAL_GRIDS
    };

    RimStatisticsContourMap();

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

    void setGridImportMode( GridImportMode mode );

    QString ensembleName() const;

    RigContourMapGrid*  contourMapGrid() const;
    std::vector<double> result( size_t timeStep, StatisticsType statisticsType ) const;

    void                                      addView( RimStatisticsContourMapView* view );
    std::vector<RimStatisticsContourMapView*> views() const;

    void ensureResultsComputed();

    QString resultAggregationText() const;
    QString resultVariable() const;
    double  sampleSpacingFactor() const;
    bool    isColumnResult() const;

    std::vector<int>       selectedTimeSteps() const;
    std::vector<QDateTime> selectedTimeStepDates() const;
    QString                timeStepName( int timeStep ) const;

    std::vector<QString>                 selectedFormations() const;
    std::vector<std::vector<cvf::Vec3d>> selectedPolygons();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void switchToSelectedSourceCase();

private:
    using TimestepResultsMap = std::map<size_t, std::vector<std::vector<double>>>;

    void computeStatistics();
    void computeStatisticsSharedGrid( RigContourMapGrid* contourMapGrid, TimestepResultsMap& timestepResults, caf::ProgressInfo& progInfo );
    void computeStatisticsIndividualGrids( RigContourMapGrid* contourMapGrid, TimestepResultsMap& timestepResults, caf::ProgressInfo& progInfo );
    void onComputeStatisticsClicked();
    void doStatisticsCalculation( TimestepResultsMap& timestep_results );

    RimFormationNames*           activeFormationNames() const;
    std::vector<RimEclipseCase*> ensembleCases() const;
    std::set<RimEclipseCase*>    ensembleCasesInViews() const;

    std::vector<std::pair<int, int>> mapLocalToGlobalTimeSteps( std::vector<QDateTime> localDates ) const;

private:
    caf::PdmField<double>                                     m_boundingBoxExpPercent;
    caf::PdmField<RimContourMapProjection::ResultAggregation> m_resultAggregation;
    caf::PdmField<std::vector<int>>                           m_selectedTimeSteps;
    caf::PdmChildField<RimEclipseResultDefinition*>           m_resultDefinition;
    caf::PdmField<bool>                                       m_enableFormationFilter;
    caf::PdmField<std::vector<QString>>                       m_selectedFormations;
    caf::PdmPtrField<RimEclipseCase*>                         m_primaryCase;
    caf::PdmPtrArrayField<RimPolygon*>                        m_selectedPolygons;

    caf::PdmField<caf::AppEnum<GridImportMode>>                                   m_gridImportMode;
    caf::PdmField<caf::AppEnum<RimContourMapResolutionTools::SamplingResolution>> m_resolution;

    caf::PdmField<caf::AppEnum<RigFloodingSettings::FloodingType>> m_oilFloodingType;
    caf::PdmField<caf::AppEnum<RigFloodingSettings::FloodingType>> m_gasFloodingType;
    caf::PdmField<double>                                          m_userDefinedFloodingGas;
    caf::PdmField<double>                                          m_userDefinedFloodingOil;

    std::unique_ptr<RigContourMapGrid>                              m_contourMapGrid;
    std::map<size_t, std::map<StatisticsType, std::vector<double>>> m_timeResults;

    caf::PdmChildArrayField<RimStatisticsContourMapView*> m_views;

    RimEclipseCase* m_openEclipseCase;
};
