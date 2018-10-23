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
#include "RigCompletionData.h"

#include "RicExportLgrUi.h"

#include "cafCmdFeature.h"
#include <cafVecIjk.h>
#include <memory>
#include <limits>

class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class QFile;
class QTextStream;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class CreateLgrException
{
public:
    CreateLgrException(const QString& message)
        : message(message)
    {
    }
    QString message;
};

//==================================================================================================
///
//==================================================================================================
class LgrInfo
{
public:
    LgrInfo(int id,
            const QString&name,
            const caf::VecIjk& sizes,
            const caf::VecIjk& mainGridStartCell,
            const caf::VecIjk& mainGridEndCell)
        : id(id), name(name), sizes(sizes), mainGridStartCell(mainGridStartCell), mainGridEndCell(mainGridEndCell)
    {
    }

    caf::VecIjk sizesPerMainGridCell() const
    {
        return caf::VecIjk(sizes.i() / (mainGridEndCell.i() - mainGridStartCell.i() + 1),
                           sizes.j() / (mainGridEndCell.j() - mainGridStartCell.j() + 1),
                           sizes.k() / (mainGridEndCell.k() - mainGridStartCell.k() + 1));
    }

    int cellCount() const
    {
        return (int)(sizes.i() * sizes.j() * sizes.k());
    }
    int cellCountPerMainGridCell() const
    {
        auto s = sizesPerMainGridCell();
        return (int)(s.i() * s.j() * s.k());
    }

    int                 id;
    QString             name;
    caf::VecIjk         sizes;
    //std::vector<double> values;

    caf::VecIjk         mainGridStartCell;
    caf::VecIjk         mainGridEndCell;
};

//==================================================================================================
///
//==================================================================================================
class CompletionInfo
{
public:
    CompletionInfo(RigCompletionData::CompletionType type, QString name)
        : type(type), name(name) {}

    RigCompletionData::CompletionType type;
    QString name;

    bool operator<(const CompletionInfo& other) const
    {
        return type < other.type || name < other.name;
    }
};

//==================================================================================================
///
//==================================================================================================
class RicExportLgrFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    typedef std::pair<size_t, size_t> Range;
    static Range initRange() { return std::make_pair(std::numeric_limits<size_t>::max(), 0); }

    static RicExportLgrUi* openDialog(const QString& dialogTitle, RimEclipseCase* defaultCase = nullptr, int defaultTimeStep = 0);
    static bool openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile);
    static void exportLgrsForWellPath(const QString& exportFolder,
                                      RimWellPath* wellPath,
                                      RimEclipseCase* eclipseCase,
                                      size_t timeStep,
                                      caf::VecIjk lgrCellCounts,
                                      RicExportLgrUi::SplitType splitType);

    static std::vector<LgrInfo> buildLgrsForWellPath(RimWellPath*                 wellPath,
                                                     RimEclipseCase*              eclipseCase,
                                                     size_t                       timeStep,
                                                     caf::VecIjk                  lgrCellCounts,
                                                     RicExportLgrUi::SplitType    splitType);

protected:
    bool isCommandEnabled() override;
    void onActionTriggered(bool isChecked) override;
    void setupActionLook(QAction* actionToSetup) override;

private:
    static void exportLgrs(QTextStream& stream, const std::vector<LgrInfo>& lgrInfos);

    static std::vector<LgrInfo> buildLgrsPerMainCell(RimEclipseCase*                               eclipseCase,
                                                     const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                     const caf::VecIjk&                            lgrSizes);
    static std::vector<LgrInfo>
                   buildLgrsPerCompletion(RimEclipseCase*                                                         eclipseCase,
                                          const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& intersectingCells,
                                          const caf::VecIjk&                                                      lgrSizesPerMainGridCell);
    static LgrInfo buildLgr(int                                           lgrId,
                            RimEclipseCase*                               eclipseCase,
                            const std::vector<RigCompletionDataGridCell>& intersectingCells,
                            const caf::VecIjk&                            lgrSizesPerMainGridCell);

    static std::vector<RigCompletionDataGridCell>
        cellsIntersectingCompletions(RimEclipseCase* eclipseCase, const RimWellPath* wellPath, size_t timeStep);
    static std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
                                     cellsIntersectingCompletions_PerCompletion(RimEclipseCase* eclipseCase, const RimWellPath* wellPath, size_t timeStep);
    static std::vector<RimWellPath*> selectedWellPaths();
    static bool containsAnyNonMainGridCells(const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& cellsPerCompletion);
    static bool containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells);
    static int firstAvailableLgrId(const RigMainGrid* mainGrid);
};
