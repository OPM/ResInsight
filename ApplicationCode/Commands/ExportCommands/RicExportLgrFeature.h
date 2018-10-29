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

#include "RigCompletionData.h"
#include "RigCompletionDataGridCell.h"

#include "RicExportLgrUi.h"

#include "cafCmdFeature.h"
#include "cafVecIjk.h"

#include <limits>
#include <memory>

class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class QFile;
class QTextStream;


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

    size_t cellCount() const
    {
        return sizes.i() * sizes.j() * sizes.k();
    }

    int                 id;
    QString             name;
    caf::VecIjk         sizes;

    caf::VecIjk         mainGridStartCell;
    caf::VecIjk         mainGridEndCell;
};

//==================================================================================================
///
//==================================================================================================
class CompletionInfo
{
public:
    CompletionInfo()
        : type(RigCompletionData::CT_UNDEFINED), name(""), number(-1) {}
    CompletionInfo(RigCompletionData::CompletionType type, QString name, int number)
        : type(type), name(name), number(number) {}

    RigCompletionData::CompletionType type;
    QString name;
    int number;

    bool isValid() const { return type != RigCompletionData::CT_UNDEFINED && !name.isEmpty() && number >= 0; }

    int priority() const
    {
        return type == RigCompletionData::FRACTURE ? 1 :
            type == RigCompletionData::FISHBONES ? 2 :
            type == RigCompletionData::PERFORATION ? 3 : 4;
    }

    // Sort by priority, then name, then number
    bool operator<(const CompletionInfo& other) const
    {
        if (priority() == other.priority())
        {
            if (name == other.name) return number < other.number;
            return name < other.name;
        }
        return priority() < other.priority();
    }

    bool operator==(const CompletionInfo& other) const
    {
        return type == other.type && name == other.name && number == other.number;
    }

    bool operator!=(const CompletionInfo& other) const
    {
        return !operator==(other);
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

    static RicExportLgrUi* openDialog(const QString& dialogTitle,
                                      RimEclipseCase* defaultCase = nullptr,
                                      int defaultTimeStep = 0,
                                      bool hideExportFolderField = false);
    static bool openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile);
    static void exportLgrsForWellPath(const QString& exportFolder,
                                      RimWellPath* wellPath,
                                      RimEclipseCase* eclipseCase,
                                      size_t timeStep,
                                      caf::VecIjk lgrCellCounts,
                                      RicExportLgrUi::SplitType splitType,
                                      const std::set<RigCompletionData::CompletionType>& completionTypes,
                                      bool* intersectingOtherLgrs);

    static std::vector<LgrInfo> buildLgrsForWellPath(RimWellPath*                 wellPath,
                                                     RimEclipseCase*              eclipseCase,
                                                     size_t                       timeStep,
                                                     caf::VecIjk                  lgrCellCounts,
                                                     RicExportLgrUi::SplitType    splitType,
                                                     const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                     bool* intersectingOtherLgrs);

    static std::vector<RimWellPath*> selectedWellPaths();

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

    static std::vector<RigCompletionDataGridCell> cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
                                                                               const RimWellPath* wellPath,
                                                                               size_t timeStep,
                                                                               const std::set<RigCompletionData::CompletionType>& completionTypes);
    static std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
        cellsIntersectingCompletions_PerCompletion(RimEclipseCase* eclipseCase,
                                                   const RimWellPath* wellPath,
                                                   size_t timeStep,
                                                   const std::set<RigCompletionData::CompletionType>& completionTypes);

    static bool containsAnyNonMainGridCells(const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& cellsPerCompletion);
    static bool containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells);
    static int firstAvailableLgrId(const RigMainGrid* mainGrid);
};
