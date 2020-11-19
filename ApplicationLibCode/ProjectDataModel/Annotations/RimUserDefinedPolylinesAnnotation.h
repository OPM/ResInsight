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

#include "RimPolylinePickerInterface.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmFieldCvfVec3d.h"

#include <memory>

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;

//==================================================================================================
///
///
//==================================================================================================

class RimUserDefinedPolylinesAnnotation : public RimPolylinesAnnotation, public RimPolylinePickerInterface
{
    friend class RimUserDefinedPolylinesAnnotationInView;

    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;

public:
    RimUserDefinedPolylinesAnnotation();
    ~RimUserDefinedPolylinesAnnotation() override;

    cvf::ref<RigPolyLinesData> polyLinesData() override;
    bool                       isEmpty() override;

    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void updateEditorsAndVisualization() override;
    void updateVisualization() override;

    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    void appendTarget( const cvf::Vec3d& defaultPos = cvf::Vec3d::ZERO );
    void deleteTarget( RimPolylineTarget* targetTodelete );

    std::pair<RimPolylineTarget*, RimPolylineTarget*>
        findActiveTargetsAroundInsertionPoint( const RimPolylineTarget* targetToInsertBefore );

    void enablePicking( bool enable );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void                 defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    caf::PdmField<QString>                      m_name;
    caf::PdmField<bool>                         m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;

    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;
};
