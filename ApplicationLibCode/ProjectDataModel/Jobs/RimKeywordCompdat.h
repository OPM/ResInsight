/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimEclipseCase;
class RimWellPath;

namespace Opm
{
class DeckKeyword;
} // namespace Opm

//==================================================================================================
///
///
//==================================================================================================
class RimKeywordCompdat : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimKeywordCompdat();
    ~RimKeywordCompdat() override;

    void setEclipseCase( RimEclipseCase* eclipseCase );
    void setWellPath( RimWellPath* wellPath );

    Opm::DeckKeyword keyword();

private:
    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;
    caf::PdmPtrField<RimWellPath*>    m_wellPath;
};
