/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicCaseAndFileExportSettingsUi.h"
#include "RicExportFractureCompletionsImpl.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

//==================================================================================================
///
///
//==================================================================================================
class RicExportCompletionDataSettingsUi : public RicCaseAndFileExportSettingsUi
{
    CAF_PDM_HEADER_INIT;

public:
    // Exported in .proto file. Do not change without changing .proto
    enum class ExportSplit
    {
        UNIFIED_FILE,
        SPLIT_ON_WELL,
    };
    using ExportSplitType = caf::AppEnum<ExportSplit>;

    // Exported in .proto file. Do not change without changing .proto
    enum class CompdatExport
    {
        TRANSMISSIBILITIES,
        WPIMULT_AND_DEFAULT_CONNECTION_FACTORS,
    };
    using CompdatExportType = caf::AppEnum<CompdatExport>;

    using TransScalingWBHPSource = caf::AppEnum<RicExportFractureCompletionsImpl::PressureDepletionWBHPSource>;

    RicExportCompletionDataSettingsUi();

    caf::PdmField<int> timeStep;

    caf::PdmField<ExportSplitType>   fileSplit;
    caf::PdmField<CompdatExportType> compdatExport;

    caf::PdmField<bool>                   performTransScaling;
    caf::PdmField<int>                    transScalingTimeStep;
    caf::PdmField<TransScalingWBHPSource> transScalingWBHPSource;
    caf::PdmField<double>                 transScalingWBHP;

    caf::PdmField<bool> includeMsw;
    caf::PdmField<bool> useLateralNTG;
    caf::PdmField<bool> includePerforations;
    caf::PdmField<bool> includeFishbones;
    caf::PdmField<bool> excludeMainBoreForFishbones;

    caf::PdmField<bool> includeFractures;

    void enableIncludeMsw();

    void showFractureInUi( bool enable );
    void showPerforationsInUi( bool enable );
    void showFishbonesInUi( bool enable );

    void setExportDataSourceAsComment( bool enable );
    bool exportDataSourceAsComment() const;

    void setExportWelspec( bool enable );
    bool exportWelspec() const;

    void setExportCompletionWelspecAfterMainBore( bool enable );
    bool exportCompletionWelspecAfterMainBore() const;

    void    setCustomFileName( const QString& fileName );
    QString customFileName() const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    std::map<int, std::vector<std::pair<QString, QString>>> generateWellProductionStartStrings();

private:
    caf::PdmField<bool>    m_exportDataSourceAsComment;
    caf::PdmField<bool>    m_exportWelspec;
    caf::PdmField<bool>    m_completionWelspecAfterMainBore;
    caf::PdmField<bool>    m_useCustomFileName;
    caf::PdmField<QString> m_customFileName;

    bool m_fracturesEnabled;
    bool m_perforationsEnabled;
    bool m_fishbonesEnabled;
};
