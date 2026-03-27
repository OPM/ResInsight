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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafVecIjk.h"

#include "RigNonUniformRefinement.h"

#include <QString>

#include <map>

//==================================================================================================
///
/// Settings for grid refinement (both uniform and non-uniform)
///
//==================================================================================================
class RicRefinementSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum RefinementMode
    {
        UNIFORM,
        NON_UNIFORM
    };

    enum NonUniformSubMode
    {
        CUSTOM_WIDTHS,
        LINEAR_EQUAL_SPLIT,
        LOGARITHMIC_CENTER
    };

    using RefinementModeEnum    = caf::AppEnum<RefinementMode>;
    using NonUniformSubModeEnum = caf::AppEnum<NonUniformSubMode>;

    RicRefinementSettings();

    void setSectorBounds( const caf::VecIjk0& min, const caf::VecIjk0& max );

    // Data access
    cvf::Vec3st                 refinement() const;
    RefinementMode              refinementMode() const;
    RigNonUniformRefinement     nonUniformRefinement() const;
    bool                        hasNonUniformRefinement() const;

    // UI integration
    void                       addToUiOrdering( caf::PdmUiOrdering& uiOrdering );
    std::map<QString, QString> validateSettings() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    static std::vector<double> parseWidths( const QString& text );

private:
    caf::PdmField<bool> m_refineGrid;
    caf::PdmField<int>  m_refinementCountI;
    caf::PdmField<int>  m_refinementCountJ;
    caf::PdmField<int>  m_refinementCountK;

    caf::PdmField<RefinementModeEnum> m_refinementMode;

    caf::PdmField<bool>    m_nonUniformEnableI;
    caf::PdmField<int>     m_nonUniformRangeStartI;
    caf::PdmField<int>     m_nonUniformRangeEndI;
    caf::PdmField<QString> m_nonUniformIntervalsI;

    caf::PdmField<bool>    m_nonUniformEnableJ;
    caf::PdmField<int>     m_nonUniformRangeStartJ;
    caf::PdmField<int>     m_nonUniformRangeEndJ;
    caf::PdmField<QString> m_nonUniformIntervalsJ;

    caf::PdmField<bool>    m_nonUniformEnableK;
    caf::PdmField<int>     m_nonUniformRangeStartK;
    caf::PdmField<int>     m_nonUniformRangeEndK;
    caf::PdmField<QString> m_nonUniformIntervalsK;

    caf::PdmField<NonUniformSubModeEnum> m_nonUniformSubMode;

    caf::PdmField<int> m_nonUniformSubcellCountI;
    caf::PdmField<int> m_nonUniformSubcellCountJ;
    caf::PdmField<int> m_nonUniformSubcellCountK;

    caf::PdmField<int> m_nonUniformTotalCellsI;
    caf::PdmField<int> m_nonUniformTotalCellsJ;
    caf::PdmField<int> m_nonUniformTotalCellsK;

    // Sector bounds (1-based) for coordinate conversion
    int m_sectorMinI;
    int m_sectorMinJ;
    int m_sectorMinK;
    int m_sectorMaxI;
    int m_sectorMaxJ;
    int m_sectorMaxK;
};
