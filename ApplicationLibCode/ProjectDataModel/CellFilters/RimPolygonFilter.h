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

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimPolygon;
class RigGridBase;
class RigFemPartGrid;
class RimPolygonInView;

namespace cvf
{
class ModelBasicList;
class BoundingBox;

}; // namespace cvf

namespace caf
{
class DisplayCoordTransform;
};

//==================================================================================================
///
///
//==================================================================================================
class RimPolygonFilter : public RimCellFilter
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

    void enableFilter( bool bEnable );
    void enableKFilter( bool bEnable );

    bool isFilterEnabled() const override;

    void enablePicking( bool enable );

    void updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex ) override;
    void onGridChanged() override;

    void configurePolygonEditor();
    void appendPartsToModel( cvf::ModelBasicList* model, const caf::DisplayCoordTransform* scaleTransform, const cvf::BoundingBox& boundingBox );

    RimPolygonInView* polygonEditor() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    QString fullName() const override;

private:
    void updateCells();
    void updateCellsForEclipse( const std::vector<cvf::Vec3d>& points, RimEclipseCase* eCase );
    void updateCellsForGeoMech( const std::vector<cvf::Vec3d>& points, RimGeoMechCase* gCase );

    void updateCellsDepthEclipse( const std::vector<cvf::Vec3d>& points, const RigGridBase* grid );
    void updateCellsKIndexEclipse( const std::vector<cvf::Vec3d>& points, const RigGridBase* grid, int K );
    int  findEclipseKLayer( const std::vector<cvf::Vec3d>& points, RigEclipseCaseData* data );

    void updateCellsDepthGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid, int partId );
    void updateCellsKIndexGeoMech( const std::vector<cvf::Vec3d>& points, const RigFemPartGrid* grid, int partId );

    bool cellInsidePolygon2D( cvf::Vec3d center, std::array<cvf::Vec3d, 8>& corners, std::vector<cvf::Vec3d> polygon );

    void initializeCellList();

    bool isPolygonClosed() const;

    void connectObjectSignals( RimPolygon* polygon );
    void onObjectChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<caf::AppEnum<PolygonFilterModeType>> m_polyFilterMode;
    caf::PdmField<caf::AppEnum<PolygonIncludeType>>    m_polyIncludeType;
    caf::PdmField<bool>                                m_enableFiltering;
    caf::PdmField<bool>                                m_enableKFilter;
    caf::PdmField<QString>                             m_kFilterStr;

    std::vector<std::vector<size_t>> m_cells;

    RimCellFilterIntervalTool m_intervalTool;

    // Local polygon and polygon editor
    caf::PdmPtrField<RimPolygon*>         m_cellFilterPolygon;
    caf::PdmChildField<RimPolygon*>       m_internalPolygon;
    caf::PdmChildField<RimPolygonInView*> m_polygonEditor;
    caf::PdmField<bool>                   m_editPolygonButton;
};
