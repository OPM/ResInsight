/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "RimCheckableNamedObject.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmChildArrayField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

//==================================================================================================
///
///
//==================================================================================================
class RimFishboneWellPath : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFishboneWellPath();
    ~RimFishboneWellPath() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void setCoordinates( std::vector<cvf::Vec3d> coordinates );
    void setMeasuredDepths( std::vector<double> measuredDepths );

    std::vector<cvf::Vec3d> coordinates() const { return m_coordinates(); }
    std::vector<double>     measuredDepths() const { return m_measuredDepths(); }

private:
    QString displayCoordinates() const;

    caf::PdmField<std::vector<cvf::Vec3d>> m_coordinates;
    caf::PdmField<std::vector<double>>     m_measuredDepths;
    caf::PdmProxyValueField<QString>       m_displayCoordinates;
};
