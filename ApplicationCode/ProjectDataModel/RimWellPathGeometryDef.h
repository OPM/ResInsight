/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor ASA
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

class RimWellPath;
class RimWellPathTarget;

class RigWellPath;

class RimWellPathGeometryDef : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT; 
public:

    RimWellPathGeometryDef();
    ~RimWellPathGeometryDef() override;

    enum WellStartType { START_AT_FIRST_TARGET, START_AT_SURFACE, START_FROM_OTHER_WELL, START_AT_AUTO_SURFACE };

    cvf::ref<RigWellPath> createWellPathGeometry();
    
    void updateWellPathVisualization();

    void insertTarget(RimWellPathTarget* targetToInsertBefore, RimWellPathTarget* targetToInsert);
    void deleteTarget(RimWellPathTarget* targetTodelete);
    void appendTarget();

    const RimWellPathTarget* firstActiveTarget() const;
    const RimWellPathTarget* lastActiveTarget() const;

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

protected:
    virtual void defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, 
                                         QMenu* menu, 
                                         QWidget* fieldEditorWidget) override;

private:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

    std::vector<RimWellPathTarget*> activeWellTargets() const;
    std::vector<cvf::Vec3d>         lineArcEndpoints() const;
    cvf::Vec3d                      startTangent() const;

private:
    caf::PdmField<caf::AppEnum<WellStartType> >      m_wellStartType;
    caf::PdmField<cvf::Vec3d>                        m_referencePoint;

    caf::PdmField<double>                            m_kickoffDepthOrMD;
    caf::PdmPtrField<RimWellPath*>                   m_parentWell;

    caf::PdmChildArrayField<RimWellPathTarget*>      m_wellTargets;
};



