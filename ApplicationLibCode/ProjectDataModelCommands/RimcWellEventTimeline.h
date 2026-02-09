/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimKeywordEvent.h"
#include "RimWellEventControl.h"
#include "RimWellEventPerf.h"
#include "RimWellEventState.h"
#include "RimWellEventTimeline.h"
#include "RimWellEventValve.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseCase;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addPerfEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addPerfEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>                               m_eventDate;
    caf::PdmPtrField<RimWellPath*>                       m_wellPath;
    caf::PdmField<double>                                m_startMd;
    caf::PdmField<double>                                m_endMd;
    caf::PdmField<double>                                m_diameter;
    caf::PdmField<double>                                m_skinFactor;
    caf::PdmField<caf::AppEnum<RimWellEventPerf::State>> m_state;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addValveEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addValveEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>                                    m_eventDate;
    caf::PdmPtrField<RimWellPath*>                            m_wellPath;
    caf::PdmField<double>                                     m_measuredDepth;
    caf::PdmField<caf::AppEnum<RimWellEventValve::ValveType>> m_valveType;
    caf::PdmField<caf::AppEnum<RimWellEventValve::State>>     m_state;
    caf::PdmField<double>                                     m_flowCoefficient;
    caf::PdmField<double>                                     m_area;
    caf::PdmField<double>                                     m_aicdStrength;
    caf::PdmField<double>                                     m_aicdDensityCalibFluid;
    caf::PdmField<double>                                     m_aicdViscosityCalibFluid;
    caf::PdmField<double>                                     m_aicdVolFlowExp;
    caf::PdmField<double>                                     m_aicdViscFuncExp;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addStateEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addStateEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>                                    m_eventDate;
    caf::PdmPtrField<RimWellPath*>                            m_wellPath;
    caf::PdmField<caf::AppEnum<RimWellEventState::WellState>> m_wellState;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addControlEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addControlEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>                                        m_eventDate;
    caf::PdmPtrField<RimWellPath*>                                m_wellPath;
    caf::PdmField<caf::AppEnum<RimWellEventControl::ControlMode>> m_controlMode;
    caf::PdmField<double>                                         m_controlValue;
    caf::PdmField<double>                                         m_bhpLimit;
    caf::PdmField<double>                                         m_oilRate;
    caf::PdmField<double>                                         m_waterRate;
    caf::PdmField<double>                                         m_gasRate;
    caf::PdmField<bool>                                           m_isProducer;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addTubingEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addTubingEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>         m_eventDate;
    caf::PdmPtrField<RimWellPath*> m_wellPath;
    caf::PdmField<double>          m_startMd;
    caf::PdmField<double>          m_endMd;
    caf::PdmField<double>          m_innerDiameter;
    caf::PdmField<double>          m_roughness;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addWellKeywordEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addWellKeywordEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>              m_eventDate;
    caf::PdmPtrField<RimWellPath*>      m_wellPath;
    caf::PdmField<QString>              m_keywordName;
    caf::PdmField<std::vector<QString>> m_itemNames;
    caf::PdmField<std::vector<QString>> m_itemTypes;
    caf::PdmField<std::vector<QString>> m_itemValues;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_addKeywordEvent : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_addKeywordEvent( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString>              m_eventDate;
    caf::PdmField<QString>              m_keywordName;
    caf::PdmField<std::vector<QString>> m_itemNames;
    caf::PdmField<std::vector<QString>> m_itemTypes;
    caf::PdmField<std::vector<QString>> m_itemValues;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_setTimestamp : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_setTimestamp( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_timestamp;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellEventTimeline_generateSchedule : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellEventTimeline_generateSchedule( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<int> m_eclipseCaseId;
};
