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

#include "RicExportLgrFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeature.h"
#include "RicExportLgrUi.h"

#include "RifEclipseDataTableFormatter.h"

#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFractureCollection.h"
#include "RimFishbonesCollection.h"
#include "RimPerforationCollection.h"

#include "RiuPlotMainWindow.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPathFracture.h"
#include "RimPerforationInterval.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QStringList>

#include <cafPdmUiPropertyViewDialog.h>
#include <cafSelectionManager.h>
#include <cafSelectionManagerTools.h>
#include <cafUtils.h>
#include <cafVecIjk.h>

#include <limits>
#include <array>
#include <set>
#include <algorithm>

CAF_CMD_SOURCE_INIT(RicExportLgrFeature, "RicExportLgrFeature");

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
/// Internal class
//--------------------------------------------------------------------------------------------------
class CellInfo
{
public:
    CellInfo(size_t globCellIndex)
        : globCellIndex(globCellIndex)
        , startMd(DOUBLE_INF)
        , endMd(DOUBLE_INF)
    {
    }
    CellInfo(size_t globCellIndex, double startMd, double endMd)
        : globCellIndex(globCellIndex)
        , startMd(startMd)
        , endMd(endMd)
    {
    }

    size_t globCellIndex;
    double startMd;
    double endMd;

    bool operator<(const CellInfo& other) const
    {
        return startMd < other.startMd;
    }
};

//--------------------------------------------------------------------------------------------------
/// Internal class
//--------------------------------------------------------------------------------------------------
class LgrNameFactory
{
public:
    LgrNameFactory();
    QString newName(RigCompletionData::CompletionType completionType);
    QString newName(const QString& baseName, int number);
    void    resetNumbering();

private:
    std::map<RigCompletionData::CompletionType, std::pair<QString, int>> m_counters;
};

//--------------------------------------------------------------------------------------------------
/// Internal class
//--------------------------------------------------------------------------------------------------
class IjkBoundingBox
{
    const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();
    enum Index {I, J, K};

public:
    IjkBoundingBox()
        : m_min({ MAX_SIZE_T, MAX_SIZE_T, MAX_SIZE_T}), m_max({MAX_SIZE_T, MAX_SIZE_T, MAX_SIZE_T}) {}

    IjkBoundingBox(const IjkBoundingBox& other)
        : m_min(other.m_min)
        , m_max(other.m_max)
    {
    }
    IjkBoundingBox(const caf::VecIjk& minCell, const caf::VecIjk& maxCell)
    {
        m_min[I] = minCell.i();
        m_min[J] = minCell.j();
        m_min[K] = minCell.k();
        m_max[I] = maxCell.i();
        m_max[J] = maxCell.j();
        m_max[K] = maxCell.k();
    }

