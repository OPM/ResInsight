/////////////////////////////////////////////////////////////////////////////////
//
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
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

// Include to make Pdm work for cvf::Color

#include "cvfObject.h"

class RigPolyLinesData;
class RimPolylineAppearance;

//==================================================================================================
///
///
//==================================================================================================
class RimPolylinesAnnotation : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolylinesAnnotation();
    ~RimPolylinesAnnotation() override;

    virtual cvf::ref<RigPolyLinesData> polyLinesData() = 0;
    virtual bool                       isEmpty()       = 0;

    bool isActive();
    bool isVisible();

    bool closePolyline() const;
    bool showLines() const;
    bool showSpheres() const;

    RimPolylineAppearance* appearance() const;

protected:
    caf::PdmFieldHandle* objectToggleField() override;

protected:
    caf::PdmField<bool> m_isActive;

    caf::PdmField<bool> m_closePolyline;
    caf::PdmField<bool> m_showLines;
    caf::PdmField<bool> m_showSpheres;

    caf::PdmChildField<RimPolylineAppearance*> m_appearance;
};
