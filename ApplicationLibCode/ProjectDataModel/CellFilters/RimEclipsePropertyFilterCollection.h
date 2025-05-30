/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimPropertyFilterCollection.h"

#include "cafPdmChildArrayField.h"

class RimEclipsePropertyFilter;
class RimEclipseView;
class RimEclipseCellColors;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipsePropertyFilterCollection : public RimPropertyFilterCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipsePropertyFilterCollection();

    RimEclipseView* reservoirView();
    void            setIsDuplicatedFromLinkedView();

    std::vector<RimEclipsePropertyFilter*>              propertyFilters() const;
    caf::PdmChildArrayField<RimEclipsePropertyFilter*>& propertyFiltersField();

    bool hasActiveFilters() const override;
    bool hasActiveDynamicFilters() const override;
    bool isUsingFormationNames() const;

    void loadAndInitializePropertyFilters() override;

    void updateIconState() override;
    void updateFromCurrentTimeStep();

    void                      updateDefaultResult( const RimEclipseCellColors* result );
    RimEclipsePropertyFilter* addFilterLinkedToCellResult();

protected:
    void initAfterRead() override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmChildArrayField<RimEclipsePropertyFilter*> m_propertyFilters;
};
