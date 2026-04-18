/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

class RimColorLegend;

//==================================================================================================
///
//==================================================================================================
class RimcColorLegendCollection_createColorLegend : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcColorLegendCollection_createColorLegend( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_legendName;
};

//==================================================================================================
///
//==================================================================================================
class RimcColorLegend_addColorLegendItem : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcColorLegend_addColorLegendItem( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<int>     m_categoryValue;
    caf::PdmField<QString> m_categoryName;
    caf::PdmField<int>     m_red;
    caf::PdmField<int>     m_green;
    caf::PdmField<int>     m_blue;
};

//==================================================================================================
///
//==================================================================================================
class RimcColorLegendCollection_setDefaultColorLegendForResult : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcColorLegendCollection_setDefaultColorLegendForResult( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int>                m_caseId;
    caf::PdmField<QString>            m_resultName;
    caf::PdmPtrField<RimColorLegend*> m_colorLegend;
};

//==================================================================================================
///
//==================================================================================================
class RimcColorLegendCollection_deleteColorLegend : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcColorLegendCollection_deleteColorLegend( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int>     m_caseId;
    caf::PdmField<QString> m_resultName;
};