    IjkBoundingBox& operator=(const IjkBoundingBox& other)
    {
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    bool isValid() const
    {
        return m_min[I] != MAX_SIZE_T && m_min[J] != MAX_SIZE_T && m_min[K] != MAX_SIZE_T &&
            m_max[I] != MAX_SIZE_T && m_max[J] != MAX_SIZE_T && m_max[K] != MAX_SIZE_T;
    }
    void addCell(size_t i, size_t j, size_t k)
    {
        if (!isValid())
        {
            m_min = m_max = { i, j, k };
        }
        else
        {
            if (i < m_min[I]) m_min[I] = i;
            if (j < m_min[J]) m_min[J] = j;
            if (k < m_min[K]) m_min[K] = k;
            if (i > m_max[I]) m_max[I] = i;
            if (j > m_max[J]) m_max[J] = j;
            if (k > m_max[K]) m_max[K] = k;
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    bool intersects(const IjkBoundingBox& box) const
    {
        CVF_TIGHT_ASSERT(isValid());
        CVF_TIGHT_ASSERT(box.isValid());

        if (m_max[I] < box.m_min[I] || m_min[I] > box.m_max[I]) return false;
        if (m_max[J] < box.m_min[J] || m_min[J] > box.m_max[J]) return false;
        if (m_max[K] < box.m_min[K] || m_min[K] > box.m_max[K]) return false;

        return true;
    }

    caf::VecIjk min() const
    {
        return caf::VecIjk(m_min[I], m_min[J], m_min[K]);
    }
    caf::VecIjk max() const
    {
        return caf::VecIjk(m_max[I], m_max[J], m_max[K]);
    }

private:
    std::array<size_t, 3> m_min;
    std::array<size_t, 3> m_max;
};

//--------------------------------------------------------------------------------------------------
// Internal function
//--------------------------------------------------------------------------------------------------
//int completionPriority(const RigCompletionData& completion)
//{
//    return completion.completionType() == RigCompletionData::FRACTURE ? 1 :
//        completion.completionType() == RigCompletionData::FISHBONES ? 2 :
//        completion.completionType() == RigCompletionData::PERFORATION ? 3 : 4;
//}

//--------------------------------------------------------------------------------------------------
// Internal function
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> filterCompletionsOnType(const std::vector<RigCompletionData>& completions,
                                                       const std::set<RigCompletionData::CompletionType>& includedCompletionTypes)
{
    std::vector<RigCompletionData> filtered;
    for (const auto& completion : completions)
    {
        if (includedCompletionTypes.count(completion.completionType()) > 0) filtered.push_back(completion);
    }
    return filtered;
}

//--------------------------------------------------------------------------------------------------
// Internal function
//--------------------------------------------------------------------------------------------------
QString completionName(const caf::PdmObject* object)
{
    auto perf = dynamic_cast<const RimPerforationInterval*>(object);
    auto frac = dynamic_cast<const RimFracture*>(object);
    auto fish = dynamic_cast<const RimFishbonesMultipleSubs*>(object);

    QString name;
    if (perf) name = perf->name();
    else if (frac) name = frac->name();
    else if (fish) name = fish->generatedName();
    return name;
}

//--------------------------------------------------------------------------------------------------
/// Internal function
/// Returns the completion having highest priority.
/// Pri: 1. Fractures, 2. Fishbones, 3. Perforation intervals
//--------------------------------------------------------------------------------------------------
//RigCompletionData findCompletionByPriority(const std::vector<RigCompletionData>& completions)
//{
//    std::vector<RigCompletionData> sorted = completions;
//
//    std::sort(sorted.begin(), sorted.end(),
//              [](const RigCompletionData& c1, const RigCompletionData& c2 )
//    { 
//        if (completionPriority(c1) == completionPriority(c2))
//        {
//            return completionName(c1.sourcePdmObject()) < completionName(c2.sourcePdmObject());
//        }
//        return completionPriority(c1) < completionPriority(c2);
//    });
//    return sorted.front();
//}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportLgrUi* RicExportLgrFeature::openDialog(const QString& dialogTitle,
                                                RimEclipseCase* defaultCase,
                                                int defaultTimeStep,
                                                bool hideExportFolderField)
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    QString startPath = app->lastUsedDialogDirectory("LGR_EXPORT_DIR");
    if (startPath.isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }

    RicExportLgrUi* featureUi = app->project()->dialogData()->exportLgrData();
    if (featureUi->exportFolder().isEmpty())
    {
        featureUi->setExportFolder(startPath);
    }

    if (!featureUi->caseToApply() && !defaultCase)
    {
        std::vector<RimCase*> cases;
        app->project()->allCases(cases);
        for (auto c : cases)
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
            if (eclipseCase != nullptr)
            {
                featureUi->setCase(eclipseCase);
                break;
            }
        }
    }
    if (defaultCase) featureUi->setCase(defaultCase);
    featureUi->setTimeStep(defaultTimeStep);
    featureUi->hideExportFolderField(hideExportFolderField);

    caf::PdmUiPropertyViewDialog propertyDialog(
        nullptr, featureUi, dialogTitle, "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(300, 320));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        app->setLastUsedDialogDirectory("LGR_EXPORT_DIR", featureUi->exportFolder());
        return featureUi;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportLgrFeature::openFileForExport(const QString& folderName, const QString& fileName, QFile* exportFile)
{
    QDir exportFolder = QDir(folderName);
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath) RiaLogging::info("Created export folder " + folderName);
    }

