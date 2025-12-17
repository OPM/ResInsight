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
#include <string>

class RiaPreferencesSummary;
class RiaPreferencesGeoMech;
class RiaPreferencesSystem;
class RiaPreferencesOsdu;
class RiaPreferencesGrid;
class RiaPreferencesSumo;
class RiaPreferencesOpm;
class RiaPreferencesOpenTelemetry;

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

    bool enableFaultsByDefault() const;

public:
    RiaPreferences();
    ~RiaPreferences() override;

    static RiaPreferences* current();

    QStringList tabNames();

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

    // Octave
    QString octaveExecutable() const;
    bool    octaveShowHeaderInfoWhenExecutingScripts() const;
    QString octavePortNumber() const;

    bool storeBackupOfProjectFiles() const;

    RiaPreferencesGeoMech*       geoMechPreferences() const;
    RiaPreferencesSummary*       summaryPreferences() const;
    RiaPreferencesSystem*        systemPreferences() const;
    RiaPreferencesOsdu*          osduPreferences() const;
    RiaPreferencesSumo*          sumoPreferences() const;
    RiaPreferencesGrid*          gridPreferences() const;
    RiaPreferencesOpm*           opmPreferences() const;
    RiaPreferencesOpenTelemetry* openTelemetryPreferences() const;

    void importPreferenceValuesFromFile( const QString& fileName );
    void exportPreferenceValuesToFile( const QString& fileName );

public:
    caf::PdmField<bool> enableGrpcServer;
    caf::PdmField<int>  defaultGrpcPortNumber;

    caf::PdmField<QString> scriptDirectories;
    caf::PdmField<QString> scriptEditorExecutable;
    caf::PdmField<bool>    showPythonDebugInfo;

    caf::PdmField<cvf::Color3f> defaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultFaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultViewerBackgroundColor;
    caf::PdmField<cvf::Color3f> defaultWellLabelColor;

    caf::PdmField<FontSizeEnum> defaultSceneFontSize;
    caf::PdmField<FontSizeEnum> defaultWellLabelFontSize;
    caf::PdmField<FontSizeEnum> defaultAnnotationFontSize;
    caf::PdmField<FontSizeEnum> defaultPlotFontSize;

    caf::PdmField<QString> lastUsedProjectFileName;

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
    static QString tabNameOpmFlow();

    static double defaultMarginSize( QPageSize::PageSizeId pageSizeId );

private:
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

    caf::PdmField<caf::FilePath> m_gridCalculationExpressionFolder;
    caf::PdmField<caf::FilePath> m_summaryCalculationExpressionFolder;

    // Octave
    caf::PdmField<QString>                  m_octaveExecutable;
    caf::PdmField<bool>                     m_octaveShowHeaderInfoWhenExecutingScripts;
    caf::PdmField<std::pair<bool, QString>> m_octavePortNumber;

    // Python
    caf::PdmField<QString> m_pythonExecutable;

    caf::PdmField<bool> m_storeBackupOfProjectFile;

    // Surface Import
    caf::PdmField<double> m_surfaceImportResamplingDistance;

    // Well Path Import
    caf::PdmField<QString> m_multiLateralWellPattern;

    // Grid import
    caf::PdmChildField<RiaPreferencesGrid*> m_gridPreferences;

    // GeoMech things
    caf::PdmChildField<RiaPreferencesGeoMech*> m_geoMechPreferences;

    // OPM settings
    caf::PdmChildField<RiaPreferencesOpm*> m_opmPreferences;

    // Summary data
    caf::PdmChildField<RiaPreferencesSummary*> m_summaryPreferences;

    // System settings
    caf::PdmChildField<RiaPreferencesSystem*> m_systemPreferences;

    // Osdu settings
    caf::PdmChildField<RiaPreferencesOsdu*> m_osduPreferences;
    caf::PdmField<bool>                     m_deleteOsduToken;

    // sumo settings
    caf::PdmChildField<RiaPreferencesSumo*> m_sumoPreferences;
    caf::PdmField<bool>                     m_deleteSumoToken;

    // OpenTelemetry settings
    caf::PdmChildField<RiaPreferencesOpenTelemetry*> m_openTelemetryPreferences;

    // 3d view
    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>>       m_defaultMeshModeType;
    caf::PdmField<caf::AppEnum<RiaDefines::RINavigationPolicy>> m_navigationPolicy;
    caf::PdmField<double>                                       m_defaultScaleFactorZ;
    caf::PdmField<bool>                                         m_showLegendBackground;
    caf::PdmField<bool>                                         m_enableFaultsByDefault;
    caf::PdmField<bool>                                         m_showInfoBox;
    caf::PdmField<bool>                                         m_showGridBox;

    // Load and save preferences
    caf::PdmField<bool> m_exportPreferences;
    caf::PdmField<bool> m_importPreferences;

    QStringList m_tabNames;
};
