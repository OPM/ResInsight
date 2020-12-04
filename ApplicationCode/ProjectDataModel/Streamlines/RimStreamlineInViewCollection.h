/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaDefines.h"
#include "cafPdmField.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

#include "cvfBoundingBox.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"

#include <list>
#include <set>
#include <vector>

class RimStreamline;
class RimEclipseCase;
class RigTracer;
class RigCell;
class RigResultAccessor;
class RigGridBase;

class RimRegularLegendConfig;

class RiuViewer;

class RimStreamlineInViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class VisualizationMode
    {
        ANIMATION = 0,
        MANUAL,
        VECTORS
    };
    using VisualizationModeEnum = caf::AppEnum<VisualizationMode>;

public:
    RimStreamlineInViewCollection();
    ~RimStreamlineInViewCollection() override;

    void            setEclipseCase( RimEclipseCase* reservoir );
    RimEclipseCase* eclipseCase() const;

    RiaDefines::PhaseType phase() const;

    VisualizationMode visualizationMode() const;
    double            distanceBetweenTracerPoints() const;
    size_t            animationSpeed() const;
    size_t            animationIndex() const;
    double            scaleFactor() const;
    size_t            tracerLength() const;
    size_t            injectionDeltaTime() const;

    void updateStreamlines();

    const std::list<RigTracer>& tracers();

    const RimRegularLegendConfig* legendConfig() const;
    void                          mappingRange( double& min, double& max ) const;
    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );
    void updateFromCurrentTimeStep();

protected:
    caf::PdmFieldHandle* objectToggleField() override;

private:
    void generateTracer( RigCell cell, double direction, QString simWellName );
    void loadDataIfMissing( RiaDefines::PhaseType phase, int timeIdx );

    void generateStartPositions( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, std::list<cvf::Vec3d>& positions );

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    cvf::ref<RigResultAccessor>
        getDataAccessor( cvf::StructGridInterface::FaceType faceIdx, RiaDefines::PhaseType phase, int timeIdx );

    bool setupDataAccessors( RiaDefines::PhaseType phase, int timeIdx );

    QString gridResultNameFromPhase( RiaDefines::PhaseType phase, cvf::StructGridInterface::FaceType faceIdx ) const;

    double faceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RigGridBase* grid ) const;
    double posFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx ) const;
    double negFaceValue( RigCell cell, cvf::StructGridInterface::FaceType faceIdx, RigGridBase* grid ) const;

    cvf::Vec3d cellDirection( RigCell cell, RigGridBase* grid ) const;

    std::vector<double> faceValues( RigCell cell, RigGridBase* grid );

    RigCell* findNeighborCell( RigCell cell, RigGridBase* grid, cvf::StructGridInterface::FaceType face ) const;
    std::vector<size_t> findNeighborCellIndexes( RigCell* cell, RigGridBase* grid ) const;

    cvf::BoundingBox cellBoundingBox( RigCell* cell, RigGridBase* grid ) const;

    void outputSummary() const;

    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<QString>                             m_collectionName;
    caf::PdmField<double>                              m_lengthThreshold;
    caf::PdmField<double>                              m_flowThreshold;
    caf::PdmField<double>                              m_resolution;
    caf::PdmField<double>                              m_maxDays;
    caf::PdmField<int>                                 m_density;
    caf::PdmPointer<RimEclipseCase>                    m_eclipseCase;
    caf::PdmChildArrayField<RimStreamline*>            m_streamlines;
    caf::PdmField<caf::AppEnum<RiaDefines::PhaseType>> m_phase;
    caf::PdmField<VisualizationModeEnum>               m_visualizationMode;
    caf::PdmField<double>                              m_distanceBetweenTracerPoints;
    caf::PdmField<size_t>                              m_animationSpeed;
    caf::PdmField<size_t>                              m_animationIndex;
    caf::PdmField<double>                              m_scaleFactor;
    caf::PdmField<size_t>                              m_tracerLength;
    caf::PdmField<size_t>                              m_injectionDeltaTime;

    size_t m_maxAnimationIndex;

    std::list<RigTracer> m_activeTracers;

    std::set<size_t> m_wellCellIds;

    std::vector<cvf::ref<RigResultAccessor>> m_dataAccess;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
};
