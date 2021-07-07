/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"

class RimFileSurface;
class RimSurface;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleSurface : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleSurface();
    void removeFileSurface( RimFileSurface* fileSurface );
    void addFileSurface( RimFileSurface* fileSurface );

    std::vector<RimFileSurface*> fileSurfaces() const;

    std::vector<RimSurface*> surfaces() const;

    void loadDataAndUpdate();

private:
    caf::PdmChildArrayField<RimFileSurface*> m_fileSurfaces;
};
