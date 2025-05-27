/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RivPolylinePartMgr.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrField.h"

class RimPolygon;
class RivPolylinePartMgr;
class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;

namespace cvf
{
class ModelBasicList;
class BoundingBox;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
} // namespace caf

class RimPolygonInView : public RimCheckableNamedObject, public RimPolylinesDataInterface, public RimPolylinePickerInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonInView();

    RimPolygon* polygon() const;
    void        setPolygon( RimPolygon* polygon );
    void        updateTargetsFromPolygon();

    void appendPartsToModel( cvf::ModelBasicList* model, const caf::DisplayCoordTransform* scaleTransform, const cvf::BoundingBox& boundingBox );
    void enablePicking( bool enable );

    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void deleteTarget( RimPolylineTarget* targetToDelete ) override;
    void updateEditorsAndVisualization() override;
    void updateVisualization() override;
    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;
    double                          scalingFactorForTarget() const override;

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

    bool showLines() const;

    void onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void uiOrderingForLocalPolygon( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    QString label() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void onObjectChanged( const caf::SignalEmitter* emitter );
    void onCoordinatesChanged( const caf::SignalEmitter* emitter );
    void initAfterRead() override;

private:
    void updateNameField();

    void updatePolygonFromTargets();
    void connectSignals();

private:
    caf::PdmPtrField<RimPolygon*> m_polygon;

    caf::PdmField<bool>                         m_selectPolygon;
    caf::PdmField<bool>                         m_enablePicking;
    caf::PdmField<double>                       m_handleScalingFactor;
    caf::PdmChildArrayField<RimPolylineTarget*> m_targets;

    caf::PdmField<bool> m_showLabel;

    cvf::ref<RivPolylinePartMgr>                        m_polylinePartMgr;
    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;
};
