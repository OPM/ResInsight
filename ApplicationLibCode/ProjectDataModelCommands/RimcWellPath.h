/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimFishbonesDefines.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimStimPlanFractureTemplate;
class RimThermalFractureTemplate;
class RimEclipseCase;

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addFracture : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addFracture( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

private:
    caf::PdmField<double>                          m_md;
    caf::PdmPtrField<RimStimPlanFractureTemplate*> m_stimPlanFractureTemplate;
    caf::PdmField<bool>                            m_alignDip;
    caf::PdmPtrField<RimEclipseCase*>              m_eclipseCase;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addThermalFracture : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addThermalFracture( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

private:
    caf::PdmPtrField<RimThermalFractureTemplate*> m_fractureTemplate;
    caf::PdmField<double>                         m_md;
    caf::PdmField<bool>                           m_placeUsingTemplateData;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_appendPerforationInterval : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_appendPerforationInterval( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_diameter;
    caf::PdmField<double> m_skinFactor;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_multiSegmentWellSettings : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_multiSegmentWellSettings( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    bool                                          isNullptrValidResult_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_appendFishbones : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_appendFishbones( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    bool                                          resultIsPersistent_obsolete() const override;
    bool                                          isNullptrValidResult_obsolete() const override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;

private:
    caf::PdmField<std::vector<double>>                             m_subLocations;
    caf::PdmField<caf::AppEnum<RimFishbonesDefines::DrillingType>> m_drillingType;
};
