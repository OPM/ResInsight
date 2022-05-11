/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "RiaDefines.h"

class RimEclipseCase;

class RimEclipseResultAddress : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseResultAddress();
    ~RimEclipseResultAddress() override;

    void    setResultName( const QString& resultName );
    QString resultName() const;

    void                      setResultType( RiaDefines::ResultCatType val );
    RiaDefines::ResultCatType resultType() const;

    void            setEclipseCase( RimEclipseCase* eclipseCase );
    RimEclipseCase* eclipseCase() const;

private:
    caf::PdmField<QString>                                 m_resultName;
    caf::PdmField<caf::AppEnum<RiaDefines::ResultCatType>> m_resultType;
    caf::PdmPtrField<RimEclipseCase*>                      m_eclipseCase;
};
