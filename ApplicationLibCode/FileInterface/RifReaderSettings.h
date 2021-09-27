/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

//==================================================================================================
///
///
//==================================================================================================
class RifReaderSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    friend class RiaPreferences;

public:
    RifReaderSettings();

    caf::PdmField<bool>    importFaults;
    caf::PdmField<bool>    importNNCs;
    caf::PdmField<bool>    includeInactiveCellsInFaultGeometry;
    caf::PdmField<bool>    importAdvancedMswData;
    caf::PdmField<QString> includeFileAbsolutePathPrefix;
    caf::PdmField<bool>    useResultIndexFile;
    caf::PdmField<bool>    skipWellData;
    caf::PdmField<bool>    importSummaryData;

    static std::shared_ptr<RifReaderSettings> createGridOnlyReaderSettings();

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};
