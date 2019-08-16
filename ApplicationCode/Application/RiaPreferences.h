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

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaDefines.h"
#include "RiaFontCache.h"


#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    

#include <map>

class RifReaderSettings;

class RiaPreferences : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum SummaryRestartFilesImportMode { IMPORT, NOT_IMPORT, SEPARATE_CASES };
    typedef caf::AppEnum<SummaryRestartFilesImportMode> SummaryRestartFilesImportModeType;
    typedef RiaFontCache::FontSizeType FontSizeType;

    RiaPreferences(void);
    ~RiaPreferences(void) override;

    QStringList tabNames();

    const RifReaderSettings* readerSettings() const;

    // Debug settings
    bool appendClassNameToUiText() const;
    bool appendFieldKeywordToToolTipText() const;
    bool showTestToolbar() const;
    bool includeFractureDebugInfoFile() const;
    bool showProjectChangedDialog() const;
    QString holoLensExportFolder() const;

    std::map<RiaDefines::FontSettingType, RiaFontCache::FontSize> defaultFontSizes() const;

public: // Pdm Fields
    caf::PdmField<caf::AppEnum< RiaGuiApplication::RINavigationPolicy > > navigationPolicy;

    caf::PdmField<bool>     enableGrpcServer;
    caf::PdmField<int>      defaultGrpcPortNumber;

    caf::PdmField<QString>  scriptDirectories;
    caf::PdmField<QString>  scriptEditorExecutable;

    caf::PdmField<QString>  octaveExecutable;
    caf::PdmField<bool>     octaveShowHeaderInfoWhenExecutingScripts;
    
    caf::PdmField<QString>  pythonExecutable;

    caf::PdmField<QString>  ssihubAddress;

    caf::PdmField<caf::AppEnum<RiaDefines::MeshModeType>> defaultMeshModeType;

    caf::PdmField<int>          defaultScaleFactorZ;
    caf::PdmField<cvf::Color3f> defaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultFaultGridLineColors;
    caf::PdmField<cvf::Color3f> defaultViewerBackgroundColor;
    caf::PdmField<cvf::Color3f> defaultWellLabelColor;
    caf::PdmField<bool>         showLasCurveWithoutTvdWarning;

    caf::PdmField<FontSizeType> defaultSceneFontSize;
    caf::PdmField<FontSizeType> defaultWellLabelFontSize;
    caf::PdmField<FontSizeType> defaultAnnotationFontSize;
    caf::PdmField<FontSizeType> defaultPlotFontSize;
    
    caf::PdmField<bool>     showLegendBackground;

    caf::PdmField<bool>     useShaders;
    caf::PdmField<bool>     showHud;    

    caf::PdmField<QString>  lastUsedProjectFileName;

    caf::PdmField<bool>     autocomputeDepthRelatedProperties;
    caf::PdmField<bool>     loadAndShowSoil;

    caf::PdmField<bool>                                 summaryRestartFilesShowImportDialog;
    caf::PdmField<SummaryRestartFilesImportModeType>    summaryImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType>    gridImportMode;
    caf::PdmField<SummaryRestartFilesImportModeType>    summaryEnsembleImportMode;

    caf::PdmField<QString>  defaultSummaryCurvesTextFilter;

    caf::PdmField<bool>     holoLensDisableCertificateVerification;
    caf::PdmField<QString>  csvTextExportFieldSeparator;

protected:
    void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    void                            initAfterRead() override;

private:
    static QString tabNameGeneral();
    static QString tabNameEclipse();
    static QString tabNameEclipseSummary();
    static QString tabNameScripting();
    static QString tabNameExport();
    static QString tabNameSystem();

private:
    caf::PdmChildField<RifReaderSettings*> m_readerSettings;
    caf::PdmField<bool>                    m_appendClassNameToUiText;
    caf::PdmField<bool>                    m_appendFieldKeywordToToolTipText;

    caf::PdmField<bool>                    m_showProjectChangedDialog;

    caf::PdmField<bool>                    m_showTestToolbar;
    caf::PdmField<bool>                    m_includeFractureDebugInfoFile;
    caf::PdmField<QString>                 m_holoLensExportFolder;
};
