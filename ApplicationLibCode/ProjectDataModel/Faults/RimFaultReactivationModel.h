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
#include "RimPolylinePickerInterface.h"
#include "RimPolylinesDataInterface.h"

#include "cafFilePath.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfColor3.h"
#include "cvfVector3.h"

#include <QString>

#include <memory>
#include <utility>

class RicPolylineTargetsPickEventHandler;
class RimEclipseCase;
class RimFaultInView;
class RimPolylineTarget;
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

public:
    RimFaultReactivationModel();
    ~RimFaultReactivationModel() override;

    QString userDescription();
    void    setUserDescription( QString description );

    void            setFault( RimFaultInView* fault );
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

    cvf::ref<RigBasicPlane> faultPlane() const;
    bool                    showFaultPlane() const;

    cvf::ref<RigFaultReactivationModel> modelPlane() const;
    bool                                showModelPlane() const;

protected:
    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    RimEclipseCase* eclipseCase();

    void initAfterRead() override;

private:
    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;

    cvf::ref<RivFaultReactivationModelPartMgr> m_partMgr;

    caf::PdmField<QString>                      m_userDescription;
    caf::PdmPtrField<RimFaultInView*>           m_fault;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;
    caf::PdmField<cvf::Color3f>                 m_faultPlaneColor;
    caf::PdmField<cvf::Color3f>                 m_modelPart1Color;
    caf::PdmField<cvf::Color3f>                 m_modelPart2Color;

    caf::PdmField<bool> m_showFaultPlane;
    caf::PdmField<bool> m_showModelPlane;

    caf::PdmField<double> m_extentVerticalAbove;
    caf::PdmField<double> m_extentVerticalBelow;
    caf::PdmField<double> m_extentHorizontal;
    caf::PdmField<double> m_modelExtentFromAnchor;
    caf::PdmField<double> m_modelMinZ;
    caf::PdmField<double> m_modelBelowSize;
    caf::PdmField<int>    m_numberOfCellsHorzPart1;
    caf::PdmField<int>    m_numberOfCellsHorzPart2;
    caf::PdmField<int>    m_numberOfCellsVertUp;
    caf::PdmField<int>    m_numberOfCellsVertMid;
    caf::PdmField<int>    m_numberOfCellsVertLow;

    cvf::ref<RigBasicPlane>             m_faultPlane;
    cvf::ref<RigFaultReactivationModel> m_modelPlane;
};
