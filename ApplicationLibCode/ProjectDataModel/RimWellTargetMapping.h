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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "Well/RigWellTargetMapping.h"

class RimEclipseResultDefinition;
class RimEclipseCase;

//==================================================================================================
///
///
//==================================================================================================
class RimWellTargetMapping : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellTargetMapping();
    ~RimWellTargetMapping() override;

    void updateResultDefinition();

    RimEclipseCase* ensembleStatisticsCase() const;

    void setDefaults();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;

private:
    void        generateCandidates( RimEclipseCase* eclipseCase );
    void        updateAllBoundaries();
    void        generateEnsembleStatistics();
    cvf::Vec3st getResultGridCellCount() const;

    RimEclipseCase* firstCase() const;

    static std::vector<RigWellTargetMapping::VolumesType> findAvailableVolumesTypes( RimEclipseCase* eclipseCase );

    RigWellTargetMapping::ClusteringLimits getClusteringLimits() const;

    caf::PdmField<int> m_timeStep;

    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeType>>       m_volumeType;
    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeResultType>> m_volumeResultType;
    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumesType>>      m_volumesType;

    caf::PdmField<double> m_pressure;
    caf::PdmField<double> m_permeability;
    caf::PdmField<double> m_transmissibility;

    caf::PdmField<int> m_maxIterations;
    caf::PdmField<int> m_maxClusters;

    caf::PdmChildField<RimEclipseResultDefinition*> m_resultDefinition;

    caf::PdmField<int> m_cellCountI;
    caf::PdmField<int> m_cellCountJ;
    caf::PdmField<int> m_cellCountK;

    caf::PdmField<bool> m_generateButton;

    caf::PdmChildField<RimEclipseCase*> m_ensembleStatisticsCase;

    double m_minimumPressure;
    double m_maximumPressure;

    double m_minimumPermeability;
    double m_maximumPermeability;

    double m_minimumTransmissibility;
    double m_maximumTransmissibility;
};
