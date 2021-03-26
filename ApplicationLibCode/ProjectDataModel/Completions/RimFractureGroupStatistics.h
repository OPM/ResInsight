/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -     Equinor ASA
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

#include "RimNamedObject.h"

#include "RigStimPlanFractureDefinition.h"

class RigFractureCell;
class RigSlice2D;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureGroupStatistics : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class StatisticsType
    {
        MEAN,
        MIN,
        MAX,
        P10,
        P50,
        P90,
        OCCURRENCE
    };

    RimFractureGroupStatistics();
    ~RimFractureGroupStatistics() override;
    void addFilePath( const QString& filePath );
    void loadAndUpdateData();

protected:
    void    defineEditorAttribute( const caf::PdmFieldHandle* field,
                                   QString                    uiConfigName,
                                   caf::PdmUiEditorAttribute* attribute ) override;
    QString generateFilePathsTable();

    void          computeStatistics();
    static double computeDepthOfWellPathAtFracture( cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData );
    static bool isCoordinateInsideFractureCell( double x, double y, const RigFractureCell& cell );

    std::vector<cvf::ref<RigStimPlanFractureDefinition>>
        readFractureDefinitions( const std::vector<caf::FilePath>& filePaths,
                                 RiaDefines::EclipseUnitSystem     unitSystem ) const;

    std::vector<cvf::cref<RigFractureGrid>>
        createFractureGrids( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                             RiaDefines::EclipseUnitSystem                               unitSystem,
                             const QString&                                              resultName );

    static std::set<std::pair<QString, QString>>
        findAllResultNames( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions );

    static std::tuple<double, double, double, double>
        findExtentsOfGrids( const std::vector<cvf::cref<RigFractureGrid>>& fractureGrids );

    static void sampleAllGrids( const std::vector<cvf::cref<RigFractureGrid>>& fractureGrids,
                                std::vector<std::vector<double>>&              samples,
                                double                                         minX,
                                double                                         maxX,
                                int                                            numSamplesX,
                                int                                            numSamplesY,
                                double                                         sampleDistanceX,
                                double                                         sampleDistanceY );

    static void generateStatisticsGrids(
        const std::vector<std::vector<double>>&                                            samples,
        int                                                                                numSamplesX,
        int                                                                                numSamplesY,
        std::map<RimFractureGroupStatistics::StatisticsType, std::shared_ptr<RigSlice2D>>& statisticsGrids );

    static bool writeStatisticsToCsv( const QString& filePath, const RigSlice2D& samples );

    caf::PdmField<std::vector<caf::FilePath>> m_filePaths;
    caf::PdmField<QString>                    m_filePathsTable;
};
