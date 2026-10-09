/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

class RimcSegmentCollection_addSegmentInterval : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSegmentCollection_addSegmentInterval( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_diameter;
    caf::PdmField<double> m_roughnessFactor;
};

//==================================================================================================
///
//==================================================================================================
class RimcSegmentInterval_setSegmentLengthBase : public caf::PdmObjectMethod
{
public:
    RimcSegmentInterval_setSegmentLengthBase( caf::PdmObjectHandle* self, const QString& description );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

protected:
    enum class LengthType
    {
        FIXED,
        MIN,
        MAX
    };

    virtual LengthType lengthType() const = 0;

private:
    caf::PdmField<double> m_length;
    caf::PdmField<bool>   m_enable;
};

//==================================================================================================
///
//==================================================================================================
class RimcSegmentInterval_setFixedSegmentLength : public RimcSegmentInterval_setSegmentLengthBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSegmentInterval_setFixedSegmentLength( caf::PdmObjectHandle* self );

protected:
    LengthType lengthType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimcSegmentInterval_setMinSegmentLength : public RimcSegmentInterval_setSegmentLengthBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSegmentInterval_setMinSegmentLength( caf::PdmObjectHandle* self );

protected:
    LengthType lengthType() const override;
};

//==================================================================================================
///
//==================================================================================================
class RimcSegmentInterval_setMaxSegmentLength : public RimcSegmentInterval_setSegmentLengthBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcSegmentInterval_setMaxSegmentLength( caf::PdmObjectHandle* self );

protected:
    LengthType lengthType() const override;
};