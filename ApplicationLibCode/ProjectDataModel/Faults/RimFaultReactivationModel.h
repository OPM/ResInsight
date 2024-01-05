/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimCheckableNamedObject.h"
#include "RimFaultReactivationEnums.h"
#include "RimPolylinePickerInterface.h"
#include "RimPolylinesDataInterface.h"
#include "RimTimeStepFilter.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfColor3.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <memory>
#include <utility>

class RicPolylineTargetsPickEventHandler;
class RimEclipseCase;
class RimGeoMechCase;
class RimFaultInView;
class RimParameterGroup;
class RimPolylineTarget;
class RimTimeStepFilter;
class RivFaultReactivationModelPartMgr;
class RigBasicPlane;
class RigFaultReactivationModel;

namespace cvf
{
class BoundingBox;
class Plane;
} // namespace cvf

class RimFaultReactivationModel : public RimCheckableNamedObject, public RimPolylinePickerInterface, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

    using TimeStepFilterEnum = caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>;
    using ElementSets        = RimFaultReactivation::ElementSets;

public:
    RimFaultReactivationModel();
    ~RimFaultReactivationModel() override;

    bool initSettings( QString& outErrmsg );

    QString userDescription();
    void    setUserDescription( QString description );

    std::pair<bool, std::string> validateModel() const;

    void            setFaultInformation( RimFaultInView* fault, size_t cellIndex, cvf::StructGridInterface::FaceType face );
    RimFaultInView* fault() const;

    void setTargets( cvf::Vec3d target1, cvf::Vec3d target2 );

    RivFaultReactivationModelPartMgr* partMgr();

    // polyline picker interface
    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void deleteTarget( RimPolylineTarget* targetToDelete ) override;
    void updateEditorsAndVisualization() override;
    void updateVisualization() override;

    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    // polyline data interface
    cvf::ref<RigPolyLinesData> polyLinesData() const override;

    cvf::ref<RigFaultReactivationModel> model() const;
    bool                                showModel() const;

    QString baseDir() const;
    void    setBaseDir( QString path );

    std::vector<QDateTime> selectedTimeSteps() const;
    std::vector<size_t>    selectedTimeStepIndexes() const;

    std::array<double, 3> materialParameters( ElementSets elementSet ) const;

    QStringList commandParameters() const;

    std::string outputOdbFilename() const;
    std::string inputFilename() const;
    std::string settingsFilename() const;
    std::string baseFilePath() const;

    void updateTimeSteps();

    bool useGridVoidRatio() const;
    bool useGridPorePressure() const;
    bool useGridTemperature() const;
    bool useGridDensity() const;
    bool useGridElasticProperties() const;
    bool useGridStress() const;

    double seaBedDepth() const;
    double waterDensity() const;
    double seabedTemperature() const;

    RimEclipseCase* eclipseCase() const;
    RimGeoMechCase* geoMechCase() const;

protected:
    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    std::string baseFilename() const;

private:
    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;

    cvf::ref<RivFaultReactivationModelPartMgr> m_partMgr;

    caf::PdmField<caf::FilePath> m_baseDir;
    caf::PdmField<double>        m_modelThickness;

    caf::PdmField<QString>                      m_userDescription;
    caf::PdmPtrField<RimFaultInView*>           m_fault;
    caf::PdmPtrField<RimGeoMechCase*>           m_geomechCase;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;
    caf::PdmField<cvf::Color3f>                 m_modelPart1Color;
    caf::PdmField<cvf::Color3f>                 m_modelPart2Color;

    caf::PdmField<bool> m_showModelPlane;

    caf::PdmField<double> m_modelExtentFromAnchor;
    caf::PdmField<double> m_modelMinZ;
    caf::PdmField<double> m_modelBelowSize;

    caf::PdmField<double> m_faultExtendUpwards;
    caf::PdmField<double> m_faultExtendDownwards;

    caf::PdmField<double> m_maxReservoirCellHeight;
    caf::PdmField<double> m_cellHeightGrowFactor;
    caf::PdmField<double> m_minReservoirCellWidth;
    caf::PdmField<double> m_cellWidthGrowFactor;

    caf::PdmField<bool> m_useLocalCoordinates;

    caf::PdmField<bool> m_useGridPorePressure;
    caf::PdmField<bool> m_useGridVoidRatio;
    caf::PdmField<bool> m_useGridTemperature;
    caf::PdmField<bool> m_useGridDensity;
    caf::PdmField<bool> m_useGridElasticProperties;
    caf::PdmField<bool> m_useGridStress;

    caf::PdmField<double> m_waterDensity;
    caf::PdmField<double> m_seabedTemperature;

    caf::PdmField<size_t> m_startCellIndex;
    caf::PdmField<int>    m_startCellFace;

    cvf::ref<RigFaultReactivationModel> m_2Dmodel;

    caf::PdmField<TimeStepFilterEnum>     m_timeStepFilter;
    caf::PdmField<std::vector<QDateTime>> m_selectedTimeSteps;

    caf::PdmChildArrayField<RimParameterGroup*> m_materialParameters;

    std::vector<QDateTime> m_availableTimeSteps;
};