    QString filePath = exportFolder.filePath(fileName);
    exportFile->setFileName(filePath);
    if (!exportFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        auto errorMessage = QString("Export Well Path: Could not open the file: %1").arg(filePath);
        RiaLogging::error(errorMessage);
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::writeLgrs(QTextStream& stream, const std::vector<LgrInfo>& lgrInfos)
{
    for (auto lgrInfo : lgrInfos)
    {
        {
            RifEclipseDataTableFormatter formatter(stream);
            formatter.comment(QString("LGR: ") + lgrInfo.name);
            formatter.keyword("CARFIN");
            formatter.header({RifEclipseOutputTableColumn("Name"),
                              RifEclipseOutputTableColumn("I1"),
                              RifEclipseOutputTableColumn("I2"),
                              RifEclipseOutputTableColumn("J1"),
                              RifEclipseOutputTableColumn("J2"),
                              RifEclipseOutputTableColumn("K1"),
                              RifEclipseOutputTableColumn("K2"),
                              RifEclipseOutputTableColumn("NX"),
                              RifEclipseOutputTableColumn("NY"),
                              RifEclipseOutputTableColumn("NZ")});

            formatter.add(lgrInfo.name);
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.i());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.i());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.j());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.j());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridStartCell.k());
            formatter.addOneBasedCellIndex(lgrInfo.mainGridEndCell.k());
            formatter.add(lgrInfo.sizes.i());
            formatter.add(lgrInfo.sizes.j());
            formatter.add(lgrInfo.sizes.k());
            formatter.rowCompleted();
            formatter.tableCompleted("", false);
        }

        {
            RifEclipseDataTableFormatter formatter(stream);
            formatter.keyword("ENDFIN");
            formatter.tableCompleted("", true);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::exportLgrsForWellPaths(const QString&            exportFolder,
                                                 std::vector<RimWellPath*> wellPaths,
                                                 RimEclipseCase*           eclipseCase,
                                                 size_t                    timeStep,
                                                 caf::VecIjk               lgrCellCounts,
                                                 Lgr::SplitType            splitType,
                                                 const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                 QStringList*              wellsIntersectingOtherLgrs)
{
    std::vector<LgrInfo> lgrs;

    lgrs = buildLgrsForWellPaths(wellPaths,
                                 eclipseCase,
                                 timeStep,
                                 lgrCellCounts,
                                 splitType,
                                 completionTypes,
                                 wellsIntersectingOtherLgrs);

    for (const auto& wellPath : wellPaths)
    {
        std::vector<LgrInfo> expLgrs;
        for (const auto& lgr : lgrs)
        {
            if (lgr.associatedWellPathName == wellPath->name())
                expLgrs.push_back(lgr);
        }

        if (!lgrs.empty())
        {
            exportLgrs(exportFolder, wellPath->name(), expLgrs);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::exportLgrs(const QString& exportFolder, const QString& wellName, const std::vector<LgrInfo>& lgrInfos)
{
    if (!lgrInfos.empty())
    {
        // Export
        QFile   file;
        QString fileName = caf::Utils::makeValidFileBasename(QString("LGR_%1").arg(wellName)) + ".dat";
        openFileForExport(exportFolder, fileName, &file);
        QTextStream stream(&file);
        stream.setRealNumberNotation(QTextStream::FixedNotation);
        stream.setRealNumberPrecision(2);
        writeLgrs(stream, lgrInfos);
        file.close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsForWellPaths(std::vector<RimWellPath*>    wellPaths,
                                                                RimEclipseCase*              eclipseCase,
                                                                size_t                       timeStep,
                                                                caf::VecIjk                  lgrCellCounts,
                                                                Lgr::SplitType               splitType,
                                                                const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                                QStringList*                 wellsIntersectingOtherLgrs)
{
    std::vector<LgrInfo> lgrs;
    LgrNameFactory lgrNameFactory;

    wellsIntersectingOtherLgrs->clear();

    bool                 isIntersectingOtherLgrs = false;

    int firstLgrId = firstAvailableLgrId(eclipseCase->mainGrid());

    if (splitType == Lgr::LGR_PER_CELL)
    {
        for (const auto& wellPath : wellPaths)
        {
            auto intersectingCells =
                cellsIntersectingCompletions(eclipseCase, wellPath, timeStep, completionTypes, &isIntersectingOtherLgrs);
            auto newLgrs = buildLgrsPerMainCell(firstLgrId + (int)lgrs.size(), eclipseCase, wellPath, intersectingCells, lgrCellCounts, lgrNameFactory);

            lgrs.insert(lgrs.end(), newLgrs.begin(), newLgrs.end());
            if (isIntersectingOtherLgrs) wellsIntersectingOtherLgrs->push_back(wellPath->name());
        }
    }
    else if (splitType == Lgr::LGR_PER_COMPLETION)
    {
        auto intersectingCells = cellsIntersectingCompletions_PerCompletion(
            eclipseCase, wellPaths, timeStep, completionTypes, wellsIntersectingOtherLgrs);

        auto newLgrs = buildLgrsPerCompletion(firstLgrId + (int)lgrs.size(), eclipseCase, intersectingCells, lgrCellCounts, lgrNameFactory);
        lgrs.insert(lgrs.end(), newLgrs.begin(), newLgrs.end());
    }
    else if (splitType == Lgr::LGR_PER_WELL)
    {
        for (const auto& wellPath : wellPaths)
        {
            int  lgrId = firstLgrId + (int)lgrs.size();
            auto lgrName = lgrNameFactory.newName("WELL", lgrId);

            auto intersectingCells =
                cellsIntersectingCompletions(eclipseCase, wellPath, timeStep, completionTypes, &isIntersectingOtherLgrs);
            lgrs.push_back(buildLgr(lgrId, lgrName, eclipseCase, wellPath->name(), intersectingCells, lgrCellCounts));

            if (isIntersectingOtherLgrs) wellsIntersectingOtherLgrs->push_back(wellPath->name());
        }
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerMainCell(int                                           firstLgrId,
                                                               RimEclipseCase*                               eclipseCase,
                                                               RimWellPath*                                  wellPath,
                                                               const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                                               const caf::VecIjk&                            lgrSizes,
                                                               LgrNameFactory&                               lgrNameFactory)
{
    std::vector<LgrInfo> lgrs;
    int lgrId = firstLgrId;
    for (const auto& intersectionCell : intersectingCells)
    {
        auto lgrName = lgrNameFactory.newName("", lgrId);
        lgrs.push_back(buildLgr(lgrId++, lgrName, eclipseCase, wellPath->name(), {intersectionCell}, lgrSizes));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<LgrInfo> RicExportLgrFeature::buildLgrsPerCompletion(
    int                                                                     firstLgrId,
    RimEclipseCase*                                                         eclipseCase,
    const std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>& completionInfo,
    const caf::VecIjk&                                                      lgrSizesPerMainGridCell,
    LgrNameFactory&                                                         lgrNameFactory)
{
    std::vector<LgrInfo> lgrs;

    std::vector<std::pair<CompletionInfo, IjkBoundingBox>> occupiedBbs;

    for (const auto& complInfo : completionInfo)
    {
        auto complCells = std::set<RigCompletionDataGridCell>(complInfo.second.begin(), complInfo.second.end());
        std::vector<RigCompletionDataGridCell> cellsUsedInBb;

        while (!complCells.empty())
        {
            IjkBoundingBox maxBb;

            for (const auto& cell : complCells)
            {
                auto candidateBb = maxBb;
                candidateBb.addCell(cell.localCellIndexI(), cell.localCellIndexJ(), cell.localCellIndexK());

                // Test bounding box
                bool intersectsExistingBb = false;
                for (const auto& bb : occupiedBbs)
                {
                    if (candidateBb.intersects(bb.second))
                    {
                        intersectsExistingBb = true;
                        break;
                    }
                }

                if (!intersectsExistingBb)
                {
                    maxBb = candidateBb;
                    cellsUsedInBb.push_back(cell);
                }
            }

            // If bounding box is invalid, all cells are already occupied
            if (!maxBb.isValid()) break;

            occupiedBbs.emplace_back(complInfo.first, maxBb);

            // Remove cells used in bounding box
            for (const auto& cell : cellsUsedInBb)
                complCells.erase(cell);
        }
    }

    int lgrId = firstLgrId;
    for (auto complInfo : occupiedBbs)
    {
        auto lgrName = lgrNameFactory.newName(complInfo.first.type);
        lgrs.push_back(buildLgr(lgrId++, lgrName, eclipseCase, complInfo.first.wellPathName, complInfo.second, lgrSizesPerMainGridCell));
    }
    return lgrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LgrInfo RicExportLgrFeature::buildLgr(int                                           lgrId,
                                      const QString&                                lgrName,
                                      RimEclipseCase*                               eclipseCase,
                                      const QString&                                wellPathName,
                                      const std::vector<RigCompletionDataGridCell>& intersectingCells,
                                      const caf::VecIjk&                            lgrSizesPerMainGridCell)
{
    // Find min and max IJK
    auto iRange = initRange();
    auto jRange = initRange();
    auto kRange = initRange();

    for (const auto& cell : intersectingCells)
    {
        iRange.first  = std::min(cell.localCellIndexI(), iRange.first);
        iRange.second = std::max(cell.localCellIndexI(), iRange.second);
        jRange.first  = std::min(cell.localCellIndexJ(), jRange.first);
        jRange.second = std::max(cell.localCellIndexJ(), jRange.second);
        kRange.first  = std::min(cell.localCellIndexK(), kRange.first);
        kRange.second = std::max(cell.localCellIndexK(), kRange.second);
    }

    caf::VecIjk mainGridStartCell(iRange.first, jRange.first, kRange.first);
    caf::VecIjk mainGridEndCell(iRange.second, jRange.second, kRange.second);

    IjkBoundingBox boundingBox(mainGridStartCell, mainGridEndCell);
    return buildLgr(lgrId, lgrName, eclipseCase, wellPathName, boundingBox, lgrSizesPerMainGridCell);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LgrInfo RicExportLgrFeature::buildLgr(int                   lgrId,
                                      const QString&        lgrName,
                                      RimEclipseCase*       eclipseCase,
                                      const QString&        wellPathName,
                                      const IjkBoundingBox& boundingBox,
                                      const caf::VecIjk&    lgrSizesPerMainGridCell)
{
    caf::VecIjk lgrSizes((boundingBox.max().i() - boundingBox.min().i() + 1) * lgrSizesPerMainGridCell.i(),
                         (boundingBox.max().j() - boundingBox.min().j() + 1) * lgrSizesPerMainGridCell.j(),
                         (boundingBox.max().k() - boundingBox.min().k() + 1) * lgrSizesPerMainGridCell.k());

    return LgrInfo(lgrId, lgrName, wellPathName, lgrSizes, boundingBox.min(), boundingBox.max());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionDataGridCell>
RicExportLgrFeature::cellsIntersectingCompletions(RimEclipseCase* eclipseCase,
                                                  const RimWellPath* wellPath,
                                                  size_t timeStep,
                                                  const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                  bool* isIntersectingOtherLgrs)
{
    std::vector<RigCompletionDataGridCell> cells;

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    *isIntersectingOtherLgrs = false;
    auto completions         = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (completions)
    {
        auto intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);

        for (auto intCell : intCells)
        {
            const RigGridBase* grid = hostGrid(mainGrid, intCell.first);
            if (grid != mainGrid)
            {
                *isIntersectingOtherLgrs = true;
                continue;
            }

            auto filteredCompletions = filterCompletionsOnType(intCell.second, completionTypes);

            if (filteredCompletions.empty()) continue;

            cells.push_back(RigCompletionDataGridCell(intCell.first, mainGrid));
        }
    }

    return cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionDataGridCell> cellsIntersectingCompletion(const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& allCells,
                                                                   caf::PdmObject* sourcePdmObject)
{
    std::vector<RigCompletionDataGridCell> cells;
    for (const auto& intInfo : allCells)
    {
        for (const auto& completion : intInfo.second)
        {
            if (completion.sourcePdmObject() == sourcePdmObject) cells.push_back(intInfo.first);
        }
    }
    return cells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>>
     createOrderedIntersectionList(const std::vector<WellPathCellIntersectionInfo>&                           allWellPathCells,
                                   const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& completionCells)
{
    // All cell indices intersecting a completion and lookup into map
    std::set<size_t>                            complCellIndices;
    std::map<size_t, RigCompletionDataGridCell> complCellLookup;
    std::set<CellInfo>                          cellsOnWellPath;
    std::vector<std::pair<bool, CellInfo>>      cellsNotOnWellPath;
    {
        for (const auto& complCell : completionCells)
        {
            complCellIndices.insert(complCell.first.globalCellIndex());
            complCellLookup.insert({complCell.first.globalCellIndex(), complCell.first});

            bool cellFoundOnWellPath = false;
            for (const auto& wellPathCell : allWellPathCells)
            {
                if (complCell.first.globalCellIndex() == wellPathCell.globCellIndex)
                {
                    cellsOnWellPath.insert(CellInfo(complCell.first.globalCellIndex(), wellPathCell.startMD, wellPathCell.endMD));
                    cellFoundOnWellPath = true;
                    break;
                }
            }

            if (!cellFoundOnWellPath)
            {
                cellsNotOnWellPath.emplace_back( true, CellInfo(complCell.first.globalCellIndex()) );
            }
        }
    }

    std::set<size_t> cellsTaken;
    std::vector<std::pair<RigCompletionDataGridCell, std::vector<RigCompletionData>>> result;

    // Walk along well path
    for (const auto& cellOnWellPath : cellsOnWellPath)
    {
        // Add cell on well path first
        auto complDataGridCell = complCellLookup.at(cellOnWellPath.globCellIndex);
        auto complDataList = completionCells.at(complDataGridCell);
        result.emplace_back(complDataGridCell, complDataList);

        // Check intersected completions in current cell
        RigCompletionData::CompletionType complTypes[] = { RigCompletionData::FRACTURE, RigCompletionData::FISHBONES, RigCompletionData::PERFORATION };

        for (auto complType : complTypes)
        {
            const caf::PdmObject* completion = nullptr;
            for (const auto& complData : complDataList)
            {
                if (complData.completionType() == complType)
                {
                    completion = complData.sourcePdmObject();
                    break;
                }
            }

            if (completion)
            {
                // Add all cells intersecting this completion
                for (auto& cellNotOnWellPath : cellsNotOnWellPath)
                {
                    if (!cellNotOnWellPath.first) continue;

                    auto complDataList2 = completionCells.at(complCellLookup.at(cellNotOnWellPath.second.globCellIndex));
                    auto itr = std::find_if(complDataList2.begin(), complDataList2.end(),
                                            [&completion](const RigCompletionData& cd) { return cd.sourcePdmObject() == completion; });

                    if (itr != complDataList2.end())
                    {
                        result.emplace_back( complCellLookup.at(cellNotOnWellPath.second.globCellIndex), complDataList2);
                        cellNotOnWellPath.first = false;
                    }
                }
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
//std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>>
//    RicExportLgrFeature::cellsIntersectingCompletions_PerCompletion_old(RimEclipseCase*    eclipseCase,
//                                                                    const RimWellPath* wellPath,
//                                                                    size_t             timeStep,
//                                                                    const std::set<RigCompletionData::CompletionType>& completionTypes,
//                                                                    bool* isIntersectingOtherLgrs)
//{
//    std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>> completionToCells;
//
//    *isIntersectingOtherLgrs = false;
//
//    auto wellPathGeometry = wellPath->wellPathGeometry();
//    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
//    if (wellPathGeometry && completions)
//    {
//        const auto& intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);
//        CompletionInfo lastCompletionInfo;
//
//        auto wpIntCells = RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(eclipseCase->eclipseCaseData(),
//                                                                                           wellPathGeometry->wellPathPoints(),
//                                                                                           wellPathGeometry->measureDepths());
//
//        auto wpComplCells = createOrderedIntersectionList(wpIntCells, intCells);
//
//        // This loop assumes that cells are ordered downwards along well path
//        for (auto intCell : wpComplCells)
//        {
//            if (!intCell.first.isMainGridCell())
//            {
//                *isIntersectingOtherLgrs = true;
//                continue;
//            }
//
//            auto filteredCompletions = filterCompletionsOnType(intCell.second, completionTypes);
//            if (filteredCompletions.empty()) continue;
//
//            auto completion = findCompletionByPriority(filteredCompletions);
//
//            QString        name = completionName(completion.sourcePdmObject());
//            CompletionInfo completionInfo(completion.completionType(), name, 0);
//
//            if (!lastCompletionInfo.isValid()) lastCompletionInfo = completionInfo;
//
//            if (completionInfo != lastCompletionInfo && completionToCells.count(completionInfo) > 0)
//            {
//                completionInfo.number++;
//            }
//            completionToCells[completionInfo].push_back(intCell.first);
//            lastCompletionInfo = completionInfo;
//        }
//    }
//    return completionToCells;
//}


template<typename T>
void appendVector(std::vector<T>& dest, const std::vector<T>& append)
{
    dest.insert(dest.end(), append.begin(), append.end());
}

void appendIntersectedCells(std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& dest,
                            const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& append)
{
    for (auto& intCell : append)
    {
        if (dest.count(intCell.first) == 0)
        {
            dest.insert(intCell);
        }
        else
        {
            appendVector(dest[intCell.first], intCell.second);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>> RicExportLgrFeature::cellsIntersectingCompletions_PerCompletion(
    RimEclipseCase*                                    eclipseCase,
    const std::vector<RimWellPath*>                    wellPaths,
    size_t                                             timeStep,
    const std::set<RigCompletionData::CompletionType>& completionTypes,
    QStringList*                                       wellsIntersectingOtherLgrs)
{
    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    std::map<CompletionInfo, std::vector<RigCompletionDataGridCell>> completionToCells;

    wellsIntersectingOtherLgrs->clear();

    auto completions      = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if (!completions) return completionToCells;

    for (const auto& wellPath : wellPaths)
    {
        bool isIntersectingOtherLgrs = false;
        const auto& intCells = completions->multipleCompletionsPerEclipseCell(wellPath, timeStep);

        for (const auto& intCell : intCells)
        {
            const RigGridBase* grid = hostGrid(mainGrid, intCell.first);
            if (grid != mainGrid)
            {
                isIntersectingOtherLgrs = true;
                continue;
            }

            for (const auto& completion : intCell.second)
            {
                auto complName = completionName(completion.sourcePdmObject());
                CompletionInfo ci(completion.completionType(), complName, completion.wellName());

                auto& item = completionToCells[ci];
                item.push_back(RigCompletionDataGridCell(intCell.first, mainGrid));
            }
        }

        if (isIntersectingOtherLgrs) wellsIntersectingOtherLgrs->push_back(wellPath->name());
    }

    return completionToCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportLgrFeature::isCommandEnabled()
{
    std::vector<RimWellPathCompletions*> completions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !completions.empty() || !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    if(wellPaths.empty()) return;

    QString dialogTitle = "Export LGR for Completions";

    RimEclipseCase* defaultEclipseCase = nullptr;
    int             defaultTimeStep    = 0;
    auto            activeView         = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeGridView());
    if (activeView)
    {
        defaultEclipseCase = activeView->eclipseCase();
        defaultTimeStep    = activeView->currentTimeStep();
    }

    auto dialogData = openDialog(dialogTitle, defaultEclipseCase, defaultTimeStep);
    if (dialogData)
    {
        auto   eclipseCase   = dialogData->caseToApply();
        auto   lgrCellCounts = dialogData->lgrCellCount();
        size_t timeStep      = dialogData->timeStep();

        QStringList wellsIntersectingOtherLgrs;
        exportLgrsForWellPaths(dialogData->exportFolder(),
                                wellPaths,
                                eclipseCase,
                                timeStep,
                                lgrCellCounts,
                                dialogData->splitType(),
                                dialogData->completionTypes(),
                                &wellsIntersectingOtherLgrs);

        if (!wellsIntersectingOtherLgrs.empty())
        {
            auto wellsList = wellsIntersectingOtherLgrs.join(", ");
            QMessageBox::warning(nullptr,
                                 "LGR cells intersected",
                                 "No export for some wells due to existing intersecting LGR(s). Affected wells: " + wellsList);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportLgrFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export LGR for completions");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportLgrFeature::selectedWellPaths()
{
    std::vector<RimWellPathCompletions*> selectedCompletions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();
    std::vector<RimWellPath*>            wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    for (auto completion : selectedCompletions)
    {
        RimWellPath* parentWellPath;
        completion->firstAncestorOrThisOfType(parentWellPath);

        if (parentWellPath) wellPaths.push_back(parentWellPath);
    }
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<caf::VecIjk, caf::VecIjk> mainGridCellBoundingBoxFromLgr(const RigGridBase* lgr)
{
    auto mainGrid = lgr->mainGrid();

    // Find min and max IJK
    auto iRange = RicExportLgrFeature::initRange();
    auto jRange = RicExportLgrFeature::initRange();
    auto kRange = RicExportLgrFeature::initRange();

    for (size_t c = 0; c < lgr->cellCount(); c++)
    {
        const auto& cell = lgr->cell(c);
        size_t mainGridCellIndex = cell.mainGridCellIndex();

        size_t i, j, k;
        mainGrid->ijkFromCellIndex(mainGridCellIndex, &i, &j, &k);

        iRange.first  = std::min(i, iRange.first);
        iRange.second = std::max(i, iRange.second);
        jRange.first  = std::min(j, jRange.first);
        jRange.second = std::max(j, jRange.second);
        kRange.first  = std::min(k, kRange.first);
        kRange.second = std::max(k, kRange.second);
    }

    caf::VecIjk mainGridStartCell(iRange.first, jRange.first, kRange.first);
    caf::VecIjk mainGridEndCell(iRange.second, jRange.second, kRange.second);
    return std::make_pair(mainGridStartCell, mainGridEndCell);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString /*wellName*/, std::vector<LgrInfo>> RicExportLgrFeature::createLgrInfoListForTemporaryLgrs(const RigMainGrid* mainGrid)
{
    std::map<QString, std::vector<LgrInfo>> lgrInfosPerWell;

    for (size_t i = 0; i < mainGrid->gridCount(); i++)
    {
        const auto grid = mainGrid->gridByIndex(i);
        if (!grid->isTempGrid()) continue;

        caf::VecIjk lgrSizes(grid->cellCountI(), grid->cellCountJ(), grid->cellCountK());
        std::pair<caf::VecIjk, caf::VecIjk> mainGridBoundingBox = mainGridCellBoundingBoxFromLgr(grid);

        QString wellName = QString::fromStdString(grid->associatedWellPathName());
        auto& item = lgrInfosPerWell[wellName];

        item.emplace_back(LgrInfo(grid->gridId(),
                                    QString::fromStdString(grid->gridName()),
                                    QString::fromStdString(grid->associatedWellPathName()),
                                    lgrSizes,
                                    mainGridBoundingBox.first,
                                    mainGridBoundingBox.second));
    }

    return lgrInfosPerWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicExportLgrFeature::firstAvailableLgrId(const RigMainGrid* mainGrid)
{
    int gridCount  = (int)mainGrid->gridCount();
    int lastUsedId = 0;
    for (int i = 0; i < gridCount; i++)
    {
        lastUsedId = std::max(lastUsedId, mainGrid->gridByIndex(i)->gridId());
    }
    return lastUsedId + 1;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigGridBase* RicExportLgrFeature::hostGrid(const RigMainGrid* mainGrid, size_t reservoirCellIndex)
{
    size_t dummy = 0;
    
    return mainGrid->gridAndGridLocalIdxFromGlobalCellIdx(reservoirCellIndex, &dummy);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
LgrNameFactory::LgrNameFactory()
{
    m_counters = {
        {RigCompletionData::FRACTURE, {"FRAC", 1}},
        {RigCompletionData::FISHBONES, {"FB", 1}},
        {RigCompletionData::PERFORATION, {"PERF", 1}}
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString LgrNameFactory::newName(RigCompletionData::CompletionType completionType)
{
    switch (completionType)
    {
    case RigCompletionData::FRACTURE:
    case RigCompletionData::FISHBONES:
    case RigCompletionData::PERFORATION:
    {
        auto& counter = m_counters[completionType];
        QString name = counter.first + "_" + QString::number(counter.second);
        counter.second++;
        return name;
    }
    default:
        return "Unknown type";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString LgrNameFactory::newName(const QString& baseName, int number)
{
    QString lgrName;
    if(baseName.isEmpty()) lgrName = "LGR";
    lgrName += baseName + "_" + QString::number(number);
    return lgrName.replace(" ", "_");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void LgrNameFactory::resetNumbering()
{
    for (auto& counter : m_counters)
    {
        counter.second.second = 1;
    }
}
