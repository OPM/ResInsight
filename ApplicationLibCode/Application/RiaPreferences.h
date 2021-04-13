/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
//  Copyright (C) 2011-2018 Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaDefines.h"
#include "RiaFontCache.h"
#include "RiaGuiApplication.h"
#include "RiaQDateTimeTools.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include <QPageLayout>
#include <QPageSize>
#include <QStringList>

#include <map>

class RifReaderSettings;
class RiaPreferencesSummary;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferences : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using DateFormatComponents = RiaQDateTimeTools::DateFormatComponents;
    using TimeFormatComponents = RiaQDateTimeTools::TimeFormatComponents;

    enum class SummaryRestartFilesImportMode
    {
        IMPORT,
        NOT_IMPORT,
        SEPARATE_CASES
    };
    typedef caf::AppEnum<SummaryRestartFilesImportMode> SummaryRestartFilesImportModeType;
    typedef RiaFontCache::FontSizeEnum                  FontSizeEnum;

    enum class SummaryHistoryCurveStyleMode
    {
        SYMBOLS,
        LINES,
        SYMBOLS_AND_LINES
    };
    typedef caf::AppEnum<SummaryHistoryCurveStyleMode> SummaryHistoryCurveStyleModeType;

    typedef caf::AppEnum<QPageSize::PageSizeId>    PageSizeEnum;
    typedef caf::AppEnum<QPageLayout::Orientation> PageOrientationEnum;

    bool enableFaultsByDefault() const;

public:
    RiaPreferences( void );
    ~RiaPreferences( void ) override;

    static RiaPreferences* current();

    QStringList tabNames();

    const RifReaderSettings* readerSettings() const;

    // Debug settings
    bool    appendClassNameToUiText() const;
    bool    appendFieldKeywordToToolTipText() const;
    bool    showViewIdInProjectTree() const;
    bool    showTestToolbar() const;
    bool    includeFractureDebugInfoFile() const;
    bool    showProjectChangedDialog() const;
    QString holoLensExportFolder() const;
    bool    useShaders() const;
    bool    show3dInformation() const;
    QString gtestFilter() const;
    bool    useUndoRedo() const;

    const QString& dateFormat() const;
    const QString& timeFormat() const;
    QString dateTimeFormat( DateFormatComponents dateComponents = DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                            TimeFormatComponents timeComponents = TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND ) const;

    bool        searchPlotTemplateFoldersRecursively() const;
    QStringList plotTemplateFolders() const;
    void        appendPlotTemplateFolders( const QString& folder );
    QString     defaultPlotTemplateAbsolutePath() const;
    void        setDefaultPlotTemplatePath( const QString& templatePath );
    bool        showSummaryTimeAsLongString() const;
    bool        useMultipleThreadsWhenReadingSummaryData() const;
    bool        showProgressBar() const;
    bool        openExportedPdfInViewer() const;

    RiaDefines::ThemeEnum guiTheme() const;

    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> defaultFontSizes() const;

    void        writePreferencesToApplicationStore();
    QPageLayout defaultPageLayout() const;
    QMarginsF   margins() const;

    double surfaceImportResamplingDistance() const;

    QString        multiLateralWellNamePattern() const;
    static QString defaultMultiLateralWellNamePattern();

    // 3D view
    RiaDefines::MeshModeType              defaultMeshModeType() const;
    RiaGuiApplication::RINavigationPolicy navigationPolicy() const;
    int                                   defaultScaleFactorZ() const;
    bool                                  showLegendBackground() const;
    bool                                  showInfoBox() const;
    bool                                  showGridBox() const;

    // Script paths
    QString pythonExecutable() const;
    QString octaveExecutable() const;

    RiaPreferencesSummary* summaryPreferences() const;

