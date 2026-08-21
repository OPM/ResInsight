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
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RimWellEvent.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

//==================================================================================================
///
/// Raw schedule text inserted at a specific position in a dated schedule section.
///
//==================================================================================================
class RimWellEventRawText : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    enum class Placement
    {
        AFTER_DATE,
        BEFORE_KEYWORD,
        AFTER_KEYWORD,
        END_OF_DATE
    };

    RimWellEventRawText();
    ~RimWellEventRawText() override;

    QString   text() const;
    void      setText( const QString& text );
    Placement placement() const;
    void      setPlacement( Placement placement );
    QString   anchorKeyword() const;
    void      setAnchorKeyword( const QString& keyword );
    int       priority() const;
    void      setPriority( int priority );

    EventType eventType() const override;
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<QString>                 m_text;
    caf::PdmField<caf::AppEnum<Placement>> m_placement;
    caf::PdmField<QString>                 m_anchorKeyword;
    caf::PdmField<int>                     m_priority;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventRawText::Placement>::setUp();
} // namespace caf
