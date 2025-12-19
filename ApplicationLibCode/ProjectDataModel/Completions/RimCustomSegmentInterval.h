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
/// Represents a custom segment interval for a specific measured depth range
///
//==================================================================================================
class RimCustomSegmentInterval : public caf::PdmObject, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimCustomSegmentInterval();
    ~RimCustomSegmentInterval() override;

    // Getters
    double startMD() const override;
    double endMD() const override;

    // Setters
    void setStartMD( double startMD );
    void setEndMD( double endMD );

    // Validation
    std::map<QString, QString> validate( const QString& configName = "" ) const override;
    bool                       isValidInterval() const;
    bool                       overlaps( const RimCustomSegmentInterval* other ) const;
    bool                       containsMD( double md ) const;

    // Comparison for sorting
    bool operator<( const RimCustomSegmentInterval& rhs ) const;

    // Overrides from RimWellPathComponentInterface
    bool                              isEnabled() const override;
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    void                              applyOffset( double offsetMD ) override;

    // Visual feedback for overlapping intervals
    void updateOverlapVisualFeedback( bool hasOverlap );

    QString generateDisplayLabel() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void updateConnectedEditors();

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
};
