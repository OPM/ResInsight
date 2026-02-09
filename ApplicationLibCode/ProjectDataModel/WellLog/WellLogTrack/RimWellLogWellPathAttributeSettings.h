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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimWellPath;
class RimWellPathAttributeCollection;

//==================================================================================================
///
/// Settings class for well path attribute/completion display in well log tracks
///
//==================================================================================================
class RimWellLogWellPathAttributeSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogWellPathAttributeSettings();
    ~RimWellLogWellPathAttributeSettings() override;

    // Show attributes/completions
    bool showWellPathAttributes() const;
    void setShowWellPathAttributes( bool show );

    bool showWellPathCompletions() const;
    void setShowWellPathCompletions( bool show );

    // Display options
    bool showBothSides() const;
    void setShowBothSides( bool show );

    bool showComponentLabels() const;
    void setShowComponentLabels( bool show );

    // Legend options
    bool showAttributesInLegend() const;
    void setShowAttributesInLegend( bool show );

    bool showCompletionsInLegend() const;
    void setShowCompletionsInLegend( bool show );

    // Well path source
    RimWellPath* wellPathComponentSource() const;
    void         setWellPathComponentSource( RimWellPath* wellPath );

    RimWellPathAttributeCollection* wellPathAttributeCollection() const;
    void                            setWellPathAttributeCollection( RimWellPathAttributeCollection* collection );

    // Overburden/underburden
    double overburdenHeight() const;
    void   setOverburdenHeight( double height );

    double underburdenHeight() const;
    void   setUnderburdenHeight( double height );

    // UI ordering
    void uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<bool>                               m_showWellPathAttributes;
    caf::PdmField<bool>                               m_showWellPathCompletions;
    caf::PdmField<bool>                               m_showWellPathComponentsBothSides;
    caf::PdmField<bool>                               m_showWellPathComponentLabels;
    caf::PdmField<bool>                               m_wellPathAttributesInLegend;
    caf::PdmField<bool>                               m_wellPathCompletionsInLegend;
    caf::PdmPtrField<RimWellPath*>                    m_wellPathComponentSource;
    caf::PdmPtrField<RimWellPathAttributeCollection*> m_wellPathAttributeCollection;
    caf::PdmField<double>                             m_overburdenHeight;
    caf::PdmField<double>                             m_underburdenHeight;
};
