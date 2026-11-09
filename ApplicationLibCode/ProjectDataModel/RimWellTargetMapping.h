/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "RigFloodingSettings.h"
#include "Well/RigWellTargetMapping.h"

#include <expected>
#include <optional>

class RimEclipseResultDefinition;
class RimEclipseCase;
class RimCellFilter;
class RimDataFilterCollection;
class RimWellTargetClusterStatistics;

//==================================================================================================
///
///
//==================================================================================================
class RimWellTargetMapping : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    using FloodingType = caf::AppEnum<RigFloodingSettings::FloodingType>;

    // Values to override after setDefaults(). Unset values keep their current value.
    struct Settings
    {
        std::optional<int>                                    timeStep;
        std::optional<RigWellTargetMapping::VolumeType>       volumeType;
        std::optional<RigWellTargetMapping::VolumeResultType> volumeResultType;
        std::optional<RigWellTargetMapping::VolumesType>      volumesType;
        std::optional<RigFloodingSettings::FloodingType>      oilFloodingType;
        std::optional<double>                                 userDefinedFloodingOil;
        std::optional<RigFloodingSettings::FloodingType>      gasFloodingType;
        std::optional<double>                                 userDefinedFloodingGas;
        std::optional<double>                                 minimumSaturationOil;
        std::optional<double>                                 minimumSaturationGas;
        std::optional<double>                                 minimumPressure;
        std::optional<double>                                 minimumPermeability;
        std::optional<double>                                 minimumTransmissibility;
        std::optional<int>                                    maxNumTargets;
        std::optional<int>                                    maxIterations;
        std::optional<RimCellFilter*>                         startDataFilter;
        std::optional<RimCellFilter*>                         growDataFilter;
        std::optional<bool>                                   avoidFaults;
        std::optional<int>                                    cellCountI;
        std::optional<int>                                    cellCountJ;
        std::optional<int>                                    cellCountK;
        std::optional<double>                                 expandBoundingBoxXYPercent;
        std::optional<double>                                 expandBoundingBoxZPercent;
    };

    RimWellTargetMapping();
    ~RimWellTargetMapping() override;

    void updateResultDefinition();

    RimEclipseCase* ensembleStatisticsCase() const;

    void setDefaults();
    void applySettings( const Settings& settings );

    std::expected<void, QString> compute();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void initAfterRead() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    std::expected<void, QString> generateCandidates( RimEclipseCase* eclipseCase, bool setTimeStepInView = true );
    std::expected<void, QString> generateEnsembleStatistics();
    std::expected<void, QString> validateInput( RimEclipseCase* eclipseCase ) const;
    void                         updateAllBoundaries();
    cvf::Vec3st                  getResultGridCellCount() const;

    RimEclipseCase*          firstCase() const;
    RimDataFilterCollection* dataFilterCollection() const;

    static std::vector<RigWellTargetMapping::VolumesType> findAvailableVolumesTypes( RimEclipseCase* eclipseCase );

    RigWellTargetMapping::ClusteringLimits getClusteringLimits() const;

    void resetMinimumCellValuesToDefault();
    void onGenerateButtonClicked();

    caf::PdmField<int> m_timeStep;

    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeType>>       m_volumeType;
    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeResultType>> m_volumeResultType;
    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumesType>>      m_volumesType;

    caf::PdmField<FloodingType> m_oilFloodingType;
    caf::PdmField<FloodingType> m_gasFloodingType;
    caf::PdmField<double>       m_userDefinedFloodingGas;
    caf::PdmField<double>       m_userDefinedFloodingOil;

    caf::PdmField<double> m_saturationOil;
    caf::PdmField<double> m_saturationGas;
    caf::PdmField<double> m_pressure;
    caf::PdmField<double> m_permeability;
    caf::PdmField<double> m_transmissibility;

    caf::PdmField<double> m_expandBoundingBoxXYPercent;
    caf::PdmField<double> m_expandBoundingBoxZPercent;

    caf::PdmField<int> m_maxIterations;
    caf::PdmField<int> m_maxNumTargets;

    caf::PdmChildField<RimEclipseResultDefinition*> m_resultDefinition;

    caf::PdmField<int> m_cellCountI;
    caf::PdmField<int> m_cellCountJ;
    caf::PdmField<int> m_cellCountK;

    caf::PdmPtrField<RimCellFilter*> m_startDataFilter;
    caf::PdmPtrField<RimCellFilter*> m_dataFilter; // grow filter

    caf::PdmField<bool> m_avoidFaults;

    caf::PdmChildField<RimEclipseCase*> m_ensembleStatisticsCase;

    caf::PdmChildArrayField<RimWellTargetClusterStatistics*> m_clusterStatistics;

    double m_minimumSaturationOil;
    double m_maximumSaturationOil;
    double m_defaultSaturationOil;

    double m_minimumSaturationGas;
    double m_maximumSaturationGas;
    double m_defaultSaturationGas;

    double m_minimumPressure;
    double m_maximumPressure;
    double m_defaultPressure;

    double m_minimumPermeability;
    double m_maximumPermeability;
    double m_defaultPermeability;

    double m_minimumTransmissibility;
    double m_maximumTransmissibility;
    double m_defaultTransmissibility;
};
