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

#include "cafPdmField.h"
#include "cafAppEnum.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RicExportCompletionDataSettingsUi : public RicCaseAndFileExportSettingsUi
{
    CAF_PDM_HEADER_INIT;
public:
    enum ExportSplit {
        UNIFIED_FILE,
        SPLIT_ON_WELL,
        SPLIT_ON_WELL_AND_COMPLETION_TYPE,
    };
    typedef caf::AppEnum<ExportSplit> ExportSplitType;

    enum WellSelection {
        ALL_WELLS,
        CHECKED_WELLS,
        SELECTED_WELLS,
    };
    typedef caf::AppEnum<WellSelection> WellSelectionType;

    enum CompdatExport {
        TRANSMISSIBILITIES,
        WPIMULT_AND_DEFAULT_CONNECTION_FACTORS,
    };
    typedef caf::AppEnum<CompdatExport> CompdatExportType;


    RicExportCompletionDataSettingsUi();
    RicExportCompletionDataSettingsUi(bool onlyWellPathCollectionSelected);


    caf::PdmField<ExportSplitType>          fileSplit;
    caf::PdmField<WellSelectionType>        wellSelection;
    caf::PdmField<CompdatExportType>        compdatExport;


    caf::PdmField<bool>                     includePerforations;
    caf::PdmField<bool>                     includeFishbones;

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    caf::PdmField<bool>                     includeFractures;
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    caf::PdmField<bool>                     excludeMainBoreForFishbones;

    caf::PdmField<int>                      timeStep;

    void                                    showForSimWells();
    void                                    showForWellPath();

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected:
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    bool                                    m_onlyWellPathCollectionSelected;
    bool                                    m_displayForSimWell;
};
