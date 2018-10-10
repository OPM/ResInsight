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

#include "RigCompletionDataGridCell.h"
#include "cafCmdFeature.h"
#include <cafVecIjk.h>
#include <memory>
#include <limits>

class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class RicExportLgrUi;
class QFile;
class QTextStream;


//==================================================================================================
///
//==================================================================================================
class LgrInfo
{
public:
    LgrInfo(const QString&name,
            const caf::VecIjk& sizes,
            const caf::VecIjk& mainGridStartCell,
            const caf::VecIjk& mainGridEndCell)
        : name(name), sizes(sizes), mainGridStartCell(mainGridStartCell), mainGridEndCell(mainGridEndCell)
    {
    }

    QString             name;
    caf::VecIjk         sizes;
    std::vector<double> values;

    caf::VecIjk         mainGridStartCell;
    caf::VecIjk         mainGridEndCell;
};

//==================================================================================================
///
//==================================================================================================
class RicExportLgrFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    typedef std::pair<size_t, size_t> Range;
    static Range initRange() { return std::make_pair(std::numeric_limits<size_t>::max(), 0); }

    static RicExportLgrUi* openDialog();
    static bool openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile);
    static void exportLgr(QTextStream& stream, const std::vector<LgrInfo>& lgrInfos);

    static std::vector<LgrInfo> buildOneLgrPerMainCell(RimEclipseCase* eclipseCase,
                                                       const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                       const caf::VecIjk& lgrSizes);
    static std::vector<LgrInfo> buildSingleLgr(RimEclipseCase* eclipseCase,
                                               const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                               const caf::VecIjk& lgrSizes);

    static std::vector<RigCompletionDataGridCell> cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
                                                                              const RimWellPath* wellPath,
                                                                               size_t timeStep);

protected:
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;

private:
    static std::vector<RimWellPath*> selectedWellPaths();
    static bool containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells);
};
