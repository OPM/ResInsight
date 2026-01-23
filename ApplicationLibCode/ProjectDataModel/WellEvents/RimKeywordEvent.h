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
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RimWellEvent.h"

#include "cafPdmField.h"

#include <vector>

//==================================================================================================
///
/// Schedule-level keyword event - represents a schedule keyword with arbitrary keyword items
/// that is NOT tied to a specific well path (e.g., RPTRST, GRUPTREE, RPTSCHED)
///
//==================================================================================================
class RimKeywordEvent : public RimWellEvent
{
    CAF_PDM_HEADER_INIT;

public:
    RimKeywordEvent();
    ~RimKeywordEvent() override;

    // Keyword management
    QString keywordName() const;
    void    setKeywordName( const QString& keyword );

    // Item management
    void addStringItem( const QString& name, const QString& value );
    void addIntItem( const QString& name, int value );
    void addDoubleItem( const QString& name, double value );

    std::vector<class RimWellEventKeywordItem*> items() const;

    // Override from RimWellEvent
    EventType eventType() const override { return EventType::SCHEDULE_KEYWORD; }
    QString   generateScheduleKeyword( const QString& wellName ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmField<QString>                                  m_keywordName;
    caf::PdmChildArrayField<class RimWellEventKeywordItem*> m_items;
};
