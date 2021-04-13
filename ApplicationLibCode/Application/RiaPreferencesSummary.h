/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesSummary : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SummaryReaderMode
    {
        LIBECL,
        OPM_COMMON,
        HDF5_OPM_COMMON
    };
    typedef caf::AppEnum<SummaryReaderMode> SummaryReaderModeType;

public:
    RiaPreferencesSummary();

    SummaryReaderMode summaryDataReader() const;
    bool              useOptimizedSummaryDataFiles() const;
    bool              createOptimizedSummaryDataFiles() const;

    bool createH5SummaryDataFiles() const;
    int  createH5SummaryDataThreadCount() const;

protected:
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    caf::PdmField<bool> m_createOptimizedSummaryDataFile;
    caf::PdmField<bool> m_useOptimizedSummaryDataFile;

    caf::PdmField<bool> m_createH5SummaryDataFile;
    caf::PdmField<int>  m_createH5SummaryFileThreadCount;

    caf::PdmField<SummaryReaderModeType> m_summaryReader;
};
