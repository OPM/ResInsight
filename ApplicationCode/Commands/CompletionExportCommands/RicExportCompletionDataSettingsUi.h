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
    enum ExportSplit
    {
        UNIFIED_FILE,
        SPLIT_ON_WELL,
        SPLIT_ON_WELL_AND_COMPLETION_TYPE,
    };
    typedef caf::AppEnum<ExportSplit> ExportSplitType;

    // Exported in .proto file. Do not change without changing .proto
    enum CompdatExport
    {
        TRANSMISSIBILITIES,
        WPIMULT_AND_DEFAULT_CONNECTION_FACTORS,

#ifdef _DEBUG
        NO_COMPLETIONS
#endif
    };
    typedef caf::AppEnum<CompdatExport> CompdatExportType;

    // Exported in .proto file. Do not change without changing .proto
    enum CombinationMode
    {
        INDIVIDUALLY,
        COMBINED,
    };
    typedef caf::AppEnum<CombinationMode> CombinationModeType;

    typedef caf::AppEnum<RicExportFractureCompletionsImpl::PressureDepletionWBHPSource> TransScalingWBHPSource;

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
    void showForSimWells();
    void showForWellPath();

    void setCombinationMode( CombinationMode combinationMode );

    void showFractureInUi( bool enable );
    void showPerforationsInUi( bool enable );
    void showFishbonesInUi( bool enable );

    bool reportCompletionsTypesIndividually() const;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    std::map<int, std::vector<std::pair<QString, QString>>> generateWellProductionStartStrings();

private:
    caf::PdmField<CombinationModeType> m_reportCompletionTypesSeparately;

    bool m_displayForSimWell;
    bool m_fracturesEnabled;
    bool m_perforationsEnabled;
    bool m_fishbonesEnabled;
};
