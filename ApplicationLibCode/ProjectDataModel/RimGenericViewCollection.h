/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "cafPdmField.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

#include <QString>

class RimEclipseView;
class RimEclipseCase;
class RimGeneric3dView;

//==================================================================================================
///
/// Top level collection holding both Eclipse views and generic (case-less) 3D views. The two
/// types are displayed as a single flat list, see defineUiTreeOrdering().
///
//==================================================================================================
class RimGenericViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGenericViewCollection();
    ~RimGenericViewCollection() override;

    bool isEmpty() const;

    RimEclipseView* addView( RimEclipseCase* eclipseCase );
    void            addView( RimEclipseView* view );
    void            removeView( RimEclipseView* view );

    std::vector<RimEclipseView*> views() const;

    RimGeneric3dView*              addGenericView();
    std::vector<RimGeneric3dView*> genericViews() const;

private:
    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    caf::PdmChildArrayField<RimEclipseView*>   m_eclipseViews;
    caf::PdmChildArrayField<RimGeneric3dView*> m_genericViews;
};
