/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfCommandObject.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

//==================================================================================================
//
//
//
//==================================================================================================
class RicfExportWellPathCompletions : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfExportWellPathCompletions();

    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<int>                  m_caseId;
    caf::PdmField<int>                  m_timeStep;
    caf::PdmField<std::vector<QString>> m_wellPathNames;

    caf::PdmField<RicExportCompletionDataSettingsUi::ExportSplitType>     m_fileSplit;
    caf::PdmField<RicExportCompletionDataSettingsUi::CompdatExportType>   m_compdatExport;
    caf::PdmField<RicExportCompletionDataSettingsUi::CombinationModeType> m_combinationMode;

    caf::PdmField<bool>                                                      m_performTransScaling;
    caf::PdmField<int>                                                       m_transScalingTimeStep;
    caf::PdmField<RicExportCompletionDataSettingsUi::TransScalingWBHPSource> m_transScalingInitialWBHP;
    caf::PdmField<double>                                                    m_transScalingWBHP;

    caf::PdmField<bool> m_includeMsw;
    caf::PdmField<bool> m_useLateralNTG;
    caf::PdmField<bool> m_includePerforations;
    caf::PdmField<bool> m_includeFishbones;
    caf::PdmField<bool> m_includeFractures;
    caf::PdmField<bool> m_excludeMainBoreForFishbones;

    // This is handeled by RicfCommandFileExecutor::exportDataSouceAsComment()
    // caf::PdmField<bool>    m_exportDataSourceAsComment;

    caf::PdmField<bool>    m_exportWelspec;
    caf::PdmField<bool>    m_completionWelspecAfterMainBore;
    caf::PdmField<QString> m_customFileName;
};
