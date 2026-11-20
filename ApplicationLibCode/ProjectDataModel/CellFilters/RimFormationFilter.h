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

#include "RimEclipsePropertyFilter.h"

#include <expected>
#include <vector>

class RimEclipseCase;
class RimReservoirGridEnsemble;

//==================================================================================================
/// Category property filter on the active formation names result. Reuses the result definition and
/// category selection of RimEclipsePropertyFilter, but with a simplified editor, formation icon and
/// formation based naming.
//==================================================================================================
class RimFormationFilter : public RimEclipsePropertyFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimFormationFilter();

    void setToFormationNames();
    void updateFilterName() override;

    // Formation names of the filter case, in formation index order. For an ensemble, the filter case is
    // the main case of the ensemble. Empty until the formation names result is loaded.
    std::vector<QString> availableFormationNames() const;

    // The selection is stored as formation indices. Each name selects all formations with that name, and
    // an empty list selects all formations.
    std::expected<void, QString> setSelectedFormationNames( const std::vector<QString>& formationNames );
    std::vector<QString>         selectedFormationNames() const;

    // Realizations of the owning ensemble with a formation list different from the main case. The
    // selection is resolved against the main case, so the filter can select other formations in these.
    std::vector<RimEclipseCase*> realizationsWithMismatchedFormations() const;
    void                         warnIfRealizationsHaveMismatchedFormations() const;

    RimReservoirGridEnsemble* parentEnsemble() const;

    // Replace generic property filters using the formation names result, as saved before this class
    // existed, with formation filters
    static void convertPropertyFiltersToFormationFilters( caf::PdmChildArrayField<RimCellFilter*>& filters );

private:
    QString iconResourceString() const override;
    void    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QString compressedSelectedCategoryNames( int maxLength ) const;
    bool    hasFormationNames() const;
};
