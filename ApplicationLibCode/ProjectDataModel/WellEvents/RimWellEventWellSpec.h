/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWellEvent.h"
#include "RimWellPathCompletionSettings.h"

#include "cafPdmField.h"

#include <optional>

struct RimWellSpecData
{
    QString                                 groupName;
    bool                                    allowCrossFlow = true;
    std::optional<double>                   referenceDepth;
    RimWellPathCompletionSettings::WellType wellType = RimWellPathCompletionSettings::OIL;
};

//==================================================================================================
///
/// Dated snapshot of the WELSPECS-related completion export settings for a well.
///
//==================================================================================================
class RimWellEventWellSpec : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellEventWellSpec();
    ~RimWellEventWellSpec() override;

    RimWellSpecData wellSpecData() const;
    RimWellSpecData baselineData() const;

    void setWellSpecData( const RimWellSpecData& data );
    void setBaselineData( const RimWellSpecData& data );

    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<QString>                                               m_groupName;
    caf::PdmField<bool>                                                  m_allowCrossFlow;
    caf::PdmField<std::optional<double>>                                 m_referenceDepth;
    caf::PdmField<caf::AppEnum<RimWellPathCompletionSettings::WellType>> m_wellType;
    caf::PdmField<QString>                                               m_baselineGroupName;
    caf::PdmField<bool>                                                  m_baselineAllowCrossFlow;
    caf::PdmField<std::optional<double>>                                 m_baselineReferenceDepth;
    caf::PdmField<caf::AppEnum<RimWellPathCompletionSettings::WellType>> m_baselineWellType;
};
