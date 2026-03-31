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

#include "RimCheckableNamedObject.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include "cvfArray.h"

namespace cvf
{
class StructGridInterface;
class CellRangeFilter;
} // namespace cvf

class RimGeoMechCase;
class RimEclipseCase;
class RimCase;

//==================================================================================================
///
///
//==================================================================================================
class RimCellFilter : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum FilterModeType
    {
        INCLUDE,
        EXCLUDE
    };

    enum FilterDefinitionType
    {
        RANGE,
        INDEX,
        PROPERTY
    };

    caf::Signal<> filterChanged;

    RimCellFilter( FilterDefinitionType defType );
    ~RimCellFilter() override;

    bool isActive() const;
    void setActive( bool active );

    void triggerFilterChanged() const;

    virtual void setCase( RimCase* srcCase );

    bool isRangeFilter() const;
    bool isIndexFilter() const;

    virtual bool isFilterEnabled() const;

    caf::AppEnum<FilterModeType> filterMode() const;
    QString                      modeString() const;

    bool propagateToSubGrids() const;

    void setGridIndex( int gridIndex );
    int  gridIndex() const;

    void updateIconState();
    void updateActiveState( bool isControlled );

    virtual void    updateCompundFilter( cvf::CellRangeFilter* cellRangeFilter, int gridIndex ) {};
    virtual void    updateCellIndexFilter( cvf::UByteArray* includeVisibility, cvf::UByteArray* excludeVisibility, int gridIndex ) {};
    virtual QString fullName() const;
    virtual void    onGridChanged() {};

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    bool                 isFilterControlled() const;

    RimGeoMechCase* geoMechCase() const;
    RimEclipseCase* eclipseCase() const;

    const cvf::StructGridInterface* selectedGrid() const;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    caf::PdmProxyValueField<QString>            m_nameProxy;
    caf::PdmField<caf::AppEnum<FilterModeType>> m_filterMode;
    caf::PdmField<int>                          m_gridIndex;
    caf::PdmField<bool>                         m_propagateToSubGrids;
    caf::PdmPtrField<RimCase*>                  m_srcCase;

private:
    FilterDefinitionType m_filterDefinitionType;
};
