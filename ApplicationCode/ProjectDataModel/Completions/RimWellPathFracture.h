/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimFracture.h"

namespace caf
{
class PdmFieldHandle;
class PdmUiEditorAttribute;
} // namespace caf

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathFracture : public RimFracture
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathFracture( void );
    ~RimWellPathFracture( void ) override;

    double fractureMD() const override;
    void   setMeasuredDepth( double mdValue );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void updateAzimuthBasedOnWellAzimuthAngle() override;

    double wellAzimuthAtFracturePosition() const override;

    void loadDataAndUpdate() override;

    std::vector<cvf::Vec3d> perforationLengthCenterLineCoords() const override;

    static bool compareByWellPathNameAndMD( const RimWellPathFracture* lhs, const RimWellPathFracture* rhs );

    bool isEnabled() const override; // RimWellPathCompletionsInterface override

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    void updatePositionFromMeasuredDepth();

private:
    caf::PdmField<float> m_measuredDepth;
};
