/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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

#include "RigFloodingSettings.h"
#include "Well/RigWellTargetMapping.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimCellFilter;
class RimWellTargetMapping;

//==================================================================================================
///
//==================================================================================================
class RimcWellTargetMapping_compute : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellTargetMapping_compute( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
/// Shared parameters for creating a well target mapping. The parent object adds the mapping.
//==================================================================================================
class RimcWellTargetMapping_addBase : public caf::PdmObjectCreationMethod
{
public:
    RimcWellTargetMapping_addBase( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

protected:
    virtual std::expected<void, QString> validateParent() const { return {}; }
    virtual void                         addToParent( RimWellTargetMapping* wellTargetMapping ) = 0;

private:
    std::expected<void, QString> applyParameters( RimWellTargetMapping* wellTargetMapping ) const;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<int>     m_timeStep;

    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeType>>       m_volumeType;
    caf::PdmField<caf::AppEnum<RigWellTargetMapping::VolumeResultType>> m_volumeResultType;
    caf::PdmField<QString>                                              m_volumesType;

    caf::PdmField<caf::AppEnum<RigFloodingSettings::FloodingType>> m_oilFloodingType;
    caf::PdmField<double>                                          m_userDefinedFloodingOil;
    caf::PdmField<caf::AppEnum<RigFloodingSettings::FloodingType>> m_gasFloodingType;
    caf::PdmField<double>                                          m_userDefinedFloodingGas;

    caf::PdmField<double> m_minimumSaturationOil;
    caf::PdmField<double> m_minimumSaturationGas;
    caf::PdmField<double> m_minimumPressure;
    caf::PdmField<double> m_minimumPermeability;
    caf::PdmField<double> m_minimumTransmissibility;

    caf::PdmField<int> m_maxNumTargets;
    caf::PdmField<int> m_maxIterations;

    caf::PdmPtrField<RimCellFilter*> m_startDataFilter;
    caf::PdmPtrField<RimCellFilter*> m_growDataFilter;
    caf::PdmField<bool>              m_avoidFaults;

    caf::PdmField<int>    m_cellCountI;
    caf::PdmField<int>    m_cellCountJ;
    caf::PdmField<int>    m_cellCountK;
    caf::PdmField<double> m_expandBoundingBoxXYPercent;
    caf::PdmField<double> m_expandBoundingBoxZPercent;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_addWellTargetMapping : public RimcWellTargetMapping_addBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_addWellTargetMapping( caf::PdmObjectHandle* self );

protected:
    std::expected<void, QString> validateParent() const override;
    void                         addToParent( RimWellTargetMapping* wellTargetMapping ) override;
};

//==================================================================================================
///
//==================================================================================================
class RimcReservoirGridEnsemble_addWellTargetMapping : public RimcWellTargetMapping_addBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcReservoirGridEnsemble_addWellTargetMapping( caf::PdmObjectHandle* self );

protected:
    void addToParent( RimWellTargetMapping* wellTargetMapping ) override;
};
