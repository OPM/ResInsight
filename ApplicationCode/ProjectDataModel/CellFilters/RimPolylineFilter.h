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
#include "RimPolylinePickerInterface.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPickEventHandler.h"

#include <memory>

class RicPolylineTargetsPickEventHandler;
class RimPolylineTarget;

//==================================================================================================
///
///
//==================================================================================================
class RimPolylineFilter : public RimCellFilter, public RimPolylinePickerInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum PolylineFilterModeType
    {
        DEPTH_Z,
        INDEX_K
    };

    RimPolylineFilter();
    ~RimPolylineFilter() override;

    void updateVisualization();

    void enablePicking( bool enable );

    void insertTarget( const RimPolylineTarget* targetToInsertBefore, RimPolylineTarget* targetToInsert ) override;
    void updateEditorsAndVisualization() override;
    std::vector<RimPolylineTarget*> activeTargets() const override;
    bool                            pickingEnabled() const override;
    caf::PickEventHandler*          pickEventHandler() const override;

    void updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter ) const override;

protected:
    // void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmField<bool>                                 m_enablePicking;
    caf::PdmChildArrayField<RimPolylineTarget*>         m_targets;
    caf::PdmField<bool>                                 m_showPolygon;
    caf::PdmField<caf::AppEnum<PolylineFilterModeType>> m_polyFilterMode;

    std::shared_ptr<RicPolylineTargetsPickEventHandler> m_pickTargetsEventHandler;
};