public:
    caf::PdmField<bool> enableGrpcServer;
    caf::PdmField<int>  defaultGrpcPortNumber;

    caf::PdmField<QString> scriptDirectories;
    caf::PdmField<QString> scriptEditorExecutable;
    caf::PdmField<bool>    octaveShowHeaderInfoWhenExecutingScripts;
    caf::PdmField<bool>    showPythonDebugInfo;

    caf::PdmField<QString> ssihubAddress;

    caf::PdmField<cvf::Color3f> defaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultFaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultViewerBackgroundColor;
    caf::PdmField<cvf::Color3f> defaultWellLabelColor;
    caf::PdmField<bool>         showLasCurveWithoutTvdWarning;

    caf::PdmField<FontSizeEnum> defaultSceneFontSize;
    caf::PdmField<FontSizeEnum> defaultWellLabelFontSize;
    caf::PdmField<FontSizeEnum> defaultAnnotationFontSize;
    caf::PdmField<FontSizeEnum> defaultPlotFontSize;

    caf::PdmField<QString> lastUsedProjectFileName;

    caf::PdmField<bool> autocomputeDepthRelatedProperties;
    caf::PdmField<bool> loadAndShowSoil;

    caf::PdmField<bool>                              summaryRestartFilesShowImportDialog;
    caf::PdmField<SummaryRestartFilesImportModeType> summaryImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> gridImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType> summaryEnsembleImportMode;

    caf::PdmField<QString>                          defaultSummaryCurvesTextFilter;
    caf::PdmField<SummaryHistoryCurveStyleModeType> defaultSummaryHistoryCurveStyle;

    caf::PdmField<bool>    holoLensDisableCertificateVerification;
    caf::PdmField<QString> csvTextExportFieldSeparator;

protected:
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          initAfterRead() override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    static QString tabNameGeneral();
    static QString tabNameEclipse();
    static QString tabNamePlotting();
    static QString tabNameScripting();
    static QString tabNameExport();
    static QString tabNameSystem();
    static QString tabNameImport();

    static double defaultMarginSize( QPageSize::PageSizeId pageSizeId );

private:
    caf::PdmChildField<RifReaderSettings*> m_readerSettings;

    caf::PdmField<bool> m_appendClassNameToUiText;
    caf::PdmField<bool> m_appendFieldKeywordToToolTipText;
    caf::PdmField<bool> m_showViewIdInProjectTree;
    caf::PdmField<bool> m_useShaders;
    caf::PdmField<bool> m_showHud;

    caf::PdmField<bool> m_showProjectChangedDialog;

    caf::PdmField<bool>    m_showTestToolbar;
    caf::PdmField<bool>    m_includeFractureDebugInfoFile;
    caf::PdmField<QString> m_holoLensExportFolder;
    caf::PdmField<QString> m_dateFormat;
    caf::PdmField<QString> m_timeFormat;
    caf::PdmField<bool>    m_showSummaryTimeAsLongString;
    caf::PdmField<bool>    m_useMultipleThreadsWhenLoadingSummaryData;
    caf::PdmField<bool>    m_showProgressBar;
    caf::PdmField<QString> m_gtestFilter;
    caf::PdmField<bool>    m_useUndoRedo;

    caf::PdmField<caf::AppEnum<RiaDefines::ThemeEnum>> m_guiTheme;

    caf::PdmField<PageSizeEnum>        m_pageSize;
    caf::PdmField<PageOrientationEnum> m_pageOrientation;
    caf::PdmField<double>              m_pageLeftMargin;
    caf::PdmField<double>              m_pageRightMargin;
    caf::PdmField<double>              m_pageTopMargin;
    caf::PdmField<double>              m_pageBottomMargin;
    caf::PdmField<bool>                m_openExportedPdfInViewer;

    caf::PdmField<QString>       m_plotTemplateFolders;
    caf::PdmField<bool>          m_searchPlotTemplateFoldersRecursively;
    caf::PdmField<caf::FilePath> m_defaultPlotTemplate;

    // Script paths
    caf::PdmField<QString> m_octaveExecutable;
    caf::PdmField<QString> m_pythonExecutable;

    // Surface Import
    caf::PdmField<double> m_surfaceImportResamplingDistance;

    // Well Path Import
    caf::PdmField<QString> m_multiLateralWellPattern;

    // Summary data
    caf::PdmChildField<RiaPreferencesSummary*> m_summaryPreferences;

    // 3d view
    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>>              m_defaultMeshModeType;
    caf::PdmField<caf::AppEnum<RiaGuiApplication::RINavigationPolicy>> m_navigationPolicy;
    caf::PdmField<int>                                                 m_defaultScaleFactorZ;
    caf::PdmField<bool>                                                m_showLegendBackground;
    caf::PdmField<bool>                                                m_enableFaultsByDefault;
    caf::PdmField<bool>                                                m_showInfoBox;
    caf::PdmField<bool>                                                m_showGridBox;

    QStringList m_tabNames;
};
