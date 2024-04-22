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
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RiaDefines.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include <QString>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferencesSummary : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class SummaryReaderMode
    {
        RESDATA,
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

    enum class DefaultSummaryPlotType
    {
        NONE,
        DATA_VECTORS,
        PLOT_TEMPLATES
    };
    using DefaultSummaryPlotEnum = caf::AppEnum<DefaultSummaryPlotType>;

    using ColumnCountEnum = caf::AppEnum<RiaDefines::ColumnCount>;
    using RowCountEnum    = caf::AppEnum<RiaDefines::RowCount>;

public:
    RiaPreferencesSummary();

    static RiaPreferencesSummary* current();

    SummaryReaderMode summaryDataReader() const;
    bool              useEnhancedSummaryDataFiles() const;
    bool              createEnhancedSummaryDataFiles() const;

    bool createH5SummaryDataFiles() const;
    int  createH5SummaryDataThreadCount() const;

    DefaultSummaryPlotType defaultSummaryPlotType() const;
    std::vector<QString>   defaultSummaryPlotTemplates( bool returnEnsembleTemplates ) const;
    bool                   isDefaultSummaryPlotTemplate( QString filename ) const;
    void                   addToDefaultPlotTemplates( QString filename );
    void                   removeFromDefaultPlotTemplates( QString filename );

    void appendRestartFileGroup( caf::PdmUiOrdering& uiOrdering ) const;
    void appendItemsToPlottingGroup( caf::PdmUiOrdering& uiOrdering ) const;

    bool showSummaryTimeAsLongString() const;
    bool useMultipleThreadsWhenLoadingSummaryData() const;
    bool summaryRestartFilesShowImportDialog() const;

    SummaryRestartFilesImportMode summaryImportMode() const;
    SummaryRestartFilesImportMode gridImportMode() const;
    SummaryRestartFilesImportMode summaryEnsembleImportMode() const;
    QString                       defaultSummaryCurvesTextFilter() const;
    bool                          colorCurvesByPhase() const;
    bool                          appendHistoryVectors() const;

    QString crossPlotAddressCombinations() const;

    SummaryHistoryCurveStyleMode defaultSummaryHistoryCurveStyle() const;

    RiaDefines::ColumnCount defaultMultiPlotColumnCount() const;
    RiaDefines::RowCount    defaultMultiPlotRowCount() const;

    cvf::Color3f historyCurveContrastColor() const;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<DefaultSummaryPlotEnum> m_defaultSummaryPlot;
    caf::PdmField<bool>                   m_selectDefaultTemplates;
    caf::PdmField<std::vector<QString>>   m_selectedDefaultTemplates;

    caf::PdmField<bool>                              m_summaryRestartFilesShowImportDialog;
    caf::PdmField<SummaryRestartFilesImportModeType> m_summaryImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> m_gridImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> m_summaryEnsembleImportMode;

    caf::PdmField<QString>                          m_defaultSummaryCurvesTextFilter;
    caf::PdmField<QString>                          m_crossPlotAddressCombinations;
    caf::PdmField<SummaryHistoryCurveStyleModeType> m_defaultSummaryHistoryCurveStyle;
    caf::PdmField<bool>                             m_curveColorByPhase;
    caf::PdmField<bool>                             m_appendHistoryVectors;

    caf::PdmField<bool> m_showSummaryTimeAsLongString;
    caf::PdmField<bool> m_useMultipleThreadsWhenLoadingSummaryCases;

    caf::PdmField<bool> m_createEnhancedSummaryDataFile;
    caf::PdmField<bool> m_useEnhancedSummaryDataFile;

    caf::PdmField<bool> m_createH5SummaryDataFile;
    caf::PdmField<int>  m_createH5SummaryFileThreadCount;

    caf::PdmField<SummaryReaderModeType> m_summaryReader;

    caf::PdmField<ColumnCountEnum> m_defaultColumnCount;
    caf::PdmField<RowCountEnum>    m_defaultRowsPerPage;

    caf::PdmField<cvf::Color3f> m_historyCurveContrastColor;
};
