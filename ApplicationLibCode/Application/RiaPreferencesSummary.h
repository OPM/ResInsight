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
    using SummaryReaderModeType = caf::AppEnum<SummaryReaderMode>;

    enum class SummaryRestartFilesImportMode
    {
        IMPORT,
        NOT_IMPORT,
        SEPARATE_CASES
    };
    using SummaryRestartFilesImportModeType = caf::AppEnum<SummaryRestartFilesImportMode>;

    enum class SummaryHistoryCurveStyleMode
    {
        SYMBOLS,
        LINES,
        SYMBOLS_AND_LINES
    };
    using SummaryHistoryCurveStyleModeType = caf::AppEnum<SummaryHistoryCurveStyleMode>;

public:
    RiaPreferencesSummary();

    static RiaPreferencesSummary* current();

    SummaryReaderMode summaryDataReader() const;
    bool              useOptimizedSummaryDataFiles() const;
    bool              createOptimizedSummaryDataFiles() const;

    bool createH5SummaryDataFiles() const;
    bool checkH5SummaryDataTimeStamp() const;
    int  createH5SummaryDataThreadCount() const;

    void appendRestartFileGroup( caf::PdmUiOrdering& uiOrdering ) const;
    void appendItemsToPlottingGroup( caf::PdmUiOrdering& uiOrdering ) const;

    bool showSummaryTimeAsLongString() const;
    bool useMultipleThreadsWhenLoadingSummaryData() const;
    bool summaryRestartFilesShowImportDialog() const;

    SummaryRestartFilesImportMode summaryImportMode() const;
    SummaryRestartFilesImportMode gridImportMode() const;
    SummaryRestartFilesImportMode summaryEnsembleImportMode() const;
    QString                       defaultSummaryCurvesTextFilter() const;

    SummaryHistoryCurveStyleMode defaultSummaryHistoryCurveStyle() const;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

private:
    caf::PdmField<bool>                              m_summaryRestartFilesShowImportDialog;
    caf::PdmField<SummaryRestartFilesImportModeType> m_summaryImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> m_gridImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> m_summaryEnsembleImportMode;

    caf::PdmField<QString>                          m_defaultSummaryCurvesTextFilter;
    caf::PdmField<SummaryHistoryCurveStyleModeType> m_defaultSummaryHistoryCurveStyle;

    caf::PdmField<bool> m_showSummaryTimeAsLongString;
    caf::PdmField<bool> m_useMultipleThreadsWhenLoadingSummaryData;

    caf::PdmField<bool> m_createOptimizedSummaryDataFile;
    caf::PdmField<bool> m_useOptimizedSummaryDataFile;

    caf::PdmField<bool> m_createH5SummaryDataFile;
    caf::PdmField<bool> m_checkH5FileTimeStamp;
    caf::PdmField<int>  m_createH5SummaryFileThreadCount;

    caf::PdmField<SummaryReaderModeType> m_summaryReader;
};
