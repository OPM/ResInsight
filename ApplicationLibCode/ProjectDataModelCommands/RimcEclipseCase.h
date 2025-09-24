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

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_importProperties : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_importProperties( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<std::vector<QString>> m_fileNames;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_exportValuesInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_exportValuesInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_coordinateX;
    caf::PdmField<QString> m_coordinateY;
    caf::PdmField<QString> m_coordinateZ;
    caf::PdmField<QString> m_propertyType;
    caf::PdmField<QString> m_propertyName;
    caf::PdmField<int>     m_timeStep;
    caf::PdmField<QString> m_porosityModel;
    caf::PdmField<QString> m_resultKey;
};

//==================================================================================================
///
//==================================================================================================
class RimcEclipseCase_exportCornerPointGridInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcEclipseCase_exportCornerPointGridInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_zcornKey;
    caf::PdmField<QString> m_coordKey;
    caf::PdmField<QString> m_actnumKey;
};
