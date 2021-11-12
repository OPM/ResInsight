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
class RigHistogramData;

class Layer
{
public:
    Layer( double topDepth, double bottomDepth )
        : m_topDepth( topDepth )
        , m_bottomDepth( bottomDepth ){};

    double topDepth() const { return m_topDepth; };
    double bottomDepth() const { return m_bottomDepth; };
    double centerDepth() const { return ( m_bottomDepth + m_topDepth ) / 2.0; };
    double thickness() const { return m_topDepth - m_bottomDepth; };

private:
    double m_topDepth;
    double m_bottomDepth;
};

//==================================================================================================
///
///
//==================================================================================================
class RimEnsembleFractureStatistics : public RimNamedObject
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

    enum class MeshAlignmentType
    {
        PERFORATION_DEPTH,
        MESH_DEPTH
    };

    enum class MeshType
    {
        ADAPTIVE,
        UNIFORM,
        NAIVE
    };

    enum class MeanType
    {
        HARMONIC,
        ARITHMETIC,
        GEOMETRIC,
        MINIMUM
    };

    enum class AdaptiveNumLayersType
    {
        MINIMUM,
        MAXIMUM,
        AVERAGE,
        USER_DEFINED
    };

    RimEnsembleFractureStatistics();
    ~RimEnsembleFractureStatistics() override;
    void addFilePath( const QString& filePath );
    void loadAndUpdateData();

    std::vector<cvf::ref<RigStimPlanFractureDefinition>> readFractureDefinitions() const;

    static std::vector<cvf::cref<RigFractureGrid>>
        createFractureGrids( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                             RiaDefines::EclipseUnitSystem                               unitSystem,
                             const QString&                                              resultName,
                             MeshAlignmentType                                           meshAlignmentType );

    bool excludeZeroWidthFractures() const;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    QString generateFilePathsTable();

    std::vector<QString> computeStatistics();
    static double computeDepthOfWellPathAtFracture( cvf::ref<RigStimPlanFractureDefinition> stimPlanFractureDefinitionData );
    static bool isCoordinateInsideFractureCell( double x, double y, const RigFractureCell& cell );

    std::pair<std::vector<cvf::ref<RigStimPlanFractureDefinition>>, std::vector<QString>>
        readFractureDefinitions( const std::vector<caf::FilePath>& filePaths,
                                 RiaDefines::EclipseUnitSystem     unitSystem ) const;

    static std::set<std::pair<QString, QString>>
        findAllResultNames( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions );

    std::tuple<double, double, double, double>
        findSamplingIntervals( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                               std::vector<double>&                                        gridXs,
                               std::vector<double>&                                        gridYs ) const;

    void generateUniformMesh( double               minX,
                              double               maxX,
                              double               minY,
                              double               maxY,
                              std::vector<double>& gridXs,
                              std::vector<double>& gridYs ) const;

    void generateNaiveMesh( double                                                      minX,
                            double                                                      maxX,
                            double                                                      minY,
                            double                                                      maxY,
                            const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                            std::vector<double>&                                        gridXs,
                            std::vector<double>&                                        gridYs ) const;

    void generateAdaptiveMesh( double                                                      minX,
                               double                                                      maxX,
                               double                                                      minY,
                               double                                                      maxY,
                               const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                               std::vector<double>&                                        gridXs,
                               std::vector<double>&                                        gridYs ) const;

    int getTargetNumberOfLayers( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions ) const;

    static std::tuple<double, double, double, double>
        findMaxGridExtents( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                            MeshAlignmentType                                           meshAlignmentType );

    void computeMeanThicknessPerLayer( const std::vector<Layer>& layers,
                                       int                       targetNumLayers,
                                       double                    minY,
                                       double                    binSize,
                                       std::vector<double>&      means,
                                       std::vector<double>&      baseDepth ) const;

    static void generateAllLayers( const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions,
                                   std::vector<Layer>&                                         layers,
                                   MeshAlignmentType                                           meshAlignmentType );

    static void sampleAllGrids( const std::vector<cvf::cref<RigFractureGrid>>& fractureGrids,
                                const std::vector<double>&                     samplesX,
                                const std::vector<double>&                     samplesY,
                                std::vector<std::vector<double>>&              samples,
                                std::shared_ptr<RigSlice2D>                    areaGrid,
                                std::shared_ptr<RigSlice2D>                    distanceGrid );

    static std::shared_ptr<RigSlice2D> setCellsToFillTargetArea( const RigSlice2D& grid,
                                                                 const RigSlice2D& occurrenceGrid,
                                                                 const RigSlice2D& areaGrid,
                                                                 const RigSlice2D& distanceGrid,
                                                                 double            targetArea );

    static void generateStatisticsGrids(
        const std::vector<std::vector<double>>&                                               samples,
        size_t                                                                                numSamplesX,
        size_t                                                                                numSamplesY,
        size_t                                                                                numGrids,
        std::map<RimEnsembleFractureStatistics::StatisticsType, std::shared_ptr<RigSlice2D>>& statisticsGrids,
        const std::vector<caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>>&       statisticsTypes,
        const RigHistogramData&                                                               areaHistogram,
        std::shared_ptr<RigSlice2D>                                                           areaGrid,
        std::shared_ptr<RigSlice2D>                                                           distanceGrid );

    static bool writeStatisticsToCsv( const QString& filePath, const RigSlice2D& samples );

    static double linearSampling( double minValue, double maxValue, int numSamples, std::vector<double>& samples );

    QString generateStatisticsTable(
        const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& stimPlanFractureDefinitions ) const;

    caf::PdmField<std::vector<caf::FilePath>>      m_filePaths;
    caf::PdmField<QString>                         m_filePathsTable;
    caf::PdmField<QString>                         m_statisticsTable;
    caf::PdmField<bool>                            m_excludeZeroWidthFractures;
    caf::PdmField<bool>                            m_computeStatistics;
    caf::PdmField<int>                             m_numSamplesX;
    caf::PdmField<int>                             m_numSamplesY;
    caf::PdmField<caf::AppEnum<MeshType>>          m_meshType;
    caf::PdmField<caf::AppEnum<MeshAlignmentType>> m_meshAlignmentType;

    caf::PdmField<caf::AppEnum<MeanType>>              m_adaptiveMeanType;
    caf::PdmField<caf::AppEnum<AdaptiveNumLayersType>> m_adaptiveNumLayersType;
    caf::PdmField<int>                                 m_adaptiveNumLayers;

    caf::PdmField<std::vector<caf::AppEnum<RimEnsembleFractureStatistics::StatisticsType>>> m_selectedStatisticsType;
};
