/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmObject.h"
#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildArrayField.h"
#include "RiaWellPlanCalculator.h"
#include "RiaLineArcWellPathCalculator.h"


class RimWellPath;
class RimWellPathTarget;
class RicCreateWellTargetsPickEventHandler;

class RigWellPath;

class RimWellPathGeometryDef : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT; 
public:

    RimWellPathGeometryDef();
    ~RimWellPathGeometryDef() override;

    enum WellStartType { START_AT_FIRST_TARGET, START_AT_SURFACE, START_FROM_OTHER_WELL, START_AT_AUTO_SURFACE };

    cvf::Vec3d                      referencePointXyz() const;
    void                            setReferencePointXyz(const cvf::Vec3d& refPointXyz );

    double                          mdrkbAtFirstTarget() const;

    cvf::ref<RigWellPath>           createWellPathGeometry();
    
    void                            updateWellPathVisualization();
    std::pair<RimWellPathTarget*, RimWellPathTarget*> findActiveTargetsAroundInsertionPoint(const RimWellPathTarget* targetToInsertBefore);

    void                            insertTarget(const RimWellPathTarget* targetToInsertBefore, 
                                                 RimWellPathTarget* targetToInsert);
    void                            deleteTarget(RimWellPathTarget* targetTodelete);
    void                            appendTarget();

    const RimWellPathTarget*        firstActiveTarget() const;
    const RimWellPathTarget*        lastActiveTarget() const;

    void                            enableTargetPointPicking(bool isEnabling);

    std::vector<RiaWellPlanCalculator::WellPlanSegment> wellPlan() const; 
    std::vector<RimWellPathTarget*> activeWellTargets() const;
protected:
    void                            defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, 
                                                            QMenu* menu, 
                                                            QWidget* fieldEditorWidget) override;
                                    
                                    
    void                            defineEditorAttribute(const caf::PdmFieldHandle* field, 
                                                          QString uiConfigName, 
                                                          caf::PdmUiEditorAttribute* attribute) override;
                                    


private:
    void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, 
                                                     const QVariant& oldValue, 
                                                     const QVariant& newValue) override;
    void                            defineUiOrdering(QString uiConfigName, 
                                                     caf::PdmUiOrdering& uiOrdering) override;
    void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, 
                                                         QString uiConfigName) override;
    void                            initAfterRead() override;
    QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

    RiaLineArcWellPathCalculator    lineArcWellPathCalculator() const;

private:
    caf::PdmField<cvf::Vec3d>                        m_referencePointUtmXyd;
    caf::PdmField<cvf::Vec3d>                        m_referencePointXyz_OBSOLETE;

    caf::PdmField<double>                            m_mdrkbAtFirstTarget;
    caf::PdmChildArrayField<RimWellPathTarget*>      m_wellTargets;

    caf::PdmField< bool >                            m_pickPointsEnabled;
    RicCreateWellTargetsPickEventHandler*            m_pickTargetsEventHandler;

    // Unused for now. Remove when dust settles

    caf::PdmField<caf::AppEnum<WellStartType> >      m_wellStartType;
    caf::PdmField<double>                            m_kickoffDepthOrMD;
    caf::PdmPtrField<RimWellPath*>                   m_parentWell;
};
