/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 equinor ASA
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
#include "RimPolylinesAnnotation.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmChildArrayField.h"

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;

//==================================================================================================
///
///
//==================================================================================================

class RimUserDefinedPolylinesAnnotation : public RimPolylinesAnnotation
{
    friend class RimUserDefinedPolylinesAnnotationInView;

    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;
public:
    RimUserDefinedPolylinesAnnotation();
    ~RimUserDefinedPolylinesAnnotation();

    cvf::ref<RigPolyLinesData>  polyLinesData() override;
    std::vector<RimPolylineTarget*> activeTargets() const;
    virtual bool isEmpty() override;

    void appendTarget();
    void insertTarget(const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert);
    void deleteTarget(RimPolylineTarget* targetTodelete);

    std::pair<RimPolylineTarget*, RimPolylineTarget*>
        findActiveTargetsAroundInsertionPoint(const RimPolylineTarget* targetToInsertBefore);
    void updateVisualization();

    void enablePicking(bool enable);

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    void defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;

private:
    caf::PdmField<bool>                         m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;
    RicPolylineTargetsPickEventHandler*         m_pickTargetsEventHandler;
};


