/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <expected>
#include <vector>

class RimCellFilter;
class RimDataFilterCollection;
class RimEclipseCase;
class RimModeledWellPath;
class RimReservoirGridEnsemble;
class RimWellPath;

//==================================================================================================
/// Geo-guided well placement: for each realization of a grid ensemble, create a modeled well path
/// from a source well where the targets inside the grid are moved vertically to a fraction of the
/// zone defined by a cell filter.
//==================================================================================================
class RimGeoGuidedWellPlacement : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoGuidedWellPlacement();
    ~RimGeoGuidedWellPlacement() override;

    void setDefaults();

    std::expected<void, QString> compute();

    std::vector<RimWellPath*> generatedWellPaths() const;

    static QString generatedWellName( const QString& sourceWellName, const QString& iterationName, int realization );
    static QString iterationName( RimEclipseCase* eclipseCase );
    static int     realizationNumber( RimEclipseCase* eclipseCase );

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    std::expected<void, QString> validateInput() const;
    void                         deleteGeneratedWellPaths();

    std::expected<RimModeledWellPath*, QString> createWellPathForRealization( RimEclipseCase* eclipseCase );
    RimModeledWellPath*                         createWellPathFromSource() const;
    void                                        addPerforationInterval( RimModeledWellPath* wellPath ) const;

    RimReservoirGridEnsemble* ensemble() const;
    RimDataFilterCollection*  dataFilterCollection() const;
    bool                      isFilterInEnsemble( RimCellFilter* filter ) const;

    void onComputeButtonClicked();

private:
    caf::PdmPtrField<RimWellPath*>   m_sourceWellPath;
    caf::PdmPtrField<RimCellFilter*> m_cellFilter;
    caf::PdmField<double>            m_fraction;
    caf::PdmPtrField<RimCellFilter*> m_perforationFilter;
    caf::PdmField<int>               m_timeStep;
    caf::PdmField<double>            m_resampleSpacing;

    caf::PdmPtrArrayField<RimWellPath*> m_generatedWellPaths;
};
