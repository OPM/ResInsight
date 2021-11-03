/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimCommandRouter.h"

#include "cafPdmField.h"

#include "cvfVector3.h"

#include <QString>
#include <QStringList>

#include <memory>

//==================================================================================================
///
//==================================================================================================
class RimcCommandRouter_extractSurfaces : public RimCommandRouterMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcCommandRouter_extractSurfaces( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle* execute() override;

    static bool readMinMaxLayerFromGridFile( const QString& gridFileName, int& minK, int& maxK );
    static std::pair<bool, QStringList> extractSurfaces( const QString&          gridModelFileName,
                                                         const std::vector<int>& layers,
                                                         int                     minI = -1,
                                                         int                     maxI = -1,
                                                         int                     minJ = -1,
                                                         int                     maxJ = -1 );

private:
    caf::PdmField<QString>          m_gridModelFilename;
    caf::PdmField<std::vector<int>> m_layers;
    caf::PdmField<int>              m_minimumI;
    caf::PdmField<int>              m_maximumI;
    caf::PdmField<int>              m_minimumJ;
    caf::PdmField<int>              m_maximumJ;
};
