/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026   Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafVecIjk.h"

#include "cvfColor3.h"

#include <memory>

class RicRefinementSettings;
class RigMainGrid;
class RigRefinement;
class RimEclipseCase;

//==================================================================================================
///
/// One declarative refinement region for sector export. Defines an axis-aligned IJK box plus
/// a refinement specification (uniform or non-uniform). Rendered as a wireframe box in the 3D
/// view so the user can preview it before opening the sector-export dialog.
///
//==================================================================================================
class RimRefinementRegion : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimRefinementRegion();

    bool    isActive() const;
    QString regionName() const;
    void    setRegionName( const QString& name );

    // IJK bounds, 1-based (Eclipse convention) as stored in the UI fields
    int startI() const;
    int startJ() const;
    int startK() const;
    int cellCountI() const;
    int cellCountJ() const;
    int cellCountK() const;

    // 0-based convenience accessors
    caf::VecIjk0 ijkMin() const;
    caf::VecIjk0 ijkMax() const;

    // Populate the region with defaults derived from the case's main grid (full grid, refinement 1,1,1).
    void setDefaultsFromCase( RimEclipseCase* eclipseCase );

    // Build the refinement for this region alone, sized to its own bounds.
    std::unique_ptr<RigRefinement> effectiveRefinement() const;

    // Access the embedded refinement settings (used by the collection to assemble combined refinement).
    RicRefinementSettings* refinementSettings() const;

    // Validate that the region lies entirely within the given sector bounds (1-based).
    // Returns an empty string on success, otherwise a user-readable error message.
    QString validateWithinSector( int sectorMinI,
                                  int sectorMinJ,
                                  int sectorMinK,
                                  int sectorMaxI,
                                  int sectorMaxJ,
                                  int sectorMaxK ) const;

    cvf::Color3f previewColor() const;

    caf::PdmFieldHandle* objectToggleField() override;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    const RigMainGrid* mainGrid() const;

    caf::PdmField<bool>    m_isActive;
    caf::PdmField<QString> m_regionName;

    caf::PdmField<int> m_startI;
    caf::PdmField<int> m_startJ;
    caf::PdmField<int> m_startK;
    caf::PdmField<int> m_cellCountI;
    caf::PdmField<int> m_cellCountJ;
    caf::PdmField<int> m_cellCountK;

    caf::PdmField<cvf::Color3f> m_previewColor;

    caf::PdmChildField<RicRefinementSettings*> m_refinementSettings;
};
