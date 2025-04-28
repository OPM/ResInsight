/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimEclipseCase.h"

#include "cafPdmObject.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RimCornerPointCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimCornerPointCase();
    ~RimCornerPointCase() override;

    bool openEclipseGridFile() override;

    bool importAsciiInputProperties( const QStringList& fileNames ) override;

    QString locationOnDisc() const override;

    static std::pair<RimCornerPointCase*, QString> createFromCoordinatesArray( int                       nx,
                                                                               int                       ny,
                                                                               int                       nz,
                                                                               const std::vector<float>& coordsv,
                                                                               const std::vector<float>& zcornsv,
                                                                               const std::vector<float>& actnumsv );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    static void buildGrid( RigEclipseCaseData*       eclipseCase,
                           const int                 nx,
                           const int                 ny,
                           const int                 nz,
                           const std::vector<float>& cornerLines,
                           const std::vector<float>& zCorners,
                           const std::vector<float>& active );

    static std::array<cvf::Vec3d, 8> getCorners( const RigMainGrid&        grid,
                                                 const std::vector<float>& cornerLines,
                                                 const std::vector<float>& zcorn,
                                                 size_t                    cellIdx,
                                                 const cvf::Vec3d&         offset,
                                                 const cvf::Vec3d&         scale );
};
