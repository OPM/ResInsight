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
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseResultCase;

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_importWellPath : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_importWellPath( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_fileName;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_importWellPathFromPointsInternal : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_importWellPathFromPointsInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_coordinateXKey;
    caf::PdmField<QString> m_coordinateYKey;
    caf::PdmField<QString> m_coordinateZKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPathCollection_wellCompletions : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPathCollection_wellCompletions( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<QString> m_wellName;
    // caf::PdmPtrField<RimEclipseResultCase*> m_eclipseCase;
    caf::PdmField<int> m_caseId;
};
