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

#include "ExportCommands/RicLgrSplitType.h"
#include "RigCompletionData.h"
#include "RigCompletionDataGridCell.h"

#include "cafCmdFeature.h"
#include "cafVecIjk.h"

#include <limits>
#include <memory>

class LgrInfo;
class RigMainGrid;
class RimEclipseCase;
class RimWellPath;
class RigGridBase;
class RigEclipseCaseData;

//==================================================================================================
///
//==================================================================================================
class RicCreateTemporaryLgrFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    void createLgrsForWellPaths( std::vector<RimWellPath*>                          wellPaths,
                                 RimEclipseCase*                                    eclipseCase,
                                 size_t                                             timeStep,
                                 caf::VecIjk                                        refinement,
                                 Lgr::SplitType                                     splitType,
                                 const std::set<RigCompletionData::CompletionType>& completionTypes,
                                 QStringList*                                       wellsIntersectingOtherLgrs );

    static RigGridBase* createLgr( const LgrInfo& lgrInfo, RigEclipseCaseData* caseData );
    static RigGridBase* createLgr( const LgrInfo& lgrInfo, RigMainGrid* mainGrid );

protected:
    bool isCommandEnabled() const override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
