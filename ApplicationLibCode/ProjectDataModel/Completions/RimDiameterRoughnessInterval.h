/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaDefines.h"
#include "RimWellPathComponentInterface.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RimWellPath;

//==================================================================================================
///
/// Represents a diameter and roughness interval for a specific measured depth range
///
//==================================================================================================
class RimDiameterRoughnessInterval : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimDiameterRoughnessInterval();
    ~RimDiameterRoughnessInterval() override;

    // Getters
    double startMD() const override;
    double endMD() const override;
    double diameter() const;
    double diameter( RiaDefines::EclipseUnitSystem unitSystem ) const;
    double roughnessFactor() const;
    double roughnessFactor( RiaDefines::EclipseUnitSystem unitSystem ) const;

    // Setters
    void setStartMD( double startMD );
    void setEndMD( double endMD );
    void setDiameter( double diameter );
    void setRoughnessFactor( double roughness );

    // Validation
    bool isValidInterval() const;
    bool overlaps( const RimDiameterRoughnessInterval* other ) const;
    bool containsMD( double md ) const;

    // Display
    QString diameterLabel() const;
    QString roughnessLabel() const;

    // Comparison for sorting
    bool operator<( const RimDiameterRoughnessInterval& rhs ) const;

    // Overrides from RimWellPathComponentInterface
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    void                              applyOffset( double offsetMD ) override;

    // Public static methods for default values
    static double defaultDiameter( RiaDefines::EclipseUnitSystem unitSystem );
    static double defaultRoughness( RiaDefines::EclipseUnitSystem unitSystem );

    // Visual feedback for overlapping intervals
    void updateOverlapVisualFeedback( bool hasOverlap );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void    updateConnectedEditors();
    QString generateDisplayLabel() const;

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_diameter;
    caf::PdmField<double> m_roughnessFactor;
};