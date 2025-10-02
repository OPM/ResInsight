/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

#include <optional>
#include <string>

namespace Opm
{
class DeckKeyword;
class DeckRecord;
class DeckItem;
} // namespace Opm

//==================================================================================================
///
///
//==================================================================================================
class RimKeywordWconinje : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimKeywordWconinje();
    ~RimKeywordWconinje() override;

    void uiOrdering( caf::PdmUiGroup* uiGroup );

    Opm::DeckKeyword keyword( const QString& wellName );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<QString>               m_type;
    caf::PdmField<QString>               m_status;
    caf::PdmField<QString>               m_target;
    caf::PdmField<std::optional<double>> m_rate;
    caf::PdmField<std::optional<double>> m_resv;
    caf::PdmField<std::optional<double>> m_bhp;
    caf::PdmField<std::optional<double>> m_thp;
    caf::PdmField<std::optional<int>>    m_vfptab;
    caf::PdmField<std::optional<double>> m_rsrvinj;
};
