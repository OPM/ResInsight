/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCellFilter.h"
#include "RimCellFilterIntervalTool.h"
#include "RimPolylinePickerInterface.h"
#include "RimShowPolylinesInterface.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafPickEventHandler.h"

#include "cvfColor3.h"

#include <list>
#include <memory>

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;
class RimCase;
class RimEclipseCase;
class RimGeoMechCase;
class RigMainGrid;
class RigFemPartGrid;
class RigPolylinesData;

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonFilter : public RimCellFilter, public RimPolylinePickerInterface, public RimShowPolylinesInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class PolygonFilterModeType
    {
        DEPTH_Z,
        INDEX_K
    };

    enum class PolygonIncludeType
    {
        FULL_CELL,
        CENTER,
        ANY
    };

    RimPolygonFilter();
    ~RimPolygonFilter() override;

    void setCase( RimCase* srcCase );
    void enableFilter( bool bEnable );
    void enableKFilter( bool bEnable );

    void updateVisualization() override;
    void updateEditorsAndVisualization() override;
    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void deleteTarget( RimPolylineTarget* targetToDelete ) override;
    void enablePicking( bool enable );

    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    void updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter ) override;

    cvf::ref<RigPolyLinesData> polyLines() const override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void initAfterRead() override;

    QString fullName() const override;

private:
    void defineCustomContextMenu( const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void updateCells();
    void updateCellsForEclipse( const std::vector<cvf::Vec3d>& points, RimEclipseCase* eCase );
    void updateCellsForGeoMech( const std::vector<cvf::Vec3d>& points, RimGeoMechCase* gCase );

    void updateCellsDepthEclipse( const std::vector<cvf::Vec3d>& points, const RigMainGrid* grid );
    void updateCellsKIndexEclipse( const std::vector<cvf::Vec3d>& points, const RigMainGrid* grid );
    void updateCellsDepthGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid );
    void updateCellsKIndexGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid );

    bool cellInsidePolygon2D( cvf::Vec3d center, std::array<cvf::Vec3d, 8>& corners, std::vector<cvf::Vec3d> polygon );

    caf::PdmField<bool>                                m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*>        m_targets;
    caf::PdmField<caf::AppEnum<PolygonFilterModeType>> m_polyFilterMode;
    caf::PdmField<caf::AppEnum<PolygonIncludeType>>    m_polyIncludeType;
    caf::PdmPtrField<RimCase*>                         m_srcCase;
    caf::PdmField<bool>                                m_enableFiltering;
    caf::PdmField<bool>                                m_enableKFilter;
    caf::PdmField<bool>                                m_showLines;
    caf::PdmField<bool>                                m_showSpheres;
    caf::PdmField<QString>                             m_kFilterStr;
    caf::PdmField<int>                                 m_lineThickness;
    caf::PdmField<double>                              m_sphereRadiusFactor;
    caf::PdmField<cvf::Color3f>                        m_lineColor;
    caf::PdmField<cvf::Color3f>                        m_sphereColor;
    caf::PdmField<double>                              m_polygonPlaneDepth;
    caf::PdmField<bool>                                m_lockPolygonToPlane;

    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;

    std::list<size_t> m_cells;

    RimCellFilterIntervalTool m_intervalTool;
};
