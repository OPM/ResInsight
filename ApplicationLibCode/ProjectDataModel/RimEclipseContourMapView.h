/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimEclipseView.h"
#include "RimNameConfig.h"

enum class RimLegendConfigChangeType;
class RimEclipseContourMapProjection;
class RimRegularLegendConfig;
class RimViewNameConfig;
class RimScaleLegendConfig;
class RivContourMapProjectionPartMgr;

class RimEclipseContourMapView : public RimEclipseView
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseContourMapView();
    RimEclipseContourMapProjection* contourMapProjection() const;

    QString createAutoName() const override;
    void    setDefaultCustomName();
    void    updatePickPointAndRedraw();

protected:
    void initAfterRead() override;
    void onCreateDisplayModel() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void onUpdateDisplayModelForCurrentTimeStep() override;
    void updateGeometry();
    void setFaultVisParameters();
    void createContourMapGeometry();
    void appendContourMapProjectionToModel();
    void appendContourLinesToModel();
    void appendPickPointVisToModel();
    void onUpdateLegends() override;
    void updateViewWidgetAfterCreation() override;
    void updateViewFollowingRangeFilterUpdates() override;
    void onLoadDataAndUpdate() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

    std::set<RivCellSetEnum> allVisibleFaultGeometryTypes() const override;

    QWidget* createViewWidget( QWidget* mainWindowParent ) override;

    void onViewNavigationChanged() override;

    bool zoomChangeAboveTreshold( const cvf::Vec3d& currentCameraPosition ) const;
    void scheduleGeometryRegen( RivCellSetEnum geometryType ) override;

    void onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType );

private:
    cvf::ref<RivContourMapProjectionPartMgr>            m_contourMapProjectionPartMgr;
    caf::PdmChildField<RimEclipseContourMapProjection*> m_contourMapProjection;
    caf::PdmField<bool>                                 m_showAxisLines;
    caf::PdmField<bool>                                 m_showScaleLegend;
    cvf::Vec3d                                          m_cameraPositionLastUpdate;

    const static cvf::Mat4d sm_defaultViewMatrix;
};
