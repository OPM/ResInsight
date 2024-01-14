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

#include "RiaDateTimeDefines.h"
#include "RiaDefines.h"
#include "RiaFontCache.h"

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
class RiaPreferencesGeoMech;
class RiaPreferencesSystem;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaPreferences : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using FontSizeEnum        = RiaFontCache::FontSizeEnum;
    using PageSizeEnum        = caf::AppEnum<QPageSize::PageSizeId>;
    using PageOrientationEnum = caf::AppEnum<QPageLayout::Orientation>;
    using GridModelEnum       = caf::AppEnum<RiaDefines::GridModelReader>;

    bool enableFaultsByDefault() const;

public:
    RiaPreferences();
    ~RiaPreferences() override;

    static RiaPreferences* current();

    QStringList tabNames();

    const RifReaderSettings*    readerSettings() const;
    RiaDefines::GridModelReader gridModelReader() const;

    bool useUndoRedo() const;

    const QString& dateFormat() const;
    const QString& timeFormat() const;
    QString
        dateTimeFormat( RiaDefines::DateFormatComponents dateComponents = RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                        RiaDefines::TimeFormatComponents timeComponents = RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND ) const;

    int maxScriptFoldersDepth() const;
    int maxPlotTemplateFoldersDepth() const;

    QStringList plotTemplateFolders() const;
    void        appendPlotTemplateFolders( const QString& folder );
    QString     lastUsedPlotTemplateAbsolutePath() const;
    void        setLastUsedPlotTemplatePath( const QString& templatePath );
    bool        openExportedPdfInViewer() const;
    bool        useQtChartsAsDefaultPlotType() const;
    bool        writeEchoInGrdeclFiles() const;

    RiaDefines::ThemeEnum guiTheme() const;

    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> defaultFontSizes() const;

    void        writePreferencesToApplicationStore();
    QPageLayout defaultPageLayout() const;
    QMarginsF   margins() const;

    double surfaceImportResamplingDistance() const;

    QString        multiLateralWellNamePattern() const;
    static QString defaultMultiLateralWellNamePattern();

    QString gridCalculationExpressionFolder() const;
    QString summaryCalculationExpressionFolder() const;

    // 3D view
    RiaDefines::MeshModeType       defaultMeshModeType() const;
    RiaDefines::RINavigationPolicy navigationPolicy() const;
    double                         defaultScaleFactorZ() const;
    bool                           showLegendBackground() const;
    bool                           showInfoBox() const;
    bool                           showGridBox() const;

    // Script paths
    QString pythonExecutable() const;
    QString octaveExecutable() const;

    QString loggerFilename() const;
    int     loggerFlushInterval() const;

    RiaPreferencesGeoMech* geoMechPreferences() const;
    RiaPreferencesSummary* summaryPreferences() const;
    RiaPreferencesSystem*  systemPreferences() const;

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

    caf::PdmField<FontSizeEnum> defaultSceneFontSize;
    caf::PdmField<FontSizeEnum> defaultWellLabelFontSize;
    caf::PdmField<FontSizeEnum> defaultAnnotationFontSize;
    caf::PdmField<FontSizeEnum> defaultPlotFontSize;

    caf::PdmField<QString> lastUsedProjectFileName;

    caf::PdmField<bool> autocomputeDepthRelatedProperties;
    caf::PdmField<bool> loadAndShowSoil;

    caf::PdmField<bool>    holoLensDisableCertificateVerification;
    caf::PdmField<QString> csvTextExportFieldSeparator;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    static QString tabNameGeneral();
    static QString tabNameGrid();
    static QString tabNameSummary();
    static QString tabNameGeomech();
    static QString tabNamePlotting();
    static QString tabNameScripting();
    static QString tabNameSystem();
    static QString tabNameImportExport();

    static double defaultMarginSize( QPageSize::PageSizeId pageSizeId );

private:
    caf::PdmField<GridModelEnum>           m_gridModelReader;
    caf::PdmChildField<RifReaderSettings*> m_readerSettings;

    caf::PdmField<QString> m_dateFormat;
    caf::PdmField<QString> m_timeFormat;

    caf::PdmField<bool> m_useUndoRedo;

    caf::PdmField<caf::AppEnum<RiaDefines::ThemeEnum>> m_guiTheme;

    caf::PdmField<int> m_maxScriptFoldersDepth;

    caf::PdmField<PageSizeEnum>        m_pageSize;
    caf::PdmField<PageOrientationEnum> m_pageOrientation;
    caf::PdmField<double>              m_pageLeftMargin;
    caf::PdmField<double>              m_pageRightMargin;
    caf::PdmField<double>              m_pageTopMargin;
    caf::PdmField<double>              m_pageBottomMargin;
    caf::PdmField<bool>                m_openExportedPdfInViewer;
    caf::PdmField<bool>                m_writeEchoInGrdeclFiles;

    caf::PdmField<QString>       m_plotTemplateFolders;
    caf::PdmField<int>           m_maxPlotTemplateFoldersDepth;
    caf::PdmField<caf::FilePath> m_lastUsedPlotTemplate;
    caf::PdmField<bool>          m_useQtChartsPlotByDefault;

    caf::PdmField<caf::FilePath> m_gridCalculationExpressionFolder;
    caf::PdmField<caf::FilePath> m_summaryCalculationExpressionFolder;

    // Script paths
    caf::PdmField<QString> m_octaveExecutable;
    caf::PdmField<QString> m_pythonExecutable;

    // Logging
    caf::PdmField<std::pair<bool, QString>> m_loggerFilename;
    caf::PdmField<int>                      m_loggerFlushInterval;

    // Surface Import
    caf::PdmField<double> m_surfaceImportResamplingDistance;

    // Well Path Import
    caf::PdmField<QString> m_multiLateralWellPattern;

    // GeoMech things
    caf::PdmChildField<RiaPreferencesGeoMech*> m_geoMechPreferences;

    // Summary data
    caf::PdmChildField<RiaPreferencesSummary*> m_summaryPreferences;

    // System settings
    caf::PdmChildField<RiaPreferencesSystem*> m_systemPreferences;

    // 3d view
    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>>       m_defaultMeshModeType;
    caf::PdmField<caf::AppEnum<RiaDefines::RINavigationPolicy>> m_navigationPolicy;
    caf::PdmField<double>                                       m_defaultScaleFactorZ;
    caf::PdmField<bool>                                         m_showLegendBackground;
    caf::PdmField<bool>                                         m_enableFaultsByDefault;
    caf::PdmField<bool>                                         m_showInfoBox;
    caf::PdmField<bool>                                         m_showGridBox;

    QStringList m_tabNames;
};
