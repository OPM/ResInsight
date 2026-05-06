/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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
#include "cafPdmPtrField.h"

#include <QString>

class RimCellFilter;
class RimValveTemplate;

//==================================================================================================
///
//==================================================================================================
class RimcPerforationInterval_addValve : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcPerforationInterval_addValve( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmPtrField<RimValveTemplate*> m_template;
    caf::PdmField<double>               m_startMd;
    caf::PdmField<double>               m_endMd;
    caf::PdmField<int>                  m_valveCount;
};

//==================================================================================================
/// Set the cell filter associated with this perforation interval. Replaces any existing filter.
//==================================================================================================
class RimcPerforationInterval_addFilter : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcPerforationInterval_addFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmPtrField<RimCellFilter*> m_filter;
};

//==================================================================================================
/// Returns the cell filter associated with this perforation interval, or null if none.
//==================================================================================================
class RimcPerforationInterval_cellFilter : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcPerforationInterval_cellFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};
