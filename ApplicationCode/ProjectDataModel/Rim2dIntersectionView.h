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

#include "Rim3dView.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimIntersection;
class RimRegularLegendConfig;
class RimTernaryLegendConfig;
class RivSimWellPipesPartMgr;
class RivWellHeadPartMgr;
class RivWellPathPartMgr;
class RivIntersectionPartMgr;

namespace cvf
{
class ModelBasicList;
class OverlayItem;
} // namespace cvf

//==================================================================================================
///
///
//==================================================================================================
class Rim2dIntersectionView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;

public:
    Rim2dIntersectionView( void );
    ~Rim2dIntersectionView( void ) override;

    void             setVisible( bool isVisible );
    void             setIntersection( RimIntersection* intersection );
    RimIntersection* intersection() const;

    bool     isUsingFormationNames() const override;
    void     scheduleGeometryRegen( RivCellSetEnum geometryType ) override;
    RimCase* ownerCase() const override;
    void     selectOverlayInfoConfig() override {}

    RimViewLinker* assosiatedViewLinker() const override
    {
        return nullptr;
    }
    RimViewController* viewController() const override
    {
        return nullptr;
    }

    bool isTimeStepDependentDataVisible() const override;

    void update3dInfo();
    void updateName();

    cvf::ref<RivIntersectionPartMgr> flatIntersectionPartMgr() const;
    cvf::Vec3d                       transformToUtm( const cvf::Vec3d& unscaledPointInFlatDomain ) const;

    cvf::ref<caf::DisplayCoordTransform> displayCoordTransform() const override;

    bool showDefiningPoints() const;

    std::vector<RimLegendConfig*> legendConfigs() const override;
    bool                          handleOverlayItemPicked( const cvf::OverlayItem* pickedOverlayItem ) const;

protected:
    void updateLegends() override;

    bool            isGridVisualizationMode() const override;
    void            axisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) override;
    void            createDisplayModel() override;
    void            createPartCollectionFromSelection( cvf::Collection<cvf::Part>* parts ) override;
    void            clampCurrentTimestep() override;
    void            updateCurrentTimeStep() override;
    void            onTimeStepChanged() override;
    void            updateStaticCellColors() override;
    void            updateScaleTransform() override;
    cvf::Transform* scaleTransform() override;
    void            resetLegendsInViewer() override;
    void            onLoadDataAndUpdate() override;
    bool            isWindowVisible() const override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    bool hasResults();
    int  timeStepCount();

private:
    QString createAutoName() const override;
    QString getName() const;
    void    setName( const QString& name );

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
    caf::PdmChildField<RimTernaryLegendConfig*> m_ternaryLegendConfig;

    caf::PdmPtrField<RimIntersection*> m_intersection;

    cvf::ref<RivIntersectionPartMgr> m_flatIntersectionPartMgr;
    cvf::ref<RivSimWellPipesPartMgr> m_flatSimWellPipePartMgr;
    cvf::ref<RivWellHeadPartMgr>     m_flatWellHeadPartMgr;
    cvf::ref<RivWellPathPartMgr>     m_flatWellpathPartMgr;
    cvf::ref<cvf::ModelBasicList>    m_intersectionVizModel;
    cvf::ref<cvf::Transform>         m_scaleTransform;

    caf::PdmProxyValueField<QString> m_nameProxy;
    caf::PdmField<bool>              m_showDefiningPoints;
    caf::PdmField<bool>              m_showAxisLines;

    caf::PdmPointer<caf::PdmObject> m_legendObjectToSelect;

    const static cvf::Mat4d sm_defaultViewMatrix;
};
