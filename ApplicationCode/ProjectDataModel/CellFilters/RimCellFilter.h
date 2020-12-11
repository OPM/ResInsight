/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

namespace cvf
{
class StructGridInterface;
class CellRangeFilter;
} // namespace cvf

//==================================================================================================
///
///
//==================================================================================================
class RimCellFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum FilterModeType
    {
        INCLUDE,
        EXCLUDE
    };

    caf::Signal<> filterChanged;

    RimCellFilter();
    ~RimCellFilter() override;

    QString                      name() const;
    bool                         isActive() const;
    caf::AppEnum<FilterModeType> filterMode() const;
    void                         setName( QString filtername );
    void                         setActive( bool active );
    QString                      modeString() const;
    bool                         propagateToSubGrids() const;

    void setGridIndex( int gridIndex );
    int  gridIndex() const;

    void updateIconState();
    void updateActiveState( bool isControlled );

    virtual void updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter ) const = 0;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    bool                 isFilterControlled() const;

    const cvf::StructGridInterface* selectedGrid() const;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    caf::PdmField<QString>                      m_name;
    caf::PdmField<bool>                         m_isActive;
    caf::PdmField<caf::AppEnum<FilterModeType>> m_filterMode;
    caf::PdmField<int>                          m_gridIndex;
    caf::PdmField<bool>                         m_propagateToSubGrids; // Do propagate the effects to the sub-grids
};
