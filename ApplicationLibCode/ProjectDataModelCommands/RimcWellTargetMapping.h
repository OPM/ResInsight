/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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
#include "cafPdmObjectMethod.h"

#include <QString>

class RimWellTargetMapping;

//==================================================================================================
///
//==================================================================================================
class RimcWellTargetMapping_compute : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellTargetMapping_compute( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
};

//==================================================================================================
/// Creates a well target mapping with default settings. The parent object adds the mapping.
//==================================================================================================
class RimcWellTargetMapping_addBase : public caf::PdmObjectCreationMethod
{
public:
    RimcWellTargetMapping_addBase( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

protected:
    virtual std::expected<void, QString> validateParent() const { return {}; }
    virtual void                         addToParent( RimWellTargetMapping* wellTargetMapping ) = 0;

private:
    caf::PdmField<QString> m_name;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_addWellTargetMapping : public RimcWellTargetMapping_addBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_addWellTargetMapping( caf::PdmObjectHandle* self );

protected:
    std::expected<void, QString> validateParent() const override;
    void                         addToParent( RimWellTargetMapping* wellTargetMapping ) override;
};

//==================================================================================================
///
//==================================================================================================
class RimcReservoirGridEnsemble_addWellTargetMapping : public RimcWellTargetMapping_addBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimcReservoirGridEnsemble_addWellTargetMapping( caf::PdmObjectHandle* self );

protected:
    void addToParent( RimWellTargetMapping* wellTargetMapping ) override;
};
