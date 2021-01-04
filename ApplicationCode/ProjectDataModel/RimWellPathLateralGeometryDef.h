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

#include "RimWellPathGeometryDefInterface.h"

#include "RiaLineArcWellPathCalculator.h"
#include "RiaWellPlanCalculator.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RimWellPath;
class RimWellPathTarget;
class RicCreateWellTargetsPickEventHandler;

class RigWellPath;

class RimWellPathLateralGeometryDef : public RimWellPathGeometryDefInterface
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<bool> changed;

public:
    RimWellPathLateralGeometryDef();
    ~RimWellPathLateralGeometryDef() override;

    double mdAtConnection() const;
    void   setMdAtConnection( double mdrkb );

    cvf::Vec3d anchorPointXyz() const override;

    void createTargetAtConnectionPoint( const cvf::Vec3d& tangent );

    void                  setParentGeometry( const RigWellPath* parentGeometry );
    cvf::ref<RigWellPath> createWellPathGeometry();

    void updateWellPathVisualization( bool fullUpdate );
    std::pair<RimWellPathTarget*, RimWellPathTarget*>
        findActiveTargetsAroundInsertionPoint( const RimWellPathTarget* targetToInsertBefore );

    void               insertTarget( const RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert );
    void               deleteTarget( RimWellPathTarget* targetTodelete );
    void               deleteAllTargets();
    RimWellPathTarget* appendTarget();

    const RimWellPathTarget* firstActiveTarget() const;
    const RimWellPathTarget* lastActiveTarget() const;

    void enableTargetPointPicking( bool isEnabling );

    std::vector<RiaWellPlanCalculator::WellPlanSegment> wellPlan() const;
    std::vector<RimWellPathTarget*>                     activeWellTargets() const;

protected:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

    void initAfterRead() override;

    RiaLineArcWellPathCalculator lineArcWellPathCalculator() const;

    void onTargetMoved( const caf::SignalEmitter* moved, bool fullUpdate );

private:
    caf::PdmField<double>                       m_connectionMdOnParentWellPath;
    caf::PdmChildArrayField<RimWellPathTarget*> m_wellTargets;
    caf::PdmField<bool>                         m_pickPointsEnabled;

    std::shared_ptr<RicCreateWellTargetsPickEventHandler> m_pickTargetsEventHandler;

    cvf::cref<RigWellPath> m_parentGeometry;
};
