/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafFilePath.h"

class RimVfpPlotCollection;

//--------------------------------------------------------------------------------------------------
/// RimVfpDeck parses a deck file (*.DATA) containing VFP data and creates a collection of VFP plots.
//--------------------------------------------------------------------------------------------------
class RimVfpDeck : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVfpDeck();

    void setFileName( const QString& filename );
    void loadDataAndUpdate();

private:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void updateObjectName();

private:
    caf::PdmField<caf::FilePath>              m_filePath;
    caf::PdmChildField<RimVfpPlotCollection*> m_vfpPlotCollection;
};
