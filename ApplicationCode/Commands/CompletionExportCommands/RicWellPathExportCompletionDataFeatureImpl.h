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

#include "RicExportCompletionDataSettingsUi.h"
#include "RicWellPathFractureReportItem.h"

#include <QFile>

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cvfVector2.h"

#include <vector>
#include <memory>

class RicMswCompletion;
class RigCell;
class RigEclipseCaseData;
class RigMainGrid;
class RimEclipseCase;
class RimFishbonesMultipleSubs;
class RimSimWellInView;
class RimPerforationInterval;
class RimWellPath;
class RimWellPathValve;
class RimWellPathFracture;
class RimNonDarcyPerforationParameters;
class RifEclipseDataTableFormatter;
class RigVirtualPerforationTransmissibilities;
class SubSegmentIntersectionInfo;

//==================================================================================================
/// 
//==================================================================================================
typedef std::shared_ptr<QFile> QFilePtr;

class TransmissibilityData
{
public:
    TransmissibilityData()
        : m_isValid(false)
        , m_effectiveH(0.0)
        , m_effectiveK(0.0)
        , m_connectionFactor(0.0)
        , m_kh(0.0)
    {
    }

    bool isValid() const
    {
        return m_isValid;
    }

    void setData(double effectiveH, double effectiveK, double connectionFactor, double kh)
    {
        m_isValid = true;

        m_effectiveH = effectiveH;
        m_effectiveK = effectiveK;
        m_connectionFactor = connectionFactor;
        m_kh = kh;
    }

    double effectiveH() const
    {
        return m_effectiveH;
    }

    double effectiveK() const
    {
        return m_effectiveK;
    }
    double connectionFactor() const
    {
        return m_connectionFactor;
    }
    double kh() const
    {
        return m_kh;
    }

private:
    bool m_isValid;
    double m_effectiveH;
    double m_effectiveK;
    double m_connectionFactor;
    double m_kh;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellPathExportCompletionDataFeatureImpl
{

public:

    static CellDirection                  calculateCellMainDirection(RimEclipseCase* eclipseCase,
                                                                     size_t globalCellIndex, 
                                                                     const cvf::Vec3d& lengthsInCell);

    static TransmissibilityData
        calculateTransmissibilityData(RimEclipseCase*    eclipseCase,
                                                 const RimWellPath* wellPath,
                                                 const cvf::Vec3d&  internalCellLengths,
                                                 double             skinFactor,
                                                 double             wellRadius,
                                                 size_t             globalCellIndex,
                                                 bool               useLateralNTG,
                                                 size_t             volumeScaleConstant       = 1,
                                                 CellDirection      directionForVolumeScaling = CellDirection::DIR_I);

    static double                         calculateDFactor(RimEclipseCase* eclipseCase,
                                                           double effectiveH,
                                                           size_t globalCellIndex,
                                                           const RimNonDarcyPerforationParameters* nonDarcyParameters,
                                                           const double effectivePermeability);

    static void                           exportCompletions(const std::vector<RimWellPath*>& wellPaths, 
                                                            const std::vector<RimSimWellInView*>& simWells, 
                                                            const RicExportCompletionDataSettingsUi& exportSettings);

    static std::vector<RigCompletionData> computeStaticCompletionsForWellPath(RimWellPath* wellPath,
                                                                              RimEclipseCase* eclipseCase);

    static std::vector<RigCompletionData> computeDynamicCompletionsForWellPath(RimWellPath* wellPath,
                                                                               RimEclipseCase* eclipseCase,
                                                                               size_t timeStepIndex);

    static std::vector<RigCompletionData> generatePerforationsCompdatValues(const RimWellPath* wellPath,
                                                                            const std::vector<const RimPerforationInterval*>& intervals,
                                                                            const RicExportCompletionDataSettingsUi& settings);

private:

    static double                         calculateTransmissibilityAsEclipseDoes(RimEclipseCase* eclipseCase,
                                                                                 double skinFactor,
                                                                                 double wellRadius,
                                                                                 size_t globalCellIndex,
                                                                                 CellDirection direction);
    
    static RigCompletionData              combineEclipseCellCompletions(const std::vector<RigCompletionData>& completions, 
                                                                        const RicExportCompletionDataSettingsUi& settings);

    static std::vector<RigCompletionData> mainGridCompletions(std::vector<RigCompletionData>& allCompletions);

    static std::map<QString, std::vector<RigCompletionData>> subGridsCompletions(std::vector<RigCompletionData>& allCompletions);

    static void                           exportWellPathFractureReport(RimEclipseCase*                                          sourceCase,
                                                                       QFilePtr                                                 exportFile,
                                                                       const std::vector<RicWellPathFractureReportItem>&        wellPathFractureReportItems);

    static void                           exportWelspecsToFile(RimEclipseCase* gridCase,
                                                               QFilePtr exportFile,
                                                               const std::vector<RigCompletionData>& completions);

    static void                           exportWelspeclToFile(RimEclipseCase* gridCase,
                                                               QFilePtr exportFile,
                                                               const std::map<QString, std::vector<RigCompletionData>>& completions);

    static void                           sortAndExportCompletionsToFile(RimEclipseCase* eclipseCase,
                                                                         const QString& exportFolder, 
                                                                         const QString& fileName, 
                                                                         std::vector<RigCompletionData>& completions, 
                                                                         const std::vector<RicWellPathFractureReportItem>& wellPathFractureReportItems,
                                                                         RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static void                           exportCompdatAndWpimultTables(RimEclipseCase* sourceCase,
                                                                        QFilePtr exportFile,
                                                                        const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid, 
                                                                        RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static void                           exportCompdatTableUsingFormatter(RifEclipseDataTableFormatter& formatter, 
                                                                           const QString& gridName, 
                                                                           const std::vector<RigCompletionData>& completionData);

    static void                           exportWpimultTableUsingFormatter(RifEclipseDataTableFormatter& formatter,
                                                                           const QString& gridName,
                                                                           const std::vector<RigCompletionData>& completionData);

    static void                           appendCompletionData(std::map<size_t, std::vector<RigCompletionData>>* completionData,
                                                               const std::vector<RigCompletionData>& data);

    static std::pair<double, cvf::Vec2i> wellPathUpperGridIntersectionIJ(const RimEclipseCase* gridCase,
                                                                         const RimWellPath*    wellPath,
                                                                         const QString&        gridName = "");

    static void                           exportCarfinForTemporaryLgrs(const RimEclipseCase* sourceCase, const QString& folder);

    static bool                           isCompletionWellPathEqual(const RigCompletionData& completion, const RimWellPath* wellPath);    
};
