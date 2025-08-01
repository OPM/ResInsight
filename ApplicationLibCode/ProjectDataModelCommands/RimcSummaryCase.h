/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
class RimSummaryCase_summaryVectorValues : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCase_summaryVectorValues( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_addressString;
};

//==================================================================================================
///
//==================================================================================================
class RimSummaryCase_availableAddresses : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCase_availableAddresses( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimSummaryCase_availableTimeSteps : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCase_availableTimeSteps( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimSummaryCase_resampleValues : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCase_resampleValues( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_addressString;
    caf::PdmField<QString> m_resamplingPeriod;
};

//==================================================================================================
///
//==================================================================================================
class RimSummaryCase_setSummaryVectorValues : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCase_setSummaryVectorValues( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString>            m_addressString;
    caf::PdmField<QString>            m_unitString;
    caf::PdmField<std::vector<float>> m_values;
};
