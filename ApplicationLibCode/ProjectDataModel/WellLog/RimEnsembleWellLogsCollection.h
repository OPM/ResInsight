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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimEnsembleWellLogs;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleWellLogsCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleWellLogsCollection();
    ~RimEnsembleWellLogsCollection() override;

    std::vector<RimEnsembleWellLogs*> ensembleWellLogs() const;

    void addEnsembleWellLogs( RimEnsembleWellLogs* ensembleWellLogs );
    void loadDataAndUpdate();

private:
    caf::PdmChildArrayField<RimEnsembleWellLogs*> m_ensembleWellLogs;
};
