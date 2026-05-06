/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

//==================================================================================================
/// Adds a new property filter as a child of a RimCombinedFilter.
//==================================================================================================
class RimcCombinedFilter_addPropertyFilter : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcCombinedFilter_addPropertyFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_resultVariable;
    caf::PdmField<QString> m_resultType;
};

//==================================================================================================
/// Adds a new IJK range filter as a child of a RimCombinedFilter.
//==================================================================================================
class RimcCombinedFilter_addRangeFilter : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcCombinedFilter_addRangeFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<int>     m_startI;
    caf::PdmField<int>     m_startJ;
    caf::PdmField<int>     m_startK;
    caf::PdmField<int>     m_cellCountI;
    caf::PdmField<int>     m_cellCountJ;
    caf::PdmField<int>     m_cellCountK;
};
