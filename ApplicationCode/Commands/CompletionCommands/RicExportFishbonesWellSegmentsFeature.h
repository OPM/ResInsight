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

#include "RifEclipseOutputTableFormatter.h"

#include "RimExportWellSegmentsSettings.h"

#include "RicWellPathExportCompletionDataFeature.h"

#include "cafCmdFeature.h"

class RimFishbonesCollection;
class RimFishbonesMultipleSubs;
class RimWellPath;

//==================================================================================================
/// 
//==================================================================================================
class RicExportFishbonesWellSegmentsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;
    virtual bool isCommandEnabled() override;

private:
    static RimFishbonesCollection*               selectedFishbonesCollection();
    static RimWellPath*                          selectedWellPath();

    static void                                  exportWellSegments(const RimWellPath* wellPath, const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs, const RimExportWellSegmentsSettings& settings);
    static void                                  generateWelsegsTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportWellSegmentsSettings& settings, const std::vector<WellSegmentLocation>& locations);
    static void                                  generateCompsegsTable(RifEclipseOutputTableFormatter& formatter, const RimWellPath* wellPath, const RimExportWellSegmentsSettings& settings, const std::vector<WellSegmentLocation>& locations);
};
