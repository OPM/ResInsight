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

class LgrNameFactory;
class RigMainGrid;
class RigGridBase;
class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class QFile;
class QTextStream;
class IjkBoundingBox;

//==================================================================================================
/// Candidate for refactoring
//==================================================================================================


//==================================================================================================
///
//==================================================================================================
class LgrInfo
{
public:
    LgrInfo(int id,
            const QString& name,
            const QString& associatedWellPathName,
            const caf::VecIjk& sizes,
            const caf::VecIjk& mainGridStartCell,
            const caf::VecIjk& mainGridEndCell)
        : id(id), name(name), associatedWellPathName(associatedWellPathName),
        sizes(sizes), mainGridStartCell(mainGridStartCell), mainGridEndCell(mainGridEndCell)
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
    QString             associatedWellPathName;
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
        : type(RigCompletionData::CT_UNDEFINED), name(""), wellPathName("") {}
    CompletionInfo(RigCompletionData::CompletionType type, const QString& name, const QString& wellPathName)
        : type(type), name(name), wellPathName(wellPathName) {}


    CompletionInfo(RigCompletionData::CompletionType type, const QString& name)
        : type(type)
        , name(name)
        , wellPathName(wellPathName)
    {
    }
    RigCompletionData::CompletionType type;
    QString name;
    QString wellPathName;

    bool isValid() const { return type != RigCompletionData::CT_UNDEFINED && !name.isEmpty() && !wellPathName.isEmpty(); }

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
            if (wellPathName == other.wellPathName) return name < other.name;
            return wellPathName < other.wellPathName;
        }
        return priority() < other.priority();
    }

    bool operator==(const CompletionInfo& other) const
    {
        return type == other.type && name == other.name && wellPathName == other.wellPathName;
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

    using Range = std::pair<size_t, size_t>;
    static Range initRange() { return std::make_pair(std::numeric_limits<size_t>::max(), 0); }

    static RicExportLgrUi* openDialog(const QString& dialogTitle,
                                      RimEclipseCase* defaultCase = nullptr,
                                      int defaultTimeStep = 0,
                                      bool hideExportFolderField = false);
    static bool openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile);
    static void exportLgrsForWellPaths(const QString& exportFolder,
                                      std::vector<RimWellPath*> wellPaths,
                                      RimEclipseCase* eclipseCase,
                                      size_t timeStep,
                                      caf::VecIjk lgrCellCounts,
                                      Lgr::SplitType splitType,
                                      const std::set<RigCompletionData::CompletionType>& completionTypes,
                                       QStringList* wellsIntersectingOtherLgrs);

    static void exportLgrs(const QString& exportFolder,
                           const QString& wellName,
                           const std::vector<LgrInfo>& lgrInfos);

    static std::vector<LgrInfo> buildLgrsForWellPaths(std::vector<RimWellPath*>    wellPaths,
                                                      RimEclipseCase*              eclipseCase,
                                                      size_t                       timeStep,
                                                      caf::VecIjk                  lgrCellCounts,
                                                      Lgr::SplitType               splitType,
                                                      const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                      QStringList*                 wellsIntersectingOtherLgrs);

    static std::vector<RimWellPath*> selectedWellPaths();

    static std::map<QString /*wellName*/, std::vector<LgrInfo>> createLgrInfoListForTemporaryLgrs(const RigMainGrid* mainGrid);

protected:
    bool isCommandEnabled() override;
    void onActionTriggered(bool isChecked) override;
    void setupActionLook(QAction* actionToSetup) override;

private:
    static void writeLgrs(QTextStream& stream, const std::vector<LgrInfo>& lgrInfos);

    static std::vector<LgrInfo> buildLgrsPerMainCell(int                                           firstLgrId,
                                                     RimEclipseCase*                               eclipseCase,
                                                     RimWellPath*                                  wellPath,
                                                     const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                     const caf::VecIjk&                            lgrSizes,
                                                     LgrNameFactory&                               lgrNameFactory);
    static std::vector<LgrInfo>
                   buildLgrsPerCompletion(int                                                                     firstLgrId,
                                          RimEclipseCase*                                                         eclipseCase,
                                          const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& completionInfo,
                                          const caf::VecIjk&                                                      lgrSizesPerMainGridCell,
                                          LgrNameFactory&                                                         lgrNameFactory);
    static LgrInfo buildLgr(int                                           lgrId,
                            const QString&                                lgrName,
                            RimEclipseCase*                               eclipseCase,
                            const QString&                                wellPathName,
                            const std::vector<RigCompletionDataGridCell>& intersectingCells,
                            const caf::VecIjk&                            lgrSizesPerMainGridCell);

    static LgrInfo buildLgr(int                                           lgrId,
                            const QString&                                lgrName,
                            RimEclipseCase*                               eclipseCase,
                            const QString&                                wellPathName,
                            const IjkBoundingBox&                         boundingBox,
                            const caf::VecIjk&                            lgrSizesPerMainGridCell);

    static std::vector<RigCompletionDataGridCell> cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
                                                                               const RimWellPath* wellPath,
                                                                               size_t timeStep,
                                                                               const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                                               bool* isIntersectingOtherLgrs);
    //static std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
    //    cellsIntersectingCompletions_PerCompletion_old(RimEclipseCase* eclipseCase,
    //                                               const RimWellPath* wellPath,
    //                                               size_t timeStep,
    //                                               const std::set<RigCompletionData::CompletionType>& completionTypes,
    //                                               bool* isIntersectingOtherLgrs);

    static std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
               cellsIntersectingCompletions_PerCompletion(RimEclipseCase*                                    eclipseCase,
                                                          const std::vector<RimWellPath*>                    wellPaths,
                                                          size_t                                             timeStep,
                                                          const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                          QStringList*                                       wellsIntersectingOtherLgrs);

    static int firstAvailableLgrId(const RigMainGrid* mainGrid);
    static const RigGridBase* hostGrid(const RigMainGrid* mainGrid, size_t reservoirCellIndex);
};
