/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiaDefines.h"
#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseResultAddress;
class RimEclipseCase;

class RimEclipseResultAddressCollection : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseResultAddressCollection();
    ~RimEclipseResultAddressCollection() override;

    void setResultType( RiaDefines::ResultCatType val );

    void addAddress( const QString& resultName, RiaDefines::ResultCatType resultType, RimEclipseCase* eclipseCase );

    bool isEmpty() const;

    void clear();

private:
    caf::PdmChildArrayField<RimEclipseResultAddress*>      m_adresses;
    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>> m_resultType;
    caf::PdmPtrField<RimEclipseCase*>                      m_eclipseCase;
};
