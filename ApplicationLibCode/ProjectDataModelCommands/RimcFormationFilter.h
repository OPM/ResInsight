/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

#include <expected>
#include <vector>

class RimFormationFilter;
class RimCombinedFilter;
class RimDataFilterCollection;

namespace caf
{
class PdmObject;
}

//==================================================================================================
/// Shared implementation of the AddFormationFilter script methods
//==================================================================================================
namespace RimcFormationFilterTools
{
// Make the formation names of the filter case available. For an ensemble, the grids of the ensemble are
// loaded, and the main case is used. Returns an error if no formations are loaded.
std::expected<void, QString> ensureFormationNamesAreLoaded( caf::PdmObject* filterOwner );

// Check the requested formation names against the filter case before a filter is created
std::expected<void, QString> validateFormationNames( caf::PdmObject* filterOwner, const std::vector<QString>& formationNames );

std::expected<RimFormationFilter*, QString>
    addFormationFilter( RimCombinedFilter* combinedFilter, const std::vector<QString>& formationNames, const QString& filterMode );

std::expected<RimFormationFilter*, QString>
    addFormationFilter( RimDataFilterCollection* collection, const std::vector<QString>& formationNames, const QString& filterMode );
} // namespace RimcFormationFilterTools

//==================================================================================================
/// Adds a new formation filter as a child of a RimCombinedFilter.
//==================================================================================================
class RimcCombinedFilter_addFormationFilter : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcCombinedFilter_addFormationFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>> m_formationNames;
    caf::PdmField<QString>              m_filterMode;
};

//==================================================================================================
/// Adds a new formation filter to a RimDataFilterCollection.
//==================================================================================================
class RimcDataFilterCollection_addFormationFilter : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcDataFilterCollection_addFormationFilter( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<std::vector<QString>> m_formationNames;
    caf::PdmField<QString>              m_filterMode;
};

//==================================================================================================
/// Names of the formations selected by a formation filter. Exposed in Python as
/// FormationFilter.selected_formation_names().
//==================================================================================================
class RimcFormationFilter_selectedFormationNames : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcFormationFilter_selectedFormationNames( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;
};

//==================================================================================================
/// Select the formations of a formation filter by name. Exposed in Python as
/// FormationFilter.set_selected_formation_names().
//==================================================================================================
class RimcFormationFilter_setSelectedFormationNames : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcFormationFilter_setSelectedFormationNames( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<std::vector<QString>> m_formationNames;
};
