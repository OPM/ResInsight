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
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addFracture : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addFracture( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<double>                          m_md;
    caf::PdmPtrField<RimStimPlanFractureTemplate*> m_stimPlanFractureTemplate;
    caf::PdmField<bool>                            m_alignDip;
    caf::PdmPtrField<RimEclipseCase*>              m_eclipseCase;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addThermalFracture : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addThermalFracture( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmPtrField<RimThermalFractureTemplate*> m_fractureTemplate;
    caf::PdmField<double>                         m_md;
    caf::PdmField<bool>                           m_placeUsingTemplateData;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_appendPerforationInterval : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_appendPerforationInterval( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

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
    QString                                       classKeywordReturnedType() const override;
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
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<double>>                             m_subLocations;
    caf::PdmField<caf::AppEnum<RimFishbonesDefines::DrillingType>> m_drillingType;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_extractWellPathPropertiesInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_extractWellPathPropertiesInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<double>  m_resamplingInterval;
    caf::PdmField<QString> m_coordinateX;
    caf::PdmField<QString> m_coordinateY;
    caf::PdmField<QString> m_coordinateZ;
    caf::PdmField<QString> m_measuredDepth;
    caf::PdmField<QString> m_inclination;
    caf::PdmField<QString> m_azimuth;
    caf::PdmField<QString> m_dogleg;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addWellLogInternal : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addWellLogInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_measuredDepthKey;
    caf::PdmField<QString> m_channelKeysCsv;
    caf::PdmField<QString> m_tvdMslKey;
    caf::PdmField<QString> m_tvdRkbKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_appendLateral : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_appendLateral( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimWellPath*> m_lateral;
};
