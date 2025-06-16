/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include <memory>

//==================================================================================================
///
//==================================================================================================
class RimcEclipseStatisticsCase_setSourceProperties : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseStatisticsCase_setSourceProperties( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString>              m_propertyType;
    caf::PdmField<std::vector<QString>> m_propertyNames;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseStatisticsCase_computeStatistics : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseStatisticsCase_computeStatistics( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseStatisticsCase_clearSourceProperties : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseStatisticsCase_clearSourceProperties( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};
