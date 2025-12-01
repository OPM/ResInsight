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

#include "cafAppEnum.h"
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
class RimKeywordBcprop : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class Type
    {
        DIRICHLET,
        FREE,
        RATE,
        THERMAL,
        NONE
    };

    enum class Component
    {
        GAS,
        OIL,
        WATER,
        SOLVENT,
        POLYMER,
        MICR,
        OXYG,
        UREA,
        NONE
    };

    enum class MechType
    {
        FREE,
        FIXED,
        NONE
    };

    RimKeywordBcprop();
    ~RimKeywordBcprop() override;

    void setIndex( int index );

    void uiOrdering( caf::PdmUiGroup* uiGroup );

    Opm::DeckKeyword keyword();

private:
    caf::PdmField<int>                     m_index;
    caf::PdmField<caf::AppEnum<Type>>      m_type;
    caf::PdmField<caf::AppEnum<Component>> m_component;
    caf::PdmField<double>                  m_rate;
    caf::PdmField<std::optional<double>>   m_press;
    caf::PdmField<std::optional<double>>   m_temp;
    caf::PdmField<caf::AppEnum<MechType>>  m_mechType;
    caf::PdmField<size_t>                  m_fixedX;
    caf::PdmField<size_t>                  m_fixedY;
    caf::PdmField<size_t>                  m_fixedZ;
};
