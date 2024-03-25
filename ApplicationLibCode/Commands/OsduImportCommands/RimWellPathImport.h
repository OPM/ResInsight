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
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimOilRegionEntry;

class RimWellPathImport : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum UtmFilterEnum
    {
        UTM_FILTER_OFF,
        UTM_FILTER_PROJECT,
        UTM_FILTER_CUSTOM
    };

public:
    RimWellPathImport();
    ~RimWellPathImport() override;

    caf::PdmField<caf::AppEnum<UtmFilterEnum>> utmFilterMode;
    caf::PdmField<double>                      north;
    caf::PdmField<double>                      south;
    caf::PdmField<double>                      east;
    caf::PdmField<double>                      west;

    caf::PdmChildArrayField<RimOilRegionEntry*> regions;

    void updateRegions( const QStringList& regions, const QStringList& fields, const QStringList& edmIds );

    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void updateFieldVisibility();

    void updateFilePaths();
};
